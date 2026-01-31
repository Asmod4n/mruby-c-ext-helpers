#pragma once
#if ((defined(__has_builtin) && __has_builtin(__builtin_expect))||(__GNUC__ >= 3) || (__INTEL_COMPILER >= 800) || defined(__clang__))
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x)   (x)
#define unlikely(x) (x)
#endif