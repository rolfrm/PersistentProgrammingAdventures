#define GL_GLEXT_PROTOTYPES
#include <iron/full.h>
#include <iron/gl.h>
#include <GL/gl.h>
#include <GL/glext.h>


static bool quadheight_initialized;
static u32 basic3d_shader;

static u32 plane_buffer;
static u32 plane_buffer_count;
static u32 plane_buffer_indicies;
static u32 plane_size;
extern char src_basic3d_vs[];
extern u32 src_basic3d_vs_len;
extern char src_basic3d_fs[];
extern u32 src_basic3d_fs_len;

bool get_key_state(int  key);
static float d = 0;
vec4 campos = {.data = {0,0,0,1}};
mat4 orient = {.data = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}}};
void test_render_distfield(float aspect){
  glDisable(GL_BLEND);
  //glClearDepth(2.0f);
  //glDepthMask(GL_TRUE);
  //glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  //glDepthFunc(GL_LEQUAL);  
  //glClear(GL_DEPTH_BUFFER_BIT);
  d += 0.01;
  int current_prog;
  int size = 128;
  glGetIntegerv(GL_CURRENT_PROGRAM, &current_prog);
  if(!quadheight_initialized){
    quadheight_initialized = true;
    basic3d_shader = gl_shader_compile2(src_basic3d_vs,src_basic3d_vs_len, src_basic3d_fs,src_basic3d_fs_len);
    
    int width = size, height = size;
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
  
  for(float _i = 0; _i < 1; _i += 1){ 
    mat4 mod2 = mat4_translate(sin(d + _i) * 20, cos(d + _i) * 20, 10.0);
    
    mat4 camera_transform_inverse = mat4_invert(camera_transform);
    //rot = mat4_rotate_Z(model_transform1, -r * 2.0);
    mat4 model = mat4_mul(model_transform, model_scale);

  
    model = mat4_mul(mod2, mat4_mul(mega_transform, model));

    mat4 transform = mat4_mul(perspective_transform, model);

  //mat4 transform = perspective_transform;
  var identity_loc = glGetUniformLocation(basic3d_shader, "transform");
  var model_loc = glGetUniformLocation(basic3d_shader, "model");
  var model2_loc = glGetUniformLocation(basic3d_shader, "model2");
  var camera_loc = glGetUniformLocation(basic3d_shader, "camera");
  var camera_inv_loc = glGetUniformLocation(basic3d_shader, "camera_inv");
  var inv_model_loc = glGetUniformLocation(basic3d_shader, "inv_model");

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
  glUniformMatrix4fv(model2_loc, 1, false, mod2.data[0]);
 
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
}
