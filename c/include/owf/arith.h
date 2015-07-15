#include <owf.h>
#include <owf/error.h>

#ifndef OWF_ARITH_H
#define OWF_ARITH_H

/* Performs a safe add of two unsigned 32-bit values.
 * @a The first value
 * @b The second value
 * @result Where to store the result of the addition if successful (a + b)
 * @error A pointer to an owf_error_t to store potential errors (overflow)
 *
 * @return True if successful, false otherwise. If successful, changes the value of *result.
 */
bool owf_arith_safe_add32(uint32_t a, uint32_t b, uint32_t *result, owf_error_t *error);

/* Performs a safe subtract of two 32-bit values.
 * @a The first value
 * @b The second value
 * @result Where to store the result of the addition if successful (a - b)
 * @error A pointer to an owf_error_t to store potential errors (underflow)
 *
 * @return True if successful, false otherwise. If successful, changes the value of *result.
 */
bool owf_arith_safe_sub32(uint32_t a, uint32_t b, uint32_t *result, owf_error_t *error);

/* Performs a safe multiply of two 32-bit values.
 * @a The first value
 * @b The second value
 * @result Where to store the result of the multiplication if successful (a * b)
 * @error A pointer to an owf_error_t to store potential errors (overflow)
 *
 * @return True if successful, false otherwise. If successful, changes the value of *result.
 */
bool owf_arith_safe_mul32(uint32_t a, uint32_t b, uint32_t *result, owf_error_t *error);

#define OWF_ARITH_SAFE_ADD32(_error, _a, _b) \
    do { \
        if (OWF_NOEXPECT(!owf_arith_safe_add32(_a, _b, &(_a), _error))) { \
            return false; \
        } \
    } while (0)

#define OWF_ARITH_SAFE_SUB32(_error, _a, _b) \
    do { \
        if (OWF_NOEXPECT(!owf_arith_safe_sub32(_a, _b, &(_a), _error))) { \
            return false; \
        } \
    } while (0)

#define OWF_ARITH_SAFE_MUL32(_error, _a, _b) \
    do { \
        if (OWF_NOEXPECT(!owf_arith_safe_mul32(_a, _b, &(_a), _error))) { \
            return false; \
        } \
    } while (0)

#endif /* OWF_ARITH_H */
