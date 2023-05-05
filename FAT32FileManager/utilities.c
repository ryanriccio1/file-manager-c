#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utilties.h"

const char* get_human_readable_size(size_t size)
{
	// rather than displaying bytes, round to KB, MB, or GB
	const size_t GIGABYTE = 1073741824;
	const size_t MEGABYTE = 1048576;
	const size_t KILOBYTE = 1024;

	char* readable_size = calloc(32, sizeof(char));
	if (size >= GIGABYTE)
		assert(sprintf(readable_size, "%.2lf GB", (double)(size / GIGABYTE)));
	else if (size >= MEGABYTE)
		assert(sprintf(readable_size, "%.2lf MB", (double)(size / MEGABYTE)));
	else if (size >= KILOBYTE)
		assert(sprintf(readable_size, "%.2lf KB", (double)(size / KILOBYTE)));
	else
		assert(sprintf(readable_size, "%lu B", size));

	return readable_size;
}

void remove_trailing_spaces(const char* buffer)
{
	// get pointer to first space, set to null character
	assert(buffer != NULL);
	char* end = strchr(buffer, ' ');
	if (end)
		*end = '\0';
}

char get_path_separator(const char* path)
{
	// Tallman's simple path separator finding function
	// Find the first path separator, and return it to caller
	// If no sep, assume '/'
	if (strchr(path, '/') != NULL)
	{
		return '/';
	}

	if (strchr(path, '\\') != NULL)
	{
		return '\\';
	}
	return '/';
}
