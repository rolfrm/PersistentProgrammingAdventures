#define GL_GLEXT_PROTOTYPES
#include <iron/full.h>
#include <iron/gl.h>
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

static float d = 5.51;
void test_render_quadheight(float aspect){
  glDisable(GL_BLEND);
  glClearDepth(2.0f);
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);  
  glClear(GL_DEPTH_BUFFER_BIT);
  d += 0.01;
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
    
  }
  glUseProgram(basic3d_shader);
  float x = d;
  float r = 0;
  if(x > 2 ){
    x = 2;
    r = d - 2;//x;
  }

  for(int j = 0; j < 1;j++){
  mat4 perspective_transform = mat4_perspective(1.5, aspect, 0.1, 100);

  mat4 model_transform1 = mat4_translate(-0.5,-0.5,-0.5 + j);
  mat4 model_transform = mat4_translate(0,0,-x);
  mat4 id = mat4_identity();
  var rot = mat4_rotate_Y(id, M_PI);

  rot = mat4_rotate_X(rot, -1);
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
