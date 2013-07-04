#ifndef PZ_INLINE_H_
#define PZ_INLINE_H_

/* Setup aliases for "static inline" for portability. */

#include <perl.h>

/* Alias Perl's */
#define PZ_STATIC STATIC

#if defined(NDEBUG)
#  if defined(_MSC_VER)
#    define PZ_STATIC_INLINE STATIC __inline
#  else
#    define PZ_STATIC_INLINE STATIC inline
#  endif
#else
   /* avoid inlining under debugging */
#  define PZ_STATIC_INLINE STATIC
#endif

#endif
