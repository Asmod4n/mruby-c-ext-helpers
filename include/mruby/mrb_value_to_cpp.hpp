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
