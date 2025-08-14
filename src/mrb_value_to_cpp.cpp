#include <mruby/mrb_value_to_cpp.hpp>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/hash.h>
MRB_BEGIN_DECL
#include <mruby/internal.h>
MRB_END_DECL
#include <stdexcept>
#include <string>
#include <mruby/presym.h>

MRB_API std::vector<std::any>
mrb_array_to_vector(mrb_state* mrb, mrb_value ary)
{
    mrb_int len = RARRAY_LEN(ary);
    std::vector<std::any> out;
    out.reserve(len);
    for (mrb_int i = 0; i < len; ++i) {
        out.push_back(mrb_value_to_any(mrb, mrb_ary_ref(mrb, ary, i)));
    }
    return out;
}

// Convert an mrb_value into a valid map key
inline MapKey mrb_value_to_map_key(mrb_state* mrb, mrb_value val) {
    switch (mrb_type(val)) {
        case MRB_TT_FALSE:
            return (mrb_integer(val) == 0) ? std::string("nil") : std::string("false");
        case MRB_TT_TRUE:
            return std::string("true");
        case MRB_TT_SYMBOL: {
            mrb_int len;
            const char* s = mrb_sym_name_len(mrb, mrb_symbol(val), &len);
            return std::string(s, static_cast<size_t>(len));
        }
        case MRB_TT_UNDEF:
        case MRB_TT_FREE:
            return std::string("undefined");
        case MRB_TT_FLOAT:
            return static_cast<double>(mrb_float(val));
        case MRB_TT_INTEGER:
            return static_cast<int64_t>(mrb_integer(val));
        case MRB_TT_STRING:
            return std::string(RSTRING_PTR(val), RSTRING_LEN(val));
        case MRB_TT_BIGINT: {
            mrb_value s = mrb_bint_to_s(mrb, val, 10);
            return std::string(RSTRING_PTR(s), RSTRING_LEN(s));
        }
        default:
            throw std::runtime_error("Unsupported or unhandled mrb_value type for map key");
    }
}

MRB_API std::map<MapKey, std::any>
mrb_hash_to_map(mrb_state* mrb, mrb_value hash)
{
    std::map<MapKey, std::any> out;
    mrb_value keys = mrb_hash_keys(mrb, hash);
    mrb_int len = RARRAY_LEN(keys);
    for (mrb_int i = 0; i < len; ++i) {
        mrb_value k = mrb_ary_ref(mrb, keys, i);
        mrb_value v = mrb_hash_get(mrb, hash, k);
        out.emplace(mrb_value_to_map_key(mrb, k), mrb_value_to_any(mrb, v));
    }
    return out;
}

MRB_API std::any
mrb_value_to_any(mrb_state* mrb, mrb_value val)
{
    switch (mrb_type(val)) {
        case MRB_TT_FALSE:
            return (mrb_integer(val) == 0) ? std::any{} : false;
        case MRB_TT_TRUE:
            return true;
        case MRB_TT_SYMBOL: {
            mrb_int len;
            const char* s = mrb_sym_name_len(mrb, mrb_symbol(val), &len);
            return std::string(s, static_cast<size_t>(len));
        }
        case MRB_TT_UNDEF:
        case MRB_TT_FREE:
            return std::any{};
        case MRB_TT_FLOAT:
            return mrb_float(val);
        case MRB_TT_INTEGER:
            return mrb_integer(val);
        case MRB_TT_HASH:
            return mrb_hash_to_map(mrb, val);
        case MRB_TT_STRING:
            return std::string(RSTRING_PTR(val), RSTRING_LEN(val));
        case MRB_TT_ARRAY:
        case MRB_TT_STRUCT:
            return mrb_array_to_vector(mrb, val);
        case MRB_TT_BIGINT: {
            mrb_value s = mrb_bint_to_s(mrb, val, 10);
            return std::string(RSTRING_PTR(s), RSTRING_LEN(s));
        }
        case MRB_TT_SET: {
            mrb_value ary = mrb_funcall_id(mrb, val, MRB_SYM(to_a), 0);
            return mrb_array_to_vector(mrb, ary);
        }

        default:
            throw std::runtime_error("Unsupported or unhandled mrb_value type");
    }
}
