#include <owf.h>
#include <owf/platform.h>
#include <owf/error.h>

#ifndef OWF_ARITH_H
#define OWF_ARITH_H

uint32_t owf_arith_safe_add32(uint32_t a, uint32_t b, owf_error_t *error);
uint32_t owf_arith_safe_sub32(uint32_t a, uint32_t b, owf_error_t *error);
uint32_t owf_arith_safe_mul32(uint32_t a, uint32_t b, owf_error_t *error);

#define OWF_ARITH_SAFE_ADD32(_error, _a, _b) \
    do { \
        uint32_t _tmp = owf_arith_safe_add32(_a, _b, _error); \
        if (OWF_NOEXPECT(_error->is_error)) { \
            return false; \
        } else { \
            _a = _tmp; \
        } \
    } while (0)

#define OWF_ARITH_SAFE_SUB32(_error, _a, _b) \
    do { \
        uint32_t _tmp = owf_arith_safe_sub32(_a, _b, _error); \
        if (OWF_NOEXPECT(_error->is_error)) { \
            return false; \
        } else { \
            _a = _tmp; \
        } \
    } while (0)

#define OWF_ARITH_SAFE_MUL32(_error, _a, _b) \
    do { \
        uint32_t _tmp = owf_arith_safe_mul32(_a, _b, _error); \
        if (OWF_NOEXPECT(_error->is_error)) { \
            return false; \
        } else { \
            _a = _tmp; \
        } \
    } while (0)

#endif /* OWF_ARITH_H */
