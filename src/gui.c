 
#include <iron/full.h>
#include "main.h"
#include <iron/gl.h>
#include <GL/gl.h>
//#include <GL/glew.h>
#include "u64_table.h"
typedef void * void_ptr;
#include "u64_to_ptr.h"
#include "u64_to_vec2_table.h"
#include "u64_to_f32_table.h"
#include "oop.h"

#include "alignments.h"
#include "margin_table.h"
#include "gui.h"

u64_table * control_tree;

void control_add_child(u64 control, u64 child){
  if(control_has_child(control, child) == false)
    u64_table_set(control_tree, control, child);
}

bool control_has_child(u64 control, u64 child){
  size_t index = 0, count = 0;
  size_t indexes[10];
  while((count = u64_table_iter(control_tree, &control, 1, NULL, indexes, array_count(indexes), &index)))
    {
      for(u64 _i = 0; _i < count; _i++){
	size_t i = indexes[_i];
	if(child == control_tree->value[i])
	  return true;
      }
    }
  return false;
}


alignments * alignment_table;

margin_table * margins;
margin_table * padding;
margin_table * color;
margin_table * layout;
u64_to_vec2_table * desired_size;
u64_to_vec2_table * window_requested_size;
u64_to_f32_table * min_width;
u64_to_f32_table * min_height;
u64_to_f32_table * max_width;
u64_to_f32_table * max_height;


void window_request_size(u64 window, int width, int height){
  u64_to_vec2_table_set(window_requested_size, window, width, height);
}

bool window_get_requested_size(u64 window, int * width, int *  height){
  float w,h;
  if(u64_to_vec2_table_try_get(window_requested_size, &window, &w, &h)){
    *width = (int)w;
    *height = (int)h;
    u64_to_vec2_table_unset(window_requested_size, window);
    return true;
  }
  return false;
}


u64 arrange_method;
u64 measure_method;
u64 render_method;
void render_control(u64 control){
 
  size_t index = 0, count = 0;
  size_t indexes[10];
  while((count = u64_table_iter(control_tree, &control, 1, NULL, indexes, array_count(indexes), &index)))
    {
      for(u64 _i = 0; _i < count; _i++){
	 
	 size_t i = indexes[_i];
	 u64 child = control_tree->value[i];
	 call_method(child, child, render_method);
      }
    }
}

void green_button_render(u64 control){
  float x = 0,y = 0,w = 0,h = 0;
  margin_table_try_get(layout, &control, &x, &y, &w, &h);
  if(w > 0 && h > 0){
    float r = 0,g = 0,b = 0,a = 0;
    margin_table_try_get(color, &control, &r, &g, &b, &a);

    blit_rectangle(x, y, w, h, r, g, b, a);
  }
}



__thread vec2 offset, scale;

void control_arrange(u64 control){
  float desired_x = 0.0, desired_y = 0.0;
  u64_to_vec2_table_try_get(desired_size, &control, &desired_x, &desired_y);
  
  //printf("arrange %i: %f %f\n", control, desired_x, desired_y);

  
  vec2 _offset = offset, _scale = scale;

  f32 left, right,up,down;
  if(margin_table_try_get(padding, &control, &left, &right, &up, &down)){
    offset.x += left;
    offset.y += up;
    scale.x -= (right + left);
    scale.y -= (down + up);
  }
  margin_table_set(layout, control, offset.x, offset.y, scale.x, scale.y);   
  size_t index = 0, count = 0;
  size_t indexes[10];
  while((count = u64_table_iter(control_tree, &control, 1, NULL, indexes, array_count(indexes), &index))){
    for(u64 _i = 0; _i < count; _i++){
      
      size_t i = indexes[_i];
      u64 child = control_tree->value[i];
      f32 left, right,up,down;
      bool reset = false;
      vec2 scale_save = scale;
      if(margin_table_try_get(margins, &child, &left, &right, &up, &down)){
	reset = true;
	offset.x += left;
	offset.y += up;
	scale.x -= (right + left);
	scale.y -= (down + up);
      }
      vec2 desired;
      u64_to_vec2_table_try_get(desired_size, &child, &desired.x, &desired.y);
      if(desired.x < scale.x){
	scale.x = desired.x;
      }
      if(desired.y < scale.y){
	scale.y = desired.y;
      }
      
      call_method(child, child, arrange_method);
      scale = scale_save;
      if(reset){
	offset.x -= left;
	offset.y -= up;
	scale.x += (right + left);
	scale.y += (down + up);
      }
    }
  }
  offset = _offset;
  scale = _scale;
}

