#pragma once
#include <mruby.h>
#include <any>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <stdexcept>

MRB_API std::any mrb_value_to_any(mrb_state* mrb, mrb_value val);
MRB_API std::vector<std::any> mrb_array_to_vector(mrb_state* mrb, mrb_value ary);

// Map keys can be int64_t, double, or string
using MapKey = std::variant<int64_t, double, std::string>;
MRB_API std::map<MapKey, std::any> mrb_hash_to_map(mrb_state* mrb, mrb_value hash);
