#pragma once
#include "fatcontextfactory.h"

typedef struct Command
{
	const char *name;
	void (*function)(FileManagerContext *, char *);
} Command;

/**
 * @brief Run the main command loop
 *
 * @param context File Manager data context
 */
void run_command_handler(FileManagerContext *context);

/**
 * @brief Help Function
 *
 */
void list_commands();