void control_measure(u64 control){

  vec2 _scale = scale;
  vec2 offset = vec2_new(0, 0);
  alignment valign = ALIGN_STRETCH, halign = ALIGN_STRETCH;
  alignments_try_get(alignment_table, &control, &halign, &valign);
  f32 left, right,up,down;
  if(margin_table_try_get(padding, &control, &left, &right, &up, &down)){
    offset.x += left;
    offset.y += up;
    scale.x -= (right + left);
    scale.y -= (down + up);
  }
  u64_to_vec2_table_set(desired_size, control, scale.x, scale.y);

  margin_table_set(layout, control, offset.x, offset.y, scale.x, scale.y);   
  size_t index = 0, count = 0;
  size_t indexes[10];
  while((count = u64_table_iter(control_tree, &control, 1, NULL, indexes, array_count(indexes), &index))){
    for(u64 _i = 0; _i < count; _i++){
      
      size_t i = indexes[_i];
      u64 child = control_tree->value[i];
      f32 left, right, up, down;
      bool reset = false;
      if(margin_table_try_get(margins, &child, &left, &right, &up, &down)){
	reset = true;
	offset.x += left;
	offset.y += up;
	scale.x -= (right + left);
	scale.y -= (down + up);
      }
      call_method(child, child, measure_method);
      if(reset){
	offset.x -= left;
	offset.y -= up;
	scale.x += (right + left);
	scale.y += (down + up);
      }
    }
  }
  float desired_x = 0.0, desired_y = 0.0;
  u64_to_vec2_table_try_get(desired_size, &control, &desired_x, &desired_y);
  float minwidth;
 
  if(u64_to_f32_table_try_get(min_width, &control, &minwidth)){
    if(desired_x < minwidth){
      desired_x = minwidth;
    }
  }

  float minheight;
  if(u64_to_f32_table_try_get(max_width, &control, &minheight)){
    if(desired_y < minheight){
      desired_y = minheight;
    }
  }
  u64_to_vec2_table_set(desired_size, control, desired_x, desired_y);
  

  scale = _scale;
}


u64_to_ptr * window_handle;
void test_render_quadheight(float aspect);
void test_render_distfield(float aspect);
void render_octree_control(u64 control){
  UNUSED(control);
  float x = 0,y = 0,w = 0,h = 0;
  margin_table_try_get(layout, &control, &x, &y, &w, &h);

  int vpdata[4];
  glGetIntegerv(GL_VIEWPORT, vpdata);
  glViewport(x,y,w,h);
  float aspect = (float) w / h;
  //glScissor(x,y,w,h);
  blit_push();
  blit_begin(BLIT_MODE_UNIT);
  blit_rectangle(-1,-1,2,2, 0,0,0,1);
  //test_render_quadheight(aspect);
  test_render_distfield(aspect);
  blit_pop();
  glViewport(vpdata[0], vpdata[1], vpdata[2], vpdata[3]);
  glScissor(vpdata[0],vpdata[1],vpdata[2],vpdata[3]);
  
}

void * current_win_handle;

bool get_key_state(int  key){
  return gl_window_get_key_state(current_win_handle, key);
}

