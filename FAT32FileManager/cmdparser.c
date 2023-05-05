#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "cmdparser.h"
#include "ConsoleUtil.h"

void list_commands()
{
	printf("list part\n");
	printf("  - list all partitions found in the image (* are readable partitions)\n");
	printf("sel part <part num>\n");
	printf("  - part to mount to root directory\n");
	printf("ls\n");
	printf("  - list all files in the current directory\n");
	printf("cd <dir>\n");
	printf("  - change directory\n");
	printf("cat <file>\n");
	printf("  - print the contents of a file in the current directory\n");
	printf("pwd\n");
	printf("  - print the current directory\n");
	printf("export <file>\n");
	printf("  - export a file in the current directory to the local disk\n");
	printf("help\n");
	printf("  - display this menu\n");
	printf("exit\n");
	printf("  - end the application\n");

}

void run_command_handler(FileManagerContext* context)
{
	const Command commands[] =
	{
		{"list part", list_part},
		{"sel part ", select_part},
		{"ls", list_directory},
		{"cd ", nested_change_directory},
		{"cat ", cat_file},
		{"pwd", display_pwd},
		{"export ", export_to_file},
		{"help", list_commands},
		{"exit", exit_file_manager }
	};
	const size_t num_commands = sizeof(commands) / sizeof(Command);

	while (true)
	{
		bool valid_command = false;
		printf("file-manager: %s $ ", context->pwd);
		char* input = get_line_dynamic();

		// see if the input matches any commands
		for (int i = 0; i < num_commands; i++)
		{
			// if command starts out input
			if (strstr(input, commands[i].name) == input)
			{
				valid_command = true;
				char* arg = input + strlen(commands[i].name);	// strip command and just pass arg
				commands[i].function(context, arg);	// call callback
				break;
			}
		}

		if (!valid_command)
		{
			printf("Invalid Command: %s\nType 'help' for a list of commands.\n\n", input);
		}
	}
}