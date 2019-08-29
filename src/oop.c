#include <iron/full.h>
#include "main.h"
#include <iron/gl.h>
#include <GL/gl.h>
//#include <GL/glew.h>
#include "u64_table.h"
typedef void * void_ptr;
#include "u64_to_ptr.h"
#include "u64_pair_to_u64.h"
#include "u64_to_vec2_table.h"
#include "u64_to_f32_table.h"

#include "oop.h"

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

u64_pair_to_u64 * class_method_table;
u64_to_ptr * method_table;

typedef union{
  struct{
    u64 class;
    u64 method;
  };
  u64_pair index;
}class_method;

bool has_method(u64 object, u64 method_id, u64 handler){
  class_method cls = { .class = object, .method = method_id};
  size_t indexes[10];
  size_t count, index = 0;

  
  while((count = u64_pair_to_u64_iter(class_method_table, &cls.index, 1, NULL, indexes, array_count(indexes), &index)) > 0){
    for(size_t i = 0; i < count; i++){
      if(class_method_table->value[indexes[i]] == handler)
	return true;
    }
  }
  return false;

}

void set_method(u64 class, u64 method_id, u64 handler){
  class_method cls = { .class = class, .method = method_id};
  printf("Setting handler.. %i %i %i\n", class, method_id, handler);
  u64_pair_to_u64_set(class_method_table, cls.index, handler);
}

u64 get_method(u64 class, u64 method_id){
  class_method cls;
  cls.method = method_id;
  cls.class = class;
  u64 method = 0;
  if(u64_pair_to_u64_try_get(class_method_table, &cls.index, &method))
    return method;
  return 0;
}

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


void oop_init(){
  class_method_table = u64_pair_to_u64_create("class methods");
  ((bool *) (&class_method_table->is_multi_table))[0] = true;
  method_table = u64_to_ptr_create(NULL);
}
