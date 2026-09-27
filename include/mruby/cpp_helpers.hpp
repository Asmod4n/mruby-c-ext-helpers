#pragma once
#include <mruby.h>
#include <mruby/data.h>
#include <memory>
#include <new>
#include "branch_pred.h"
#include <type_traits>
#include <array>
#include <cstddef>

template <typename T, typename Enable = void>
struct mrb_data_type_traits;

template <typename T, typename... Args>
T* mrb_cpp_new(mrb_state* mrb, mrb_value self, Args&&... args) {
  if (unlikely(DATA_PTR(self) != nullptr))
    mrb_raisef(mrb, E_TYPE_ERROR, "already initialized %C", mrb_obj_class(mrb, self));
  const mrb_data_type* dt = mrb_data_type_traits<T>::get();
  T* obj = new T(std::forward<Args>(args)...);
  mrb_data_init(self, static_cast<typename mrb_data_type_traits<T>::base*>(obj), dt);
  return obj;
}

template <typename T>
void mrb_cpp_delete(mrb_state*, T* ptr) {
  delete ptr;
}

// Strip namespaces from a type name
template <std::size_t N>
constexpr auto mrb_cpp_basename(const char (&s)[N]) {
  std::size_t start = 0;
  for (std::size_t i = 0; i + 1 < N; ++i) {
    if (s[i] == ':' && s[i + 1] == ':') {
      start = i + 2;
    }
  }

  std::array<char, N> out{};
  std::size_t j = 0;
  for (std::size_t i = start; i < N - 1; ++i) {
    out[j++] = s[i];
  }
  out[j] = '\0';
  return out;
}

#define MRB_CPP_DEFINE_TYPE(BaseClass, Identifier)                                \
  static void Identifier##_free(mrb_state* mrb, void* ptr) {                      \
    mrb_cpp_delete<BaseClass>(mrb, static_cast<BaseClass*>(ptr));                 \
  }                                                                               \
                                                                                  \
  static constexpr auto Identifier##_name_arr = mrb_cpp_basename(#BaseClass);     \
                                                                                  \
  static const mrb_data_type Identifier##_type = {                                \
    Identifier##_name_arr.data(),                                                 \
    Identifier##_free                                                             \
  };                                                                              \
                                                                                  \
  /* Exact BaseClass */                                                           \
  template <>                                                                      \
  struct mrb_data_type_traits<BaseClass, void> {                                  \
    using base = BaseClass;                                                       \
    static const mrb_data_type* get() {                                           \
      return &Identifier##_type;                                                  \
    }                                                                             \
  };                                                                              \
                                                                                  \
  /* Any subclass of BaseClass */                                                 \
  template <typename T>                                                           \
  struct mrb_data_type_traits<                                                    \
    T, std::enable_if_t<std::is_base_of<BaseClass, T>::value &&                  \
                        !std::is_same<BaseClass, T>::value &&                     \
                        std::has_virtual_destructor<BaseClass>::value>> {         \
    using base = BaseClass;                                                       \
    static const mrb_data_type* get() {                                           \
      return &Identifier##_type;                                                  \
    }                                                                             \
  };

template <typename T>
T* mrb_cpp_get(mrb_state* mrb, mrb_value obj) {
  using base = typename mrb_data_type_traits<T>::base;
  base* b = static_cast<base*>(mrb_data_get_ptr(mrb, obj, mrb_data_type_traits<T>::get()));
  if constexpr (std::is_same_v<T, base>)
    return b;
  else
    return dynamic_cast<T*>(b);
}
