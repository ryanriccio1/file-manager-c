#include <stdio.h>
#include <stdlib.h>

#include "fatcontextfactory.h"
#include "cmdparser.h"


int main(int argc, char* argv[])
{
	// parse cmdline input
	if (argc > 2) 
	{
		printf("Too many arguments.\n");
		return EXIT_FAILURE;
	}
	if (argc < 2) 
	{
		printf("Usage: %s <image file>.\n", argv[0]);
		return EXIT_FAILURE;
	}

	// create file manager data context
	FileManagerContext* context = setup_file_manager_context(argv[1]);

	// run main loop given data context
	run_command_handler(context);
}
