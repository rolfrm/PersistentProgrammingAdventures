#include <iron/full.h>

#include "redirection_table.h"
typedef struct{
  redirection_table * redirected;
  u8 * buffer;
  size_t capacity;
  size_t count;
}octree;

typedef struct{
  size_t index;
}octree_index;

const size_t octree_invalid_index = -1;

octree_index octree_index_new(){
  octree_index index = {0};
  return index;
}

octree * octree_new(){
  octree * d = alloc0(sizeof(octree));
  d->buffer = alloc0(128);
  d->capacity = 128;
  d->count = 1;
  d->redirected = redirection_table_create(NULL);
  ((bool *)&d->redirected->is_multi_table)[0] = true;
  return d;
}

octree_index octree_add(octree * d){
  if(d->capacity == d->count){
    d->buffer = ralloc(d->buffer, d->capacity * 2);
    d->capacity *= 2;
  }
  d->count += 1;
  return (octree_index){.index = d->count - 1};
}

size_t octree_sub_size(const octree * tree, octree_index index){
  if(index.index > 7)
    ERROR("Invalid octree index");
  size_t subinfo = tree->buffer[index.index];

  size_t size = 0;
  for(size_t mask = 1; mask <= 0xFF; mask = mask << 1){
    if(subinfo & mask){
      octree_index index2 = (octree_index){size + index.index + 1};
      size_t sub_size = octree_sub_size(tree, index2);
      size += sub_size + 1;
    }
  }
  return size;
}


octree_index octree_index_sub(octree * tree, octree_index index, size_t sub, bool create){
  UNUSED(create);
  if(sub >= 8)
    ERROR("Invalid octree index");
  size_t subinfo = tree->buffer[index.index];
  size_t size = 0;
  size_t indexes[8];
  size_t __index = 0;
  size_t rcount = redirection_table_iter(tree->redirected, &index.index,1,NULL, indexes, 8, &__index);
  for(size_t mask = 1, i = 0; mask <= 0xFF; mask = mask << 1, i++){
    bool ctn = false;
    for(size_t i3 = 0; i3 < rcount; i3++){

	size_t i2 = indexes[i3];
	printf("Check... %i\n", i2);
	if(tree->redirected->index[i2] == i){

	  u64 newindex = tree->redirected->redirection[i2];
	  octree_index index2 = (octree_index){.index = newindex};
	  printf("Redirected... %i\n", index2.index);
	  size_t ns = octree_sub_size(tree, index2);
	  size += ns + 1;
	  ctn = true;
	  break;
	}
    }
    if(ctn) continue;
    if(subinfo & mask){
      octree_index index2 = (octree_index){size + index.index + 1};
      if(i == sub){
	return index2;
      }
      size_t sub_size = octree_sub_size(tree, index2);
      size += sub_size + 1;
    }
    if(i == sub && create){
      octree_index index2 = (octree_index){size + index.index + 1};

      if(tree->count == index2.index){
	printf("resize..\n");
	octree_index index3 = octree_add(tree);
	if(index3.index != index2.index)
	  ERROR("Index should be same!");
	subinfo |= (1 << i);
	tree->buffer[index.index] = subinfo;
	return index3;
      }else{

	var index2 = octree_add(tree);
	redirection_table_set(tree->redirected, index.index, i, index2.index);
	printf("Redirecting... %i\n", index2.index);
	return index2;
      }
      
    }
    
  }
  printf("error finding sub index\n");
  return (octree_index){.index = octree_invalid_index};
}

void test_octree_algorithm(){
  octree * o = octree_new();
  var i = octree_index_new();
  octree_index_sub(o, i, 0, true);
  octree_index_sub(o, i, 1, true);
  octree_index_sub(o, i, 2, true);
  octree_index_sub(o, i, 4, true);
  var i2 = octree_index_sub(o, i, 3, true);
  octree_index_sub(o, i, 5, true);
  octree_index_sub(o, i, 6, true);
  var i3 = octree_index_sub(o, i2, 0, true);
  var i4 = octree_index_sub(o, i3, 1, true);
  var i5 = octree_index_sub(o, i4, 2, true);
  var i6 = octree_index_sub(o, i5, 3, true);
  octree_index_sub(o, i5, 2, true);
  octree_index_sub(o, i5, 1, true);
  octree_index_sub(o, i5, 0, true);
  UNUSED(i6);
  var i7 = octree_index_sub(o, i6, 4, true);
  var i8 = octree_index_sub(o, i7, 2, true);
  octree_index_sub(o, octree_index_sub(o, octree_index_sub(o, i8, 2, true), 2, true), 2, true);
  logd("%i\n", i8.index);

  for(u32 j = 0; j < o->count; j++){
    logd("%i : %i\n", j, o->buffer[j]);
  }
  
  //
  // TODO: When redirection is done, consequent calculations are wrong.
  //

}

