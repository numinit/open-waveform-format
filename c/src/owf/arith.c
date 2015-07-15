#include <owf/arith.h>
#include <owf/error.h>
#include <owf/platform.h>

bool owf_arith_safe_add32(uint32_t a, uint32_t b, uint32_t *result, owf_error_t *error) {
    uint64_t ret = a + b;
    if (OWF_NOEXPECT(ret > UINT32_MAX)) {
        OWF_ERROR_SETF(error, "unsigned 32-bit addition overflow (" OWF_PRINT_U32 " + " OWF_PRINT_U32 ")", a, b);
        return false;
    } else {
        *result = (uint32_t)ret;
        return true;
    }
}

bool owf_arith_safe_sub32(uint32_t a, uint32_t b, uint32_t *result, owf_error_t *error) {
    if (OWF_NOEXPECT(b > a)) {
        OWF_ERROR_SETF(error, "unsigned 32-bit subtraction underflow (" OWF_PRINT_U32 " - " OWF_PRINT_U32 ")", a, b);
        return false;
    } else {
        *result = a - b;
        return true;
    }
}

bool owf_arith_safe_mul32(uint32_t a, uint32_t b, uint32_t *result, owf_error_t *error) {
    uint64_t ret = a * b;
    if (OWF_NOEXPECT(ret > UINT32_MAX)) {
        OWF_ERROR_SETF(error, "unsigned 32-bit multiplication overflow (" OWF_PRINT_U32 " * " OWF_PRINT_U32 ")", a, b);
        return false;
    } else {
        *result = (uint32_t)ret;
        return true;
    }
}
