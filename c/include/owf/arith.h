#include <owf.h>
#include <owf/platform.h>
#include <owf/error.h>

#ifndef OWF_ARITH_H
#define OWF_ARITH_H

uint32_t owf_arith_safe_add32(uint32_t a, uint32_t b, owf_error_t *error);
uint32_t owf_arith_safe_sub32(uint32_t a, uint32_t b, owf_error_t *error);
uint32_t owf_arith_safe_mul32(uint32_t a, uint32_t b, owf_error_t *error);

#endif /* OWF_ARITH_H */
