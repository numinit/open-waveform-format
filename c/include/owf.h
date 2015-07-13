#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#ifndef OWF_H
#define OWF_H

/** The OWF magic word */
#define OWF_MAGIC 0x4f574631UL

/**
 * Min/max
 */
#define OWF_MIN(a, b) (a < b ? a : b)
#define OWF_MAX(a, b) (a > b ? a : b)

#endif /* OWF_H */
