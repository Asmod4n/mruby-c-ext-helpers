#include <mruby.h>
#include <mruby/value.h>
#include <mruby/array.h>
#include <mruby/hash.h>
#include <mruby/class.h>
#include <mruby/presym.h>
#include <mruby/string.h>
#include <cassert>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <chrono>
#include <any>
#include <mruby/mrb_value_to_cpp.hpp>
#include <mruby/mrb_convert_cpp_value.hpp>

static void run_value_to_cpp_tests(mrb_state* mrb) {
    // --- Scalars ---
    mrb_value i = mrb_convert_cpp_value(mrb, 42);
    std::any ai = mrb_value_to_any(mrb, i);
    assert(std::any_cast<mrb_int>(ai) == 42);

    mrb_value f = mrb_convert_cpp_value(mrb, 3.14);
    std::any af = mrb_value_to_any(mrb, f);
    assert(std::abs(std::any_cast<mrb_float>(af) - 3.14) < 1e-6);

    // --- Boolean ---
    mrb_value t = mrb_convert_cpp_value(mrb, true);
    mrb_value fval = mrb_convert_cpp_value(mrb, false);
    assert(std::any_cast<bool>(mrb_value_to_any(mrb, t)) == true);
    assert(std::any_cast<bool>(mrb_value_to_any(mrb, fval)) == false);

    // --- Nil ---
    mrb_value n = mrb_nil_value();
    std::any an = mrb_value_to_any(mrb, n);
    assert(!an.has_value());

    // --- String ---
    mrb_value s = mrb_str_new_cstr(mrb, "hello");
    std::any as = mrb_value_to_any(mrb, s);
    assert(std::any_cast<std::string>(as) == "hello");

    // --- Symbol ---
    mrb_value sym = mrb_symbol_value(mrb_intern_cstr(mrb, "sym"));
    std::any sym_any = mrb_value_to_any(mrb, sym);
    assert(std::any_cast<std::string>(sym_any) == "sym");

    // --- Array ---
    std::vector<int> vec = {1,2,3};
    mrb_value arr = mrb_convert_cpp_value(mrb, vec);
    std::any avec = mrb_value_to_any(mrb, arr);
    auto vec_out = std::any_cast<std::vector<std::any>>(avec);
    assert(std::any_cast<mrb_int>(vec_out[0]) == 1);

    // --- Hash ---
    std::map<std::string,int> m = {{"a", 1}, {"b", 2}};
    mrb_value h = mrb_convert_cpp_value(mrb, m);
    std::any ah = mrb_value_to_any(mrb, h);
    auto map_out = std::any_cast<std::map<MapKey,std::any>>(ah);
    assert(std::any_cast<mrb_int>(map_out[std::string("a")]) == 1);

    // --- Set ---
    std::set<std::string> sset = {"x","y"};
    mrb_value ruby_set = mrb_convert_cpp_value(mrb, sset);
    std::any aset = mrb_value_to_any(mrb, ruby_set);
    auto set_vec = std::any_cast<std::vector<std::any>>(aset);
    assert(set_vec.size() == 2);
}

static void run_cpp_to_mrb_tests(mrb_state* mrb) {
    using namespace std::chrono;

    // --- Booleans ---
    mrb_value t = mrb_convert_cpp_value(mrb, true);
    mrb_value f = mrb_convert_cpp_value(mrb, false);
    assert(mrb_bool(t));
    assert(!mrb_bool(f));

    // --- Arithmetic ---
    mrb_value i = mrb_convert_cpp_value(mrb, 123);
    assert(mrb_integer(i) == 123);
    mrb_value d = mrb_convert_cpp_value(mrb, 3.1415);
    assert(std::abs(mrb_float(d) - 3.1415) < 1e-6);

    // --- Strings ---
    mrb_value s1 = mrb_convert_cpp_value(mrb, std::string("foo"));
    mrb_value s2 = mrb_convert_cpp_value(mrb, std::string_view("bar"));
    mrb_value s3 = mrb_convert_cpp_value(mrb, "baz");
    assert(std::string(RSTRING_PTR(s1), RSTRING_LEN(s1)) == "foo");
    assert(std::string(RSTRING_PTR(s2), RSTRING_LEN(s2)) == "bar");
    assert(std::string(RSTRING_PTR(s3), RSTRING_LEN(s3)) == "baz");

    // --- nullptr ---
    mrb_value n = mrb_convert_cpp_value(mrb, nullptr);
    assert(mrb_nil_p(n));

    // --- map-like ---
    std::map<std::string,int> smap = {{"a",1},{"b",2}};
    mrb_value hash_val = mrb_convert_cpp_value(mrb, smap);
    assert(mrb_type(hash_val) == MRB_TT_HASH);
    assert(mrb_integer(mrb_hash_get(mrb, hash_val, mrb_str_new_cstr(mrb,"a"))) == 1);

    std::unordered_map<std::string,bool> umap = {{"x",true},{"y",false}};
    mrb_value uval = mrb_convert_cpp_value(mrb, umap);
    assert(mrb_bool(mrb_hash_get(mrb, uval, mrb_str_new_cstr(mrb,"x"))));

    // --- set-like ---
    std::set<int> sset = {10,20};
    mrb_value sset_val = mrb_convert_cpp_value(mrb, sset);
    struct RClass* set_cls = mrb_class_get_id(mrb, MRB_SYM(Set));
    assert(mrb_obj_is_kind_of(mrb, sset_val, set_cls));

    std::unordered_set<std::string> uset = {"foo","bar"};
    mrb_value uset_val = mrb_convert_cpp_value(mrb, uset);
    assert(mrb_obj_is_kind_of(mrb, uset_val, set_cls));

    // --- iterable containers ---
    std::vector<int> v = {1,2,3};
    mrb_value vval = mrb_convert_cpp_value(mrb, v);
    assert(mrb_type(vval) == MRB_TT_ARRAY);

    std::array<std::string,2> arr = {"x","y"};
    mrb_value aval = mrb_convert_cpp_value(mrb, arr);
    assert(mrb_type(aval) == MRB_TT_ARRAY);

    // --- time_point ---
    auto now = system_clock::now();
    mrb_value tval = mrb_convert_cpp_value(mrb, now);
    struct RClass* time_cls = mrb_class_get_id(mrb, MRB_SYM(Time));
    assert(mrb_obj_is_kind_of(mrb, tval, time_cls));
}

MRB_BEGIN_DECL
void mrb_mruby_c_ext_helpers_gem_test(mrb_state* mrb) {
    run_value_to_cpp_tests(mrb);
    run_cpp_to_mrb_tests(mrb);
}
MRB_END_DECL
