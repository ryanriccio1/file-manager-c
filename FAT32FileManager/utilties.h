#pragma once

/**
 * @brief Get the human readable size
 *
 * @param size Number bytes
 * @return const char* Human readable size
 */
const char *get_human_readable_size(size_t size);
/**
 * @brief Terminate a non-null-terminated string by removing trailing spaces
 *
 * @param buffer String to modify
 */
void remove_trailing_spaces(const char *buffer);
/**
 * @brief Guess the path separator being used from input
 *
 * @param path Input to inspect
 * @return char Path separator
 */
char get_path_separator(const char *path);
