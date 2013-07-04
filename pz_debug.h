#ifndef PZ_DEBUG_H_
#define PZ_DEBUG_H_

/* Set up debugging output to be compiled out without assertions */

#ifdef NDEBUG
#  define PZ_DEBUGGING 0
#  define PZ_DEBUG (void)
#  define PZ_DEBUG_1 (void)
#  define PZ_DEBUG_2 (void)
#else
#  define PZ_DEBUGGING 1
#  define PZ_DEBUG(s) printf(s)
#  define PZ_DEBUG_1(s, par1) printf(s, par1)
#  define PZ_DEBUG_2(s, par1, par2) printf(s, par1, par2)
#endif

#endif
