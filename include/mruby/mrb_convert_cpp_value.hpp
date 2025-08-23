#include "cpp_to_mrb_value.hpp"

template <typename T>
constexpr MRB_API mrb_value mrb_convert_cpp_value(mrb_state* mrb, T&& val) {
  return mrbcpp::value_converter::mrb_converter<std::decay_t<T>>::convert(mrb, std::forward<T>(val));
}