typedef void * void_ptr;

typedef enum {
  ALIGN_STRETCH,
  ALIGN_MIN,
  ALIGN_MAX,
}alignment;

u64 intern_aggregate(u64 intern1, u64 intern2);
u64 intern_string(const char * name);
u64 id_new();

