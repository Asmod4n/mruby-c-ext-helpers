#pragma once
#include <mruby.h>
#include <mruby/data.h>
#include <new>
#include <type_traits>
#include <array>
#include <cstddef>

template <typename T, typename Enable = void>
struct mrb_data_type_traits;

template <typename T, typename... Args>
T* mrb_cpp_new(mrb_state* mrb, mrb_value self, Args&&... args) {
  const mrb_data_type* dt = mrb_data_type_traits<T>::get();
  T* mem = static_cast<T*>(mrb_malloc(mrb, sizeof(T)));
  mrb_data_init(self, mem, dt);
  return new (mem) T(std::forward<Args>(args)...);
}

template <typename T>
void mrb_cpp_delete(mrb_state* mrb, T* ptr) {
  ptr->~T();
  mrb_free(mrb, ptr);
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
    static const mrb_data_type* get() {                                           \
      return &Identifier##_type;                                                  \
    }                                                                             \
  };                                                                              \
                                                                                  \
  /* Any subclass of BaseClass */                                                 \
  template <typename T>                                                           \
  struct mrb_data_type_traits<                                                    \
    T, std::enable_if_t<std::is_base_of<BaseClass, T>::value &&                  \
                        !std::is_same<BaseClass, T>::value>> {                    \
    static const mrb_data_type* get() {                                           \
      return &Identifier##_type;                                                  \
    }                                                                             \
  };
