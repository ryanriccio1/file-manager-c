#pragma once

#include <stdio.h>
#include <stdint.h>

// macro to print the filename and line number for each error message
// #define debug(msg) fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, msg);
#define debug(msg)

/**
 * @brief Get a string from the cmdline and return a dynamic pointer from heap
 *
 * @return char*
 */
char *get_line_dynamic();

/**
 * @brief Convert string to long and store in result
 *
 * @param buffer String to use in conversion
 * @param result Result of conversion
 * @return uint8_t
 */
uint8_t string_to_long(char *buffer, int64_t *result);
/**
 * @brief Convert string to int and store in result
 *
 * @param buffer String to use in conversion
 * @param result Result of conversion
 * @return uint8_t
 */
uint8_t string_to_int(char *buffer, int32_t *result);
/**
 * @brief Convert string to double and store in result
 *
 * @param buffer String to use in conversion
 * @param result Result of conversion
 * @return uint8_t
 */
uint8_t string_to_double(char *buffer, double *result);
/**
 * @brief Convert string to float and store in result
 *
 * @param buffer String to use in conversion
 * @param result Result of conversion
 * @return uint8_t
 */
uint8_t string_to_float(char *buffer, float *result);
