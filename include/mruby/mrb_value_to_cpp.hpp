#pragma once
#include <mruby.h>
#include <any>
#include <string>
#include <vector>
#include <map>
#include <variant>

using MapKey = std::variant<mrb_int, mrb_float, std::string>;

MRB_API std::any mrb_value_to_any(mrb_state* mrb, mrb_value val);
MRB_API std::vector<std::any> mrb_array_to_vector(mrb_state* mrb, mrb_value ary);
MRB_API std::map<MapKey, std::any> mrb_hash_to_map(mrb_state* mrb, mrb_value hash);

// Typed conversion: the mirror of cpp_to_mrb_value. mrb_value_to<T>(mrb, v)
// makes a T from a Ruby value, or raises TypeError/ArgumentError through
// mruby when the value has no such form. A class with mrb_data_type_traits
// is taken from its data object as it is.
#include <mruby/array.h>
#include <mruby/hash.h>
#include <mruby/numeric.h>
#include <mruby/string.h>
#include <mruby/error.h>
#include <array>
#include <chrono>
#include <optional>
#include <set>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "cpp_helpers.hpp"
#include "cpp_to_mrb_value.hpp"

namespace mrbcpp::value_converter {
  template <typename T> struct is_std_array : std::false_type {};
  template <typename E, std::size_t N> struct is_std_array<std::array<E, N>> : std::true_type {};
  template <typename T> struct is_std_vector : std::false_type {};
  template <typename E, typename A> struct is_std_vector<std::vector<E, A>> : std::true_type {};
  template <typename T> struct is_std_optional : std::false_type {};
  template <typename E> struct is_std_optional<std::optional<E>> : std::true_type {};
  template <typename T> struct is_std_pair : std::false_type {};
  template <typename A, typename B> struct is_std_pair<std::pair<A, B>> : std::true_type {};
  template <typename T, typename = void> struct has_data_type : std::false_type {};
  template <typename T> struct has_data_type<T, std::void_t<decltype(mrb_data_type_traits<T>::get())>> : std::true_type {};

  template <typename T>
  struct from_mrb {
    static T convert(mrb_state* mrb, mrb_value v) {
      if constexpr (std::is_same_v<T, mrb_value>) {
        return v;
      } else if constexpr (std::is_same_v<T, bool>) {
        return mrb_test(v);
      } else if constexpr (std::is_integral_v<T>) {
        return static_cast<T>(mrb_as_int(mrb, v));
      } else if constexpr (std::is_floating_point_v<T>) {
        return static_cast<T>(mrb_as_float(mrb, v));
      } else if constexpr (std::is_same_v<T, std::string>) {
        mrb_value s = mrb_ensure_string_type(mrb, v);
        return std::string(RSTRING_PTR(s), static_cast<std::size_t>(RSTRING_LEN(s)));
      } else if constexpr (std::is_same_v<T, std::string_view>) {
        mrb_value s = mrb_ensure_string_type(mrb, v);
        return std::string_view(RSTRING_PTR(s), static_cast<std::size_t>(RSTRING_LEN(s)));
      } else if constexpr (is_std_optional<T>::value) {
        if (mrb_nil_p(v)) return std::nullopt;
        return from_mrb<typename T::value_type>::convert(mrb, v);
      } else if constexpr (is_std_pair<T>::value) {
        mrb_value a = mrb_ensure_array_type(mrb, v);
        if (RARRAY_LEN(a) != 2) mrb_raise(mrb, E_ARGUMENT_ERROR, "pair wants an array of two");
        return T(from_mrb<std::remove_cv_t<typename T::first_type>>::convert(mrb, mrb_ary_ref(mrb, a, 0)),
                 from_mrb<std::remove_cv_t<typename T::second_type>>::convert(mrb, mrb_ary_ref(mrb, a, 1)));
      } else if constexpr (is_std_array<T>::value) {
        mrb_value a = mrb_ensure_array_type(mrb, v);
        T out{};
        if (static_cast<std::size_t>(RARRAY_LEN(a)) != out.size()) mrb_raisef(mrb, E_ARGUMENT_ERROR, "array wants %d elements, given %d", static_cast<int>(out.size()), static_cast<int>(RARRAY_LEN(a)));
        for (std::size_t i = 0; i < out.size(); i++) out[i] = from_mrb<typename T::value_type>::convert(mrb, mrb_ary_ref(mrb, a, static_cast<mrb_int>(i)));
        return out;
      } else if constexpr (is_std_vector<T>::value) {
        mrb_value a = mrb_ensure_array_type(mrb, v);
        T out;
        out.reserve(static_cast<std::size_t>(RARRAY_LEN(a)));
        for (mrb_int i = 0; i < RARRAY_LEN(a); i++) out.push_back(from_mrb<typename T::value_type>::convert(mrb, mrb_ary_ref(mrb, a, i)));
        return out;
      } else if constexpr (is_map_like_v<T>) {
        mrb_value h = mrb_ensure_hash_type(mrb, v);
        T out;
        mrb_value keys = mrb_hash_keys(mrb, h);
        for (mrb_int i = 0; i < RARRAY_LEN(keys); i++) {
          mrb_value k = mrb_ary_ref(mrb, keys, i);
          out.emplace(from_mrb<typename T::key_type>::convert(mrb, k), from_mrb<typename T::mapped_type>::convert(mrb, mrb_hash_get(mrb, h, k)));
        }
        return out;
      } else if constexpr (is_set_like_v<T>) {
        mrb_value a = mrb_ensure_array_type(mrb, mrb_funcall_id(mrb, v, MRB_SYM(to_a), 0));
        T out;
        for (mrb_int i = 0; i < RARRAY_LEN(a); i++) out.insert(from_mrb<typename T::value_type>::convert(mrb, mrb_ary_ref(mrb, a, i)));
        return out;
      } else if constexpr (is_time_point_v<T>) {
        mrb_value f = mrb_funcall_id(mrb, v, MRB_SYM(to_f), 0);
        return T(std::chrono::duration_cast<typename T::duration>(std::chrono::duration<double>(mrb_as_float(mrb, f))));
      } else if constexpr (has_data_type<T>::value) {
        return *mrb_cpp_get<T>(mrb, v);
      } else {
        static_assert(sizeof(T) == 0, "no conversion from mrb_value to this type");
      }
    }
  };
}

namespace mrbcpp::value_converter {
  template <typename T>
  constexpr bool convertible_from_mrb =
    std::is_const_v<T> || std::is_volatile_v<T> ? false :
    std::is_same_v<T, mrb_value> || std::is_arithmetic_v<T> || std::is_same_v<T, std::string> ||
    std::is_same_v<T, std::string_view> || is_std_optional<T>::value || is_std_pair<T>::value ||
    is_std_array<T>::value || is_std_vector<T>::value || is_map_like_v<T> || is_set_like_v<T> ||
    is_time_point_v<T> || has_data_type<T>::value;
}

template <typename T>
T mrb_value_to(mrb_state* mrb, mrb_value v) {
  return mrbcpp::value_converter::from_mrb<T>::convert(mrb, v);
}
