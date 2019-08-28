
#include <iron/full.h>
#include "main.h"
#include <iron/gl.h>
#include <GL/gl.h>
//#include <GL/glew.h>
#include "u64_table.h"
typedef void * void_ptr;
#include "u64_to_ptr.h"
#include "u128_to_u64.h"
#include "u64_to_vec2_table.h"
#include "u64_to_f32_table.h"
u64_table * class_table;

size_t iterate_subclasses(u64 class, u64 * subclasses, size_t cnt, size_t * it){
  size_t indexes[cnt];
  size_t cnt2 = u64_table_iter(class_table, &class, 1, NULL, indexes, array_count(indexes), it);
  for(size_t i = 0; i < cnt2; i++){
    subclasses[i] = class_table->value[indexes[i]];
  }
  return cnt2;
}

size_t u64_lookup_find(u64_table * table, u64 key, u64 value){
  size_t indexes[10];
  size_t it = 0;
  size_t cnt2 = u64_table_iter(table, &key, 1, NULL, indexes, array_count(indexes), &it);
  for(size_t i = 0; i < cnt2; i++){
    if(value == class_table->value[indexes[i]])
      return indexes[i];
  }
  return 0;
}


bool is_subclass_of(u64 class, u64 baseclass){
  return u64_lookup_find(class_table, class, baseclass) != 0;
}

void set_baseclass(u64 class, u64 baseclass){
  if(is_subclass_of(class, baseclass))
    return;
     
  u64_table_set(class_table, class, baseclass);
}

void unset_baseclass(u64 class, u64 baseclass){
  size_t index = u64_lookup_find(class_table, class, baseclass);
  if(index != 0){
    icy_table_remove_indexes((icy_table *)class_table, &index, 1);
  }
}

u128_to_u64 * class_method_table;
u64_to_ptr * method_table;

typedef void (* method)(u64 control, ...);

typedef union{
  struct{
    u64 class;
    u64 method;
  };
  u128 index;
}class_method;

bool has_method(u64 object, u64 method_id, u64 handler){
  class_method cls = { .class = object, .method = method_id};
  size_t indexes[10];
  size_t count, index = 0;

  
  while((count = u128_to_u64_iter(class_method_table, &cls.index, 1, NULL, indexes, array_count(indexes), &index)) > 0){
    for(size_t i = 0; i < count; i++){
      if(class_method_table->value[indexes[i]] == handler)
	return true;
    }
  }
  return false;

}

void set_method(u64 class, u64 method_id, u64 handler){
  class_method cls = { .class = class, .method = method_id};
  u128_to_u64_set(class_method_table, cls.index, handler);
}

u64 get_method(u64 class, u64 method_id){
  class_method cls;
  cls.method = method_id;
  cls.class = class;
  u64 method = 0;
  if(u128_to_u64_try_get(class_method_table, &cls.index, &method))
    return method;
  return 0;
}



void call_method(u64 object, u64 class, u64 method_id);
__thread u64 current_class;
__thread u64 current_method;
__thread u64 current_object;
void call_next(){
  u64 subclasses[10];
  size_t it = 0;
  size_t cnt;
  while((cnt = iterate_subclasses(current_class, subclasses, array_count(subclasses), &it)) > 0){
    for(u64 i = 0; i < cnt; i++){
      call_method(current_object, subclasses[i], current_method);
    }
  }
}

void call_method(u64 object, u64 class, u64 method_id){

  u64 prev_class = current_class;
  u64 prev_method = current_method;
  u64 prev_object = current_object;
  current_class = class;
  current_method = method_id;
  current_object = object;
  
  u64 method_handler_id = get_method(class, method_id);
  if(method_handler_id != 0){
    void * ptr;
    if(u64_to_ptr_try_get(method_table, &method_handler_id, &ptr)){
      method m = ptr;
      m(object);
    }
  }else{
    call_next();
  }
  current_class = prev_class;
  current_method = prev_method;
  current_object = prev_object;
}


void define_method(u64 id, method handler){
  u64_to_ptr_set(method_table, id, handler);
}


void define_method2(u64 class, u64 method_id, method m){
  u64 handler_id = intern_aggregate(class, method_id);
  define_method(handler_id, m);
  set_method(class, method_id, handler_id);
}


u64_table * control_tree;
bool control_has_child(u64 control, u64 child);
void control_add_child(u64 control, u64 child){
  if(control_has_child(control, child) == false)
    u64_table_set(control_tree, control, child);
}

