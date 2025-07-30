#pragma once
#include <mruby.h>
#include <mruby/data.h>
#include <new>
#include <iostream>

template <typename T>
struct mrb_data_type_traits;

template <typename T, typename... Args>
MRB_API T* mrb_cpp_new(mrb_state* mrb, mrb_value self, Args&&... args) {
  const mrb_data_type* data_type = mrb_data_type_traits<T>::get();
  T* mem = static_cast<T*>(mrb_malloc(mrb, sizeof(T)));
  mrb_data_init(self, mem, data_type);
  return new (mem) T(std::forward<Args>(args)...);
}

template <typename T>
MRB_API void mrb_cpp_delete(mrb_state* mrb, T* ptr) {
  ptr->~T();
  mrb_free(mrb, ptr);
}

#define MRB_CPP_DEFINE_TYPE(ClassName, Identifier)                           \
  static void Identifier##_free(mrb_state* mrb, void* ptr) {      \
    mrb_cpp_delete<ClassName>(mrb, static_cast<ClassName*>(ptr));\
  }                                                              \
                                                                 \
  static const struct mrb_data_type Identifier##_type = {         \
    #ClassName, Identifier##_free                                 \
  };                                                             \
                                                                 \
  template <>                                                    \
  struct mrb_data_type_traits<ClassName> {                       \
    static const mrb_data_type* get() {                          \
      return &Identifier##_type;                                  \
    }                                                            \
  };
