#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <emscripten.h>
#include <iron/types.h>
#include <iron/linmath.h>
#include <iron/log.h>
#include <iron/utils.h>
#include <iron/time.h>
#include <iron/gl.h>
#include "main.h"
#include "u64_table.h"
typedef void * void_ptr;
#include "u64_to_ptr.h"
#include "u64_to_vec2_table.h"
#include "u64_to_f32_table.h"
#include "oop.h"

#include "alignments.h"
#include "margin_table.h"
#include "gui.h"

__thread int logd_enable = 1;

typedef struct{
u64 my_window;
}context;

void * x11_create_backend(){
  return NULL;
}


void do_mainloop(context * ctx){

  u64 my_window = ctx->my_window;

  call_method(my_window, my_window, render_method);

  return;
  gl_window_poll_events();
  gl_window_event events[10];
  for(int cnt = gl_get_events(events, array_count(events)), i = 0; i < cnt; i++){
    if(events[i].type == EVT_MOUSE_BTN_DOWN){
      //var evtmousedown = (evt_mouse_btn *) &events[i];
      printf("Mouse down! %i\n", 1);
      
    }
  }
}


int main(){
  icy_mem_emulate_memory_maps = true;
  oop_init();
  gui_init();

  
  context * ctx = calloc(sizeof(context), 1);
  int simulate_infinite_loop = 1;
  int fps = -1;


  
  u64 my_window = intern_string("my window");
  logd("1?\n");
  margin_table_set(padding, my_window, 50.0, 50.0, 50.0,50.0);
  logd("1?\n");
  set_baseclass(my_window, class_window);
  logd("1?\n");
  control_add_child(my_window, class_green_button);
  logd("2?\n");
  logd("Window: %i\n", my_window);
  
  
  u64 btn2 = intern_string("2nd button");
  set_baseclass(btn2, class_green_button);
  margin_table_set(margins, btn2, 200, 100, 100, 100);
  margin_table_set(color, btn2, 1.0, 0.0, 0.0, 1.0);

  control_add_child(my_window, btn2);
  
  u64 octree_view = intern_string("octree");
  set_baseclass(octree_view, class_uielement);
  control_add_child(my_window, octree_view);
  define_method2(octree_view, render_method, (method)render_octree_control);
  ctx->my_window = my_window;
  emscripten_set_main_loop_arg((void *) &do_mainloop, ctx, fps, simulate_infinite_loop);
  return 0;
}
