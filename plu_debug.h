#ifndef PLU_DEBUG_H_
#define PLU_DEBUG_H_

/* Set up debugging output to be compiled out without assertions */

#ifdef NDEBUG
#  define PLU_DEBUGGING 0
#  define PLU_DEBUG (void)
#  define PLU_DEBUG_1 (void)
#  define PLU_DEBUG_2 (void)
#else
#  define PLU_DEBUGGING 1
#  define PLU_DEBUG(s) printf(s)
#  define PLU_DEBUG_1(s, par1) printf(s, par1)
#  define PLU_DEBUG_2(s, par1, par2) printf(s, par1, par2)
#endif

#endif
