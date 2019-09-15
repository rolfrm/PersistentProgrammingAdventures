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
  *distance = vec3_len(p) - 0.5;
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
  if(d < -s || size2 < limit){
    push_cube(ctx, p, size, color);
    return;
  }
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
  ctx->verts = realloc(ctx->verts, sizeof(ctx->verts[0]) * 3 * ctx->cube_count * 8);
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
      v[0] = bufferdata[j * 3] * cube.w + cube.x;
      v[1] = bufferdata[j * 3 + 1] * cube.w + cube.y;
      v[2] = bufferdata[j * 3 + 2] * cube.w + cube.z;
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


static float d = 5.51;
void test_render_quadheight(float aspect){
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
    
  }
  glUseProgram(basic3d_shader);
  gl_texture_bind(heightmap_texture);
  
  float x = d;
  float r = 0;
  if(x > 2 ){
    x = 2;
    r = d - 2;//x;
  }

  for(int k = -8; k < 8; k++){
    for(int j = -8; j < 8;j++){
  mat4 perspective_transform = mat4_perspective(1.0, aspect, 0.1, 100);

  mat4 model_transform1 = mat4_translate(-0.5 + k,-0.2,-0.5 + j);
  mat4 model_transform = mat4_translate(0,0,-x);
  mat4 id = mat4_identity();
  var rot = mat4_rotate_Y(id, M_PI);

  rot = mat4_rotate_X(rot, -0.5);
  rot = mat4_rotate_Y(rot, r);
  //rot = mat4_rotate_Z(rot, r);1
  
  UNUSED(r);

  //rot = mat4_rotate_Y(rot, -r);
  mat4 model = mat4_mul(model_transform, mat4_mul(rot, model_transform1));
  mat4 transform = mat4_mul(perspective_transform, model);

  //mat4 transform = perspective_transform;
  var identity_loc = glGetUniformLocation(basic3d_shader, "transform");
  var model_loc = glGetUniformLocation(basic3d_shader, "model");
  var camera_center_loc = glGetUniformLocation(basic3d_shader, "camera_center");
  var inv_model_loc = glGetUniformLocation(basic3d_shader, "inv_model");
  var texture_loc = glGetUniformLocation(basic3d_shader, "tex");
  var size_loc = glGetUniformLocation(basic3d_shader, "size");
  glUniform1i(texture_loc, 0);
  glUniform1f(size_loc, (float) size);
  //ASSERT(!glGetError());


  mat4 inv_model = mat4_invert(model);
  
  //UNUSED(identity_loc);
  //UNUSED(identity);
  glUniformMatrix4fv(identity_loc, 1, false, transform.data[0]);
  
  glUniformMatrix4fv(model_loc, 1, false, model.data[0]);
  glUniformMatrix4fv(inv_model_loc, 1, false, inv_model.data[0]);
  glUniform3f(camera_center_loc, 0, 0, 0);

  glCullFace( GL_BACK );
  glEnable(GL_CULL_FACE);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadbuffer_indicies);
  glDrawElements(GL_TRIANGLE_STRIP, 19, GL_UNSIGNED_INT, 0);
  //glDrawArrays(GL_POINTS, 0, 8);
  }
  }
}
