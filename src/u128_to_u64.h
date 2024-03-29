// This file is auto generated by icy-table.
#include "icydb.h"
typedef struct _u128_to_u64{
  char ** column_names;
  char ** column_types;
  size_t count;
  const bool is_multi_table;
  const int column_count;
  int (*cmp) (const u128 * k1, const u128 * k2);
  const size_t sizes[2];

  u128 * key;
  u64 * value;
  icy_mem * key_area;
  icy_mem * value_area;
}u128_to_u64;

u128_to_u64 * u128_to_u64_create(const char * optional_name);
void u128_to_u64_set(u128_to_u64 * table, u128 key, u64 value);
void u128_to_u64_insert(u128_to_u64 * table, u128 * key, u64 * value, size_t count);
void u128_to_u64_lookup(u128_to_u64 * table, u128 * keys, size_t * out_indexes, size_t count);
void u128_to_u64_remove(u128_to_u64 * table, u128 * keys, size_t key_count);
void u128_to_u64_clear(u128_to_u64 * table);
void u128_to_u64_unset(u128_to_u64 * table, u128 key);
bool u128_to_u64_try_get(u128_to_u64 * table, u128 * key, u64 * value);
void u128_to_u64_print(u128_to_u64 * table);
size_t u128_to_u64_iter(u128_to_u64 * table, u128 * keys, size_t keycnt, u128 * optional_keys_out, size_t * indexes, size_t cnt, size_t * iterator);
