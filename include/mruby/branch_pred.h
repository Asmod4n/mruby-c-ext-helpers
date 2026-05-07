#pragma once

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_expect) \
    || (defined(__GNUC__) && __GNUC__ >= 3) \
    || (defined(__INTEL_COMPILER) && __INTEL_COMPILER >= 800)
#  define likely(x)   __builtin_expect(!!(x), 1)
#  define unlikely(x) __builtin_expect(!!(x), 0)
#else
#  define likely(x)   (x)
#  define unlikely(x) (x)
#endif
