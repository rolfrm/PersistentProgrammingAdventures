 
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


void test_octree_algorithm();

int main(int argc, char ** argv){

  oop_init();
  gui_init();
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

  
   u64 my_window = intern_string("my window");
  margin_table_set(padding, my_window, 50.0, 50.0, 50.0,50.0);
  set_baseclass(my_window, class_window);
  control_add_child(my_window, class_green_button);

  u64 btn2 = intern_string("2nd button");
  set_baseclass(btn2, class_green_button);
  margin_table_set(margins, btn2, 200, 100, 100, 100);
  margin_table_set(color, btn2, 1.0, 0.0, 0.0, 1.0);

  control_add_child(my_window, btn2);

  u64 octree_view = intern_string("octree");
  set_baseclass(octree_view, class_uielement);
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
