#pragma once
#include <mruby.h>
#include <mruby/data.h>
#include <new>

template <typename T, typename... Args>
MRB_API T* mrb_cpp_new(mrb_state* mrb, mrb_value self, const struct mrb_data_type *data_type, Args&&... args) {
  T* mem = static_cast<T*>(mrb_malloc(mrb, sizeof(T)));
  mrb_data_init(self, mem, data_type);

  return new (mem) T(std::forward<Args>(args)...);
}

template <typename T>
MRB_API void mrb_cpp_delete(mrb_state* mrb, T* ptr) {
  ptr->~T();
  mrb_free(mrb, ptr);
}