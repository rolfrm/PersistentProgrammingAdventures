#define GL_GLEXT_PROTOTYPES
#include <iron/full.h>
#include <iron/gl.h>
#include <GL/gl.h>
#include <GL/glext.h>


static bool quadheight_initialized;
static u32 basic3d_shader;
static u32 quadbuffer;
static u32 quadbuffer_indicies;
static texture heightmap_texture;

static u32 cubes_buffer;
static u32 cubes_elem_count;

static u32 plane_buffer;
static u32 plane_buffer_count;
static u32 plane_buffer_indicies;
static u32 plane_size;
extern char src_basic3d_vs[];
extern u32 src_basic3d_vs_len;
extern char src_basic3d_fs[];
extern u32 src_basic3d_fs_len;

vec2 advance_ray(vec2 dir, vec2 p){
  vec2 isneg = vec2_new(dir.x < 0? -1 : 0, dir.y < 0? -1: 0);
  vec2 p2 = vec2_add(p, vec2_scale(dir, 0.00001));
  p2 = vec2_add(p2, isneg);
  p2.x = ceil(p2.x);
  p2.y = ceil(p2.y);
  vec2 d = vec2_div(vec2_sub(p2, p), dir);
  vec2 out = vec2_add(p, vec2_scale(dir, MIN(d.x, d.y)));
  vec2 out3 = vec2_new(floor(out.x), floor(out.y));
  if(d.x < d.y){
    out3.x += isneg.x;
  }else{
    out3.y += isneg.y;
  }
  vec2_print(out3);logd("\n");
  return out;
}

void test_ray_scenarios(){
  vec2 p = vec2_new1(0.5);
  for(int i = 0; i < 10; i++){
    printf("%f %f\n", p.x, p.y);

    p = advance_ray(vec2_new(0.25,-0.5), p);
  }
}

void one_sphere(vec3 p, float * distance, vec3 * color){
  *distance = vec3_len(vec3_sub(p, vec3_new(0.5,0.5,0.5))) - 0.3;
  *color = vec3_new(1,0,0);
}

typedef void (* distance_field)(vec3 p, float * distance, vec3 * color);

typedef struct{
  float limit;
  distance_field f;
  vec4 * cubes;
  size_t cube_count;
  size_t cube_capacity;
  float * verts;
  size_t vert_count;
  int * indicies;
}cubes_context;

void push_cube(cubes_context * ctx, vec3 p, float size, vec3 color){
  UNUSED(ctx);
  ctx->cube_count++;
  if(ctx->cube_count > ctx->cube_capacity){
    ctx->cube_capacity = 1 + ctx->cube_capacity * 2;
    ctx->cubes = realloc(ctx->cubes, sizeof(ctx->cubes[0]) * ctx->cube_capacity);
  }
  ctx->cubes[ctx->cube_count - 1] = vec4_new(p.x, p.y, p.z, size);
  //logd("%i: ", ctx->cube_count);
  //vec3_print(p);logd("%f", size);vec3_print(color);logd("\n");
  UNUSED(p);UNUSED(size);UNUSED(color);
  
  
  
}
void cubes_build_arrays(cubes_context * ctx);
void distfield_collect_cubes(cubes_context * ctx, vec3 p, float size){
  distance_field f = ctx->f;
  float limit = ctx->limit;
  float d;
  vec3 color;
  f(p, &d, &color);
  float s = sqrtf(size * size * 3) * 0.5f;
  float size2 = size * 0.5;
  //UNUSED()
  if(size < limit){
    push_cube(ctx, p, size, color);
    return;
  }
  if(d < -s) return;
  if(d > s) return;
  //if(size2 < limit) return;
  distfield_collect_cubes(ctx, p, size2); //0 0 0
  p.x += size2;  
  distfield_collect_cubes(ctx, p, size2); //1 0 0
  p.y += size2;
  distfield_collect_cubes(ctx, p, size2); //1 1 0
  p.z += size2;
  distfield_collect_cubes(ctx, p, size2); //1 1 1
  p.x -= size2;
  distfield_collect_cubes(ctx, p, size2); //0 1 1
  p.y -= size2;
  distfield_collect_cubes(ctx, p, size2); //0 0 1
  p.x += size2;
  distfield_collect_cubes(ctx, p, size2); //1 0 1 
  p.x -= size2;
  p.y += size2;
  distfield_collect_cubes(ctx, p, size2); //0 1 1
}

