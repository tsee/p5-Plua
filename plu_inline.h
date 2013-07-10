#ifndef PLU_INLINE_H_
#define PLU_INLINE_H_

/* Setup aliases for "static inline" for portability. */

#include <perl.h>

/* Alias Perl's */
#define PLU_STATIC STATIC

#if defined(NDEBUG)
#  if defined(_MSC_VER)
#    define PLU_STATIC_INLINE STATIC __inline
#  else
#    define PLU_STATIC_INLINE STATIC inline
#  endif
#else
   /* avoid inlining under debugging */
#  define PLU_STATIC_INLINE STATIC
#endif

#endif
