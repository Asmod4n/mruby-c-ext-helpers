#pragma once
#include <mruby.h>
#include <mruby/array.h>
#include <mruby/hash.h>
#include <mruby/presym.h>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <iterator>
#include <type_traits>
#include "branch_pred.h"
#include <chrono>

namespace mrbcpp::value_converter {
  template <typename T, typename = void>
  struct is_iterable : std::false_type {};

  template <typename T>
  struct is_iterable<T, std::void_t<
    decltype(std::begin(std::declval<T>())),
    decltype(std::end(std::declval<T>()))>> : std::true_type {};

  template <typename T>
  constexpr bool is_iterable_v = is_iterable<T>::value;

  template <typename T>
  struct is_map_like : std::false_type {};

  template <typename Key, typename Val>
  struct is_map_like<std::map<Key, Val>> : std::true_type {};
  template <typename Key, typename Val>
  struct is_map_like<std::unordered_map<Key, Val>> : std::true_type {};

  template <typename T>
  constexpr bool is_map_like_v = is_map_like<T>::value;

  template <typename T>
  struct is_set_like : std::false_type {};

  template <typename Val>
  struct is_set_like<std::set<Val>> : std::true_type {};
  template <typename Val>
  struct is_set_like<std::unordered_set<Val>> : std::true_type {};

  template <typename T>
  constexpr bool is_set_like_v = is_set_like<T>::value;

  template <typename Clock, typename Duration>
  std::chrono::system_clock::time_point to_system_time(std::chrono::time_point<Clock, Duration> tp) {
    return std::chrono::system_clock::time_point(
      std::chrono::duration_cast<std::chrono::system_clock::duration>(tp.time_since_epoch())
    );
  }

  template <typename T>
  struct is_time_point : std::false_type {};

  template <typename Clock, typename Duration>
  struct is_time_point<std::chrono::time_point<Clock, Duration>> : std::true_type {};

  template <typename T>
  constexpr bool is_time_point_v = is_time_point<T>::value;


  template <typename T>
  struct mrb_converter {
    static constexpr mrb_value convert(mrb_state* mrb, const T& val) {
      if constexpr (std::is_same_v<T, bool>) {
        return mrb_bool_value(val);
      } else if constexpr (std::is_arithmetic_v<T>) {
        return mrb_convert_number(mrb, val);
      } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>) {
        return mrb_str_new(mrb, val.data(), val.size());
      } else if constexpr (std::is_same_v<T, const char*>) {
        return mrb_str_new_cstr(mrb, val);
      } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
        return mrb_nil_value();
      } else if constexpr (is_map_like_v<T>) {
        mrb_value hash = mrb_hash_new(mrb);
        int arena_index = mrb_gc_arena_save(mrb);
        for (const auto& [k, v] : val) {
          mrb_hash_set(mrb, hash,
            mrb_convert_cpp_value(mrb, k),
            mrb_convert_cpp_value(mrb, v));
          mrb_gc_arena_restore(mrb, arena_index);
        }
        return hash;
      } else if constexpr (is_set_like_v<T>) {
        struct RClass* set_class = mrb_class_get_id(mrb, MRB_SYM(Set));
        if (unlikely(!set_class)) {
          mrb_raise(mrb, E_NAME_ERROR, "Set class not defined — is it included in your mruby build");
        }

        mrb_value ruby_set = mrb_obj_new(mrb, set_class, 0, nullptr);
        int arena_index = mrb_gc_arena_save(mrb);
        for (const auto& item : val) {
          mrb_funcall_id(mrb, ruby_set, MRB_SYM(add), 1, mrb_convert_cpp_value(mrb, item));
          mrb_gc_arena_restore(mrb, arena_index);
        }
        return ruby_set;
      } else if constexpr (is_iterable_v<T>) {
        mrb_value ary = mrb_ary_new_capa(mrb, static_cast<mrb_int>(std::size(val)));
        int arena_index = mrb_gc_arena_save(mrb);
        for (const auto& item : val) {
          mrb_ary_push(mrb, ary, mrb_convert_cpp_value(mrb, item));
          mrb_gc_arena_restore(mrb, arena_index);
        }
        return ary;
      } else if constexpr (is_time_point_v<T>) {
        using namespace std::chrono;

        auto sys_tp = to_system_time(val);
        time_t time = system_clock::to_time_t(sys_tp);
        auto duration = sys_tp.time_since_epoch();
        auto micros = duration_cast<microseconds>(duration).count() % 1000000;

        mrb_value sec = mrb_convert_number(mrb, time);
        mrb_value usec = mrb_convert_number(mrb, micros);

        struct RClass* time_class = mrb_class_get_id(mrb, MRB_SYM(Time));
        if (unlikely(!time_class)) {
          mrb_raise(mrb, E_NAME_ERROR, "Time class not defined — is it included in your mruby build?");
        }

        return mrb_funcall_id(mrb, mrb_obj_value(time_class), MRB_SYM(at), 2, sec, usec);
      } else {
        static_assert(sizeof(T) == 0, "Type not supported by mrb_converter");
      }
    }
  };

}

template <typename T>
constexpr MRB_API mrb_value mrb_convert_cpp_value(mrb_state* mrb, T&& val) {
  return mrbcpp::value_converter::mrb_converter<std::decay_t<T>>::convert(mrb, std::forward<T>(val));
}