void render_window(u64 control){
  //return;
  //logd("Render window...\n");
  if(window_handle == NULL){
    window_handle = u64_to_ptr_create(NULL);
  }
  void * win_handle;
  if(!u64_to_ptr_try_get(window_handle, &control, &win_handle)){
    win_handle = gl_window_open(512, 512);
    u64_to_ptr_set(window_handle, control, win_handle);
  }
  gl_window_make_current(win_handle);

  int width = 512, height = 512;

  {
    int r_width, r_height;
    if(window_get_requested_size(control, &r_width, &r_height)){
      gl_window_get_size(win_handle, &width, &height);
      if(r_width != width || r_height != height)
	gl_window_set_size(win_handle, r_width, r_height);
    }
  }
  current_win_handle = win_handle;
  if(gl_window_get_key_state(win_handle, KEY_ENTER)){
    //printf("SPACE Pressed\n");
  }
  call_method(control, control, measure_method);
  call_method(control, control, arrange_method);

  blit_begin(BLIT_MODE_PIXEL);
  blit_scale(2,2);

  gl_window_get_size(win_handle, &width, &height);
    
  blit_translate(- width / 2, - height / 2);

  blit_rectangle(0,0,width,height, 0.2,0.2,0.2,1);
  offset = vec2_new(0,0);
  scale = vec2_new(width, height);
  
  call_next();
  gl_window_swap(win_handle);
}

void mouse_down_called(u64 id){
  UNUSED(id);
}

void green_button_mouse_down_called(u64 id){
  printf("Clicked button\n");
  UNUSED(id);
  call_next();
}
u64 class_window, class_uielement, class_button, class_green_button;
u64 mouse_down_method;
void gui_init(){
  layout = margin_table_create("layout");
  window_requested_size = u64_to_vec2_table_create("window_requested_size");
  class_table = u64_table_create("class");
  ((bool *) (&class_table->is_multi_table))[0] = true;
  control_tree = u64_table_create("control_tree");
  ((bool *) (&control_tree->is_multi_table))[0] = true;

  alignment_table = alignments_create("alignment");
  margins = margin_table_create("margins");
  padding = margin_table_create("padding");
  color = margin_table_create("color");

  min_width = u64_to_f32_table_create("min_width");
  min_height= u64_to_f32_table_create("min_height");
  max_width= u64_to_f32_table_create("max_width");
  max_height= u64_to_f32_table_create("max_height");
  
  desired_size = u64_to_vec2_table_create("desired_size");

  class_button = intern_string("button");

  class_window = intern_string("window");
  class_uielement = intern_string("ui-element");
  
  set_baseclass(class_window, class_uielement);
  set_baseclass(class_button, class_uielement);

  ASSERT(is_subclass_of(intern_string("window"), intern_string("ui-element")));
  ASSERT(is_subclass_of(class_button, intern_string("ui-element")));
  ASSERT(false == is_subclass_of(class_button, intern_string("window")));

  render_method  = intern_string("method/render");
  arrange_method = intern_string("method/arrange");
  define_method2(class_uielement, render_method, (method)render_control);
  define_method2(class_uielement, arrange_method, (void *) control_arrange);
  define_method2(class_uielement, measure_method, (method) control_measure);

  define_method2(class_window, render_method, (method) render_window);
  
  mouse_down_method = intern_string("method/mouse down");
  define_method2(class_button, mouse_down_method, (void *) mouse_down_called);

  class_green_button = intern_string("green button");
  define_method2(class_green_button, mouse_down_method, (method) green_button_mouse_down_called);
  define_method2(class_green_button, render_method, (method) green_button_render);
  alignments_set(alignment_table, class_green_button, ALIGN_MIN, ALIGN_STRETCH);

  set_baseclass(class_green_button, class_button);
  
  //call_method(class_green_button, class_green_button, mouse_down_method);
  margin_table_print(margins);
  margin_table_set(margins, 500, 0.0f, 0.0f, 0.0f, 0.0f);
  margin_table_set(margins, class_green_button, 5, 5, 5, 5);
  //return;
  //margin_table_set(color, class_green_button, 0.0, 0.7, 0.2, 1.0);


}