void cubes_build_arrays(cubes_context * ctx){
  ctx->vert_count = ctx->cube_count * 8;
  ctx->verts = realloc(ctx->verts, sizeof(ctx->verts[0]) * 3 * ctx->vert_count);
  ctx->indicies = realloc(ctx->indicies, sizeof(ctx->indicies[0]) * ctx->cube_count * 20);

  float bufferdata[] = {0,0,0,
			1,0,0,
			0,1,0,
			1,1,0,
			0,0,1,
			1,0,1,
			0,1,1,
			1,1,1};
    // triangle strip
  u32 quadindicies[] = {1, 0, 3, 2,  7, 6,  5, 4,  1, 0, 1, 3, 5, 7, 7, 0, 2, 4, 6, 6};
  UNUSED(quadindicies);
  for(size_t i = 0 ;i < ctx->cube_count; i++){
    float * v = ctx->verts + i * 3 * 8;
    vec4 cube = ctx->cubes[i];
    for(int j = 0; j < 8; j++){
      v[j * 3] = bufferdata[j * 3] * cube.w + cube.x;
      v[j * 3 + 1] = bufferdata[j * 3 + 1] * cube.w + cube.y;
      v[j * 3 + 2] = bufferdata[j * 3 + 2] * cube.w + cube.z;
    }
  }
}


void test_distfield_cubing(){
  cubes_context ctx = {0};
  ctx.limit = 0.01;
  ctx.f = one_sphere;
  distfield_collect_cubes(&ctx, vec3_zero, 2.0);
  cubes_build_arrays(&ctx);
}

