#include <mruby/mrb_value_to_cpp.hpp>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/hash.h>
#include <string>
#include <mruby/presym.h>
#include <mruby/branch_pred.h>
#include <mruby/numeric.h>

MRB_API std::vector<std::any>
mrb_array_to_vector(mrb_state* mrb, mrb_value ary)
{
    switch (mrb_type(ary)) {
        case MRB_TT_ARRAY:
        case MRB_TT_STRUCT:
            break;
        default: mrb_raise(mrb, E_TYPE_ERROR, "not an array or struct");
    }
    mrb_int len = RARRAY_LEN(ary);
    std::vector<std::any> out;
    out.reserve(len);
    for (mrb_int i = 0; i < len; ++i) {
        out.push_back(mrb_value_to_any(mrb, mrb_ary_ref(mrb, ary, i)));
    }
    return out;
}

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
#ifndef MRB_NO_FLOAT
        case MRB_TT_FLOAT:
            return mrb_float(val);
#endif
        case MRB_TT_INTEGER:
            return mrb_integer(val);
        case MRB_TT_STRING:
            return std::string(RSTRING_PTR(val), RSTRING_LEN(val));
#ifdef MRB_USE_BIGINT
        case MRB_TT_BIGINT: {
            mrb_value s = mrb_integer_to_str(mrb, val, 10);
            return std::string(RSTRING_PTR(s), RSTRING_LEN(s));
        }
#endif
        default:
            mrb_raise(mrb, E_TYPE_ERROR, "Unsupported or unhandled mrb_value type for map key");
    }
}

MRB_API std::map<MapKey, std::any>
mrb_hash_to_map(mrb_state* mrb, mrb_value hash)
{
    if(unlikely(!mrb_hash_p(hash))) mrb_raise(mrb, E_TYPE_ERROR, "not a hash");
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
#ifndef MRB_NO_FLOAT
        case MRB_TT_FLOAT:
            return mrb_float(val);
#endif
        case MRB_TT_INTEGER:
            return mrb_integer(val);
        case MRB_TT_HASH:
            return mrb_hash_to_map(mrb, val);
        case MRB_TT_STRING:
            return std::string(RSTRING_PTR(val), RSTRING_LEN(val));
        case MRB_TT_ARRAY:
        case MRB_TT_STRUCT:
            return mrb_array_to_vector(mrb, val);
#ifdef MRB_USE_BIGINT
        case MRB_TT_BIGINT: {
            mrb_value s = mrb_integer_to_str(mrb, val, 10);
            return std::string(RSTRING_PTR(s), RSTRING_LEN(s));
        }
#endif
#ifdef MRB_USE_SET
        case MRB_TT_SET: {
            mrb_value ary = mrb_funcall_id(mrb, val, MRB_SYM(to_a), 0);
            return mrb_array_to_vector(mrb, ary);
        }
#endif
        default:
            mrb_raise(mrb, E_TYPE_ERROR, "Unsupported or unhandled mrb_value type");
    }
}