bool control_has_child(u64 control, u64 child){
  u64 index = 0, count = 0;
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

#include "alignments.h"
alignments * alignment_table;

#include "margin_table.h"
margin_table * margins;
margin_table * padding;
margin_table * color;
margin_table * layout;
u64_to_vec2_table * desired_size;
u64_to_f32_table * min_width;
u64_to_f32_table * min_height;
u64_to_f32_table * max_width;
u64_to_f32_table * max_height;

u64 arrange_method;
u64 measure_method;
u64 render_method;
void render_control(u64 control){
 
  u64 index = 0, count = 0;
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
    float r,g,b,a;
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
  u64 index = 0, count = 0;
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
  u64 index = 0, count = 0;
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
void test_render_quadheight();
void render_octree_control(u64 control){
  UNUSED(control);
  float x = 0,y = 0,w = 0,h = 0;
  margin_table_try_get(layout, &control, &x, &y, &w, &h);

  int vpdata[4];
  glGetIntegerv(GL_VIEWPORT, vpdata);
  glViewport(x,y,w,h);
  glScissor(x,y,w,h);
  blit_push();
  blit_begin(BLIT_MODE_UNIT);
  blit_rectangle(-1,-1,2,2, 0,0,0,1);
  test_render_quadheight();
  blit_pop();
  glViewport(vpdata[0], vpdata[1], vpdata[2], vpdata[3]);
  glScissor(vpdata[0],vpdata[1],vpdata[2],vpdata[3]);
  
}

void render_window(u64 control){
  if(window_handle == NULL){
    window_handle = u64_to_ptr_create(NULL);
  }
  void * win_handle;
  if(!u64_to_ptr_try_get(window_handle, &control, &win_handle)){
    win_handle = gl_window_open(512, 512);
    u64_to_ptr_set(window_handle, control, win_handle);
  }
  gl_window_make_current(win_handle);

      
  if(gl_window_get_key_state(win_handle, 0x0020)){
    printf("SPACE Pressed\n");
  }
  call_method(control, control, measure_method);
  call_method(control, control, arrange_method);

  blit_begin(BLIT_MODE_PIXEL);
  blit_scale(2,2);
  int width = 512, height = 512;
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

void test_octree_algorithm();

int main(int argc, char ** argv){
  test_octree_algorithm();
  //return 0;
  //testGlx();
  
  //return 0;
  UNUSED(argc);
  if(argc == 2){
    printf("Intern: %i\n", intern_string(argv[1]));
    return 0;
  }
  if(argc == 3){
    printf("Intern %i %i %i\n", intern_string(argv[1]), intern_string(argv[2]), intern_aggregate(intern_string(argv[1]), intern_string(argv[2])));
    return 0;
  }

  layout = margin_table_create("layout");
  
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
  
  u64 button = intern_string("button");

  u64 window = intern_string("window");
  u64 uielement = intern_string("ui-element");
  
  set_baseclass(window, uielement);
  set_baseclass(button, uielement);

  ASSERT(is_subclass_of(intern_string("window"), intern_string("ui-element")));
  ASSERT(is_subclass_of(button, intern_string("ui-element")));
  ASSERT(false == is_subclass_of(button, intern_string("window")));


  class_method_table = u128_to_u64_create("class methods");
  ((bool *) (&class_method_table->is_multi_table))[0] = true;
  method_table = u64_to_ptr_create(NULL);

  render_method  = intern_string("method/render");
  arrange_method = intern_string("method/arrange");
  define_method2(uielement, render_method, (method)render_control);
  define_method2(uielement, arrange_method, (void *) control_arrange);
  define_method2(uielement, measure_method, (method) control_measure);

  
  define_method2(window, render_method, (method) render_window);
  
  u64 mouse_down = intern_string("method/mouse down");
  define_method2(button, mouse_down, (void *) mouse_down_called);
  call_method(button, button, mouse_down);

  u64 green_button = intern_string("green button");
  define_method2(green_button, mouse_down, (method) green_button_mouse_down_called);
  define_method2(green_button, render_method, (method) green_button_render);
  alignments_set(alignment_table, green_button, ALIGN_MIN, ALIGN_STRETCH);
  
  set_baseclass(green_button, button);
  //set_method(green_button, mouse_down, green_button_mouse_down);
  
  call_method(green_button, green_button, mouse_down);
  margin_table_set(margins, green_button, 5, 5, 5, 5);
  margin_table_set(color, green_button, 0.0, 0.7, 0.2, 1.0);
  u64 my_window = intern_string("my window");
  margin_table_set(padding, my_window, 50.0, 50.0, 50.0,50.0);
  unset_baseclass(my_window, uielement);
  set_baseclass(my_window, window);
  control_add_child(my_window, green_button);

  u64 btn2 = intern_string("2nd button");
  set_baseclass(btn2, green_button);
  margin_table_set(margins, btn2, 200, 100, 100, 100);
  margin_table_set(color, btn2, 1.0, 0.0, 0.0, 1.0);

  control_add_child(my_window, btn2);

  u64 octree_view = intern_string("octree");
  set_baseclass(octree_view, uielement);
  control_add_child(my_window, octree_view);
  define_method2(octree_view, render_method, (method)render_octree_control);

  

  
  iron_gl_backend = IRON_GL_BACKEND_X11;

  
  bool running = true;
  while(running){
    
    call_method(my_window, my_window, render_method);

    gl_window_poll_events();
    gl_window_event events[10];
    for(int cnt = gl_get_events(events, array_count(events)), i = 0; i < cnt; i++){
      if(events[i].type == EVT_WINDOW_CLOSE)
	running = false;
      if(events[i].type == EVT_MOUSE_BTN_DOWN){
	//var evtmousedown = (evt_mouse_btn *) &events[i];
	printf("Mouse down! %i\n", 1);
	
      }
    }
  }
  
  return 0;
}
