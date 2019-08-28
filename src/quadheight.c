#define GL_GLEXT_PROTOTYPES
#include <iron/full.h>
#include <GL/gl.h>
#include <GL/glext.h>


static bool quadheight_initialized;
static u32 basic3d_shader;
static u32 quadbuffer;
static u32 quadbuffer_indicies;

extern char src_basic3d_vs[];
extern u32 src_basic3d_vs_len;
extern char src_basic3d_fs[];
extern u32 src_basic3d_fs_len;
u32 create_shader_from_codes(const char * vsc, const char * fsc);

u32 create_shader_from_codes2(const char * vsc, int vlen, const char * fsc, int flen){
  void * buffer1 = alloc0(vlen + 1);
  memcpy(buffer1, vsc, vlen);
  void * buffer2 = alloc0(flen + 1);
  memcpy(buffer2, fsc, flen);  
  u32 result = create_shader_from_codes(buffer1, buffer2);
  printf("shader code: %s\n", buffer1);
  dealloc(buffer1);
  dealloc(buffer2);
  return result;
  
}
static float d = 2.01;
void test_render_quadheight(){
  glClear(GL_DEPTH_BUFFER_BIT);
  d += 0.01;
  int current_prog;
  glGetIntegerv(GL_CURRENT_PROGRAM, &current_prog);
  if(!quadheight_initialized){
    quadheight_initialized = true;
    basic3d_shader = create_shader_from_codes2(src_basic3d_vs,src_basic3d_vs_len, src_basic3d_fs,src_basic3d_fs_len);
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
    
  }
  glUseProgram(basic3d_shader);

  float x = d;
  float r = 0;
  if(x > 1.5 ){
    x = 1.5;
    r = d - x;
  }
  glEnable(GL_DEPTH_TEST);  
  mat4 perspective_transform = mat4_perspective(1.0, 1, 0.1, 100);
  mat4_print(perspective_transform);
  logd("\n");
  mat4 model_transform1 = mat4_translate(-0.5,-0.5,-0.5);
  mat4 model_transform = mat4_translate(0,0,-x);
  mat4 id = mat4_identity();
  var rot = mat4_rotate_Y(id, M_PI);
  rot = mat4_rotate_Y(rot, r);
  rot = mat4_rotate_Z(rot, r);
  
  UNUSED(r);

  //rot = mat4_rotate_Y(rot, -r);
  mat4 model = mat4_mul(model_transform, mat4_mul(rot, model_transform1));
  mat4 transform = mat4_mul(perspective_transform, model);

  //mat4 transform = perspective_transform;
  var identity_loc = glGetUniformLocation(basic3d_shader, "transform");
  var model_loc = glGetUniformLocation(basic3d_shader, "model");
  var camera_center_loc = glGetUniformLocation(basic3d_shader, "camera_center");
  var inv_model_loc = glGetUniformLocation(basic3d_shader, "inv_model");

  mat4 inv_model = mat4_invert(model);
  
  //UNUSED(identity_loc);
  //UNUSED(identity);
  glUniformMatrix4fv(identity_loc, 1, false, transform.data[0]);
  
  glUniformMatrix4fv(model_loc, 1, false, model.data[0]);
  glUniformMatrix4fv(inv_model_loc, 1, false, inv_model.data[0]);
  glUniform3f(camera_center_loc, 0, 0, 0);
  logd("err? %i\n", glGetError());
  glCullFace( GL_BACK );
  glEnable(GL_CULL_FACE);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadbuffer_indicies);
  glDrawElements(GL_TRIANGLE_STRIP, 19, GL_UNSIGNED_INT, 0);
  //glDrawArrays(GL_POINTS, 0, 8);
  glDisable(GL_DEPTH_TEST);  
}
