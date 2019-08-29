
extern u64_table * class_table;

size_t iterate_subclasses(u64 class, u64 * subclasses, size_t cnt, size_t * it);
size_t u64_lookup_find(u64_table * table, u64 key, u64 value);
bool is_subclass_of(u64 class, u64 baseclass);
void set_baseclass(u64 class, u64 baseclass);
void unset_baseclass(u64 class, u64 baseclass);
typedef void (* method)(u64 control);
bool has_method(u64 object, u64 method_id, u64 handler);
void set_method(u64 class, u64 method_id, u64 handler);
void call_next();
void call_method(u64 object, u64 class, u64 method_id);
void define_method(u64 id, method handler);
void define_method2(u64 class, u64 method_id, method m);
void oop_init();