bool get_key_state(int  key);
static float d = 0;
//mat4 campos = {.data = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}}};
vec4 campos = {.data = {0,0,0,1}};
mat4 orient = {.data = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}}};
void test_render_distfield(float aspect){
  glDisable(GL_BLEND);
  glClearDepth(2.0f);
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);  
  glClear(GL_DEPTH_BUFFER_BIT);
  d += 0.01;
  size_t size = 16;
  int current_prog;
  glGetIntegerv(GL_CURRENT_PROGRAM, &current_prog);
  if(!quadheight_initialized){
    quadheight_initialized = true;
    basic3d_shader = gl_shader_compile2(src_basic3d_vs,src_basic3d_vs_len, src_basic3d_fs,src_basic3d_fs_len);
    glGenBuffers(1, &quadbuffer);
    glGenBuffers(1, &quadbuffer_indicies);
 
    float bufferdata[] = {0,0,0,
			  1,0,0,
			  0,1,0,
			  1,1,0,
			  0,0,1,
			  1,0,1,
			  0,1,1,
			  1,1,1};
    // triangle strip
    u32 quadindicies[] = {1, 0, 3, 2,  7, 6,  5, 4,  1, 0, 1, 3, 5, 7, 7, 0, 2, 4, 6};
    
    UNUSED(bufferdata);
    glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bufferdata), bufferdata, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadbuffer_indicies);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadindicies), quadindicies, GL_STATIC_DRAW);


    size_t total_heightmap_size = size * 3 * size;
    u8 * heightmap = alloc0(total_heightmap_size);

    size_t h = size / 2;
    for(size_t i = 0; i < size; i++){
      for(size_t j = 0; j < size; j++){
	int d = h - (int) sqrt((i - h) *(i - h) + (j - h) * (j - h)) * 0.5;
	u8 * p = (i + j * size) * 3 + heightmap;
	p[0] = d * (255 / size);
	p[1] = (d + 1) * (255 / size);
	p[2] = i % 4 * (255 / size);
      }
    }
    
    image img = image_new(size, size, 3);
    memcpy(image_data(&img), heightmap, total_heightmap_size);
    heightmap_texture = texture_from_image2(&img, TEXTURE_INTERPOLATION_NEAREST);
    image_delete(&img);
    dealloc(heightmap);

    cubes_context ctx = {0};
    ctx.limit = 0.01;
    ctx.f = one_sphere;
    distfield_collect_cubes(&ctx, vec3_zero, 4.0);
    cubes_build_arrays(&ctx);
    
    glGenBuffers(1, &cubes_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, cubes_buffer);
    glBufferData(GL_ARRAY_BUFFER, ctx.vert_count  * sizeof(float) * 3, ctx.verts, GL_STATIC_DRAW);
    cubes_elem_count = ctx.vert_count;

    int width = 128, height = 128;
    //u32 totalCount = width * height;
    float * plane_data = alloc0(sizeof(float) * 3 * width * height);
    int * indicies = alloc0(sizeof(int) * width * height * 3);
    for(int i = 0; i < height; i++){
      // iterate rows
      for(int j = 0; j < width; j++){
	// iterate columns
	int index = (j + i * width) * 3;
	
	plane_data[index] = j;
	plane_data[index + 1] = i;
	plane_data[index + 2] = 0;
	
      }
    }
    u32 pcnt = 0;
    for(int i = 0; i < height - 1; i++){
      //int row_offset = i * width;
      /*if(i > 0){
	indicies[(row_offset- 1) * 2] = indicies[(row_offset - 2) * 2 + 1];
	indicies[(row_offset- 1) * 2 + 1] = indicies[(row_offset - 2) * 2 + 1]; 
	}*/
      if(i > 0){
	indicies[pcnt] = (i) * width;
	pcnt += 1;
	indicies[pcnt] = (i) * width;
	pcnt += 1;
      }
      for(int j = 0; j < width; j++){
	indicies[pcnt] = i * width + j;
	pcnt += 1;
	indicies[pcnt] = (i + 1) * width + j;
	pcnt += 1;
	if(j == (width - 1)){
	  indicies[pcnt] = (i + 1) * width + j;
	  pcnt += 1;
	}
	
      }
    }

    plane_buffer_count = pcnt;
    glGenBuffers(1, &plane_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, plane_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * width * height, plane_data, GL_STATIC_DRAW);
    glGenBuffers(1, &plane_buffer_indicies);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plane_buffer_indicies);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * pcnt, indicies, GL_STATIC_DRAW);
    plane_size = width;
  }
  glUseProgram(basic3d_shader);
  gl_texture_bind(heightmap_texture);
  
  
  for(int k = -0; k < 1; k++){
    for(int j = 0; j < 1;j++){
  mat4 perspective_transform = mat4_perspective(1.0, aspect, 0.2, 1000);

  float ps = plane_size;
  //mat4 model_transform1 = mat4_translate(-(ps - 1) / 2.0f,-(ps - 1) / 2.0f,-0);
  mat4 model_scale = mat4_scaled(2.0 / ps, 2.0 / ps, 1);
  mat4 model_transform = mat4_translate(-1.0,-1.0, -1);
  mat4 mega_transform = mat4_translate(0, 0, 0);
  mat4 camera_transform = mat4_translate(campos.x,campos.y,campos.z);

  //
  //var rot = mat4_rotate_Y(id, 0);

  //rot = mat4_rotate_X(rot, -0.5);
  //rot = mat4_rotate_Y(rot, r);
  //rot = mat4_rotate_Z(rot, r);

  float rx = 0.0;
  float ry = 0.0;
  float cz = 0.0;
  float cx = 0.0;
  float cy = 0.0;
  if(get_key_state(KEY_ENTER)){
    cz += get_key_state(KEY_SHIFT) ? 1 : -1.0;
  }
  if(get_key_state(KEY_UP)){
    rx += 1.0;
  }
  if(get_key_state(KEY_DOWN)){
    rx -= 1.0;
  }
  if(get_key_state(KEY_LEFT)){
    ry -= 1;
  }
  if(get_key_state(KEY_RIGHT)){
    ry += 1;
  }
  if(get_key_state(KEY_SHIFT)){
    cx = ry;
    ry = 0;
    cy = rx;
    rx = 0;
  }
  
  mat4 id = mat4_identity();
  mat4 roll = mat4_mul(mat4_rotate_X(id, rx * 0.05), mat4_rotate_Y(id, ry * 0.05));
  orient = mat4_mul(orient, roll);
  campos.w = 0.0;
  campos = vec4_add(campos, mat4_mul_vec4(orient, vec4_new(cx,cy,cz, 1)));
  camera_transform = mat4_mul(camera_transform, orient);
  mat4 camera_transform_inverse = mat4_invert(camera_transform);
  //rot = mat4_rotate_Z(model_transform1, -r * 2.0);
  mat4 model = mat4_mul(model_transform, model_scale);
  model = mat4_mul(camera_transform, mat4_mul(mega_transform, model));

  mat4 transform = mat4_mul(perspective_transform, model);

  //mat4 transform = perspective_transform;
  var identity_loc = glGetUniformLocation(basic3d_shader, "transform");
  var model_loc = glGetUniformLocation(basic3d_shader, "model");
  var camera_loc = glGetUniformLocation(basic3d_shader, "camera");
  var camera_inv_loc = glGetUniformLocation(basic3d_shader, "camera_inv");
  var inv_model_loc = glGetUniformLocation(basic3d_shader, "inv_model");
  var texture_loc = glGetUniformLocation(basic3d_shader, "tex");
  var size_loc = glGetUniformLocation(basic3d_shader, "size");
  glUniform1i(texture_loc, 0);
  glUniform1f(size_loc, (float) size);
  //ASSERT(!glGetError());
  UNUSED(transform);

  mat4 inv_model = mat4_invert(model);
  
  //UNUSED(identity_loc);
  //UNUSED(identity);
  glUniformMatrix4fv(identity_loc, 1, false, perspective_transform.data[0]);
  
  glUniformMatrix4fv(model_loc, 1, false, model.data[0]);
  glUniformMatrix4fv(inv_model_loc, 1, false, inv_model.data[0]);
  glUniformMatrix4fv(camera_loc, 1, false, camera_transform.data[0]);
  glUniformMatrix4fv(camera_inv_loc, 1, false, camera_transform_inverse.data[0]);
 
  glCullFace( GL_BACK );
  glDisable(GL_CULL_FACE);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, plane_buffer);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
  //glDrawArrays(GL_TRIANGLE_STRIP , 0, 19);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plane_buffer_indicies);
  glDrawElements(GL_TRIANGLE_STRIP, /*((int)(x * 50)) %*/plane_buffer_count, GL_UNSIGNED_INT, 0);

  }
  }
}
