#include <io.h>

typedef int int32_t;
//#define int32_t int
#define NULL    ((void *)0)
struct random_data
{
    int32_t *fptr;		/* Front pointer.  */
    int32_t *rptr;		/* Rear pointer.  */
    int32_t *state;		/* Array of state values.  */
    int rand_type;		/* Type of random number generator.  */
    int rand_deg;		/* Degree of random number generator.  */
    int rand_sep;		/* Distance between front and rear.  */
    int32_t *end_ptr;		/* Pointer behind state table.  */
};

/*
 * Not required as the empire scheduler ensure only
 * one thread is active at a time
 */
#define __libc_lock_lock(lock)
#define __libc_lock_unlock(lock)
#define __libc_lock_define_initialized(static1, lock)

#define __set_errno(error) _set_errno((error))

#define weak_alias(arg1, arg2)
#define srandom_r(seed, buf) __srandom_r((seed), (buf))
#define random_r(buf, result) __random_r((buf), (result))
#define initstate_r(seed, state, size, buf) \
    __initstate_r((seed), (state), (size), (buf))
#define setstate_r(state, buf) __setstate_r((state), (buf))

extern int __random_r (struct random_data *__restrict __buf,
		     int32_t *__restrict __result);
extern int __srandom_r (unsigned int __seed, struct random_data *__buf);
extern int __initstate_r (unsigned int __seed, char *__restrict __statebuf,
			size_t __statelen,
			struct random_data *__restrict __buf);
extern int __setstate_r (char *__restrict __statebuf,
		       struct random_data *__restrict __buf);
