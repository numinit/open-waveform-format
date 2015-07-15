#include <owf.h>

#ifndef OWF_VERSION_H
#define OWF_VERSION_H

/* The included OWF library version.
 * This is a 16-bit integer representing the semantic version of OWF.
 * The most significant nibble is the major version.
 * The two next most significant nibbles are the minor version.
 * The least significant version is the patch version.
 */
#define OWF_VERSION 0x0090

/* The major, minor, and patch versions */
#define OWF_VERSION_MAJOR 0
#define OWF_VERSION_MINOR 9
#define OWF_VERSION_PATCH 0

/* The version, as a string */
#define OWF_VERSION_STRING "0.9.0"

/* Retrieves the version compiled into libowf.
 *
 * @major A pointer to an int to store the major version.
 * @minor A pointer to an int to store the minor version.
 * @patch A pointer to an int to store the patch version.
 */
void owf_version(int *major, int *minor, int *patch);

#endif /* OWF_VERSION_H */
