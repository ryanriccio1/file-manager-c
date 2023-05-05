#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "fatcontextfactory.h"
#include "fatparser.h"
#include "ConsoleUtil.h"
#include "utilties.h"

FileManagerContext* setup_file_manager_context(const char* filename)
{
	FileManagerContext* context = calloc(1, sizeof(FileManagerContext));

	context->mbr = calloc(1, sizeof(MBR));
	for (size_t idx = 0; idx < 64; idx++)
	{
		context->pwd_chain[idx] = calloc(16, sizeof(char));
	}
	context->pwd = calloc(128, sizeof(char));
	context->pwd_level = 0;

	context->file = fopen(filename, "rb");
	if (!context->file)
		exit_file_manager(context, "Could not open file.\n");

	// read in mbr
	if (!fread(context->mbr, sizeof(uint8_t), sizeof(MBR), context->file))
		exit_file_manager(context, "Could not read MBR.\n");

	return context;
}

void exit_file_manager(FileManagerContext* context, char* arg)
{
	printf("%s\n", arg);

	if (context->current_dir)
		free(context->current_dir);

	if (context->part_offsets)
		free(context->part_offsets);

	if (context->part_info)
		free(context->part_info);


	free(context->mbr);
	for (size_t idx = 0; idx < 64; idx++)
	{
		free(context->pwd_chain[idx]);
	}
	free(context->pwd);
	free(context);
	exit(EXIT_SUCCESS);
}

void calculate_pwd(const FileManagerContext* context)
{
	strcpy(context->pwd, "/");
	for (size_t idx = 0; idx < context->pwd_level; idx++)
	{
		strcat(context->pwd, context->pwd_chain[idx]);
		strcat(context->pwd, "/");
	}
}

void append_pwd(FileManagerContext* context, const char* dir)
{
	strcpy(context->pwd_chain[context->pwd_level], dir);
	context->pwd_level += 1;
	calculate_pwd(context);
}

void pop_pwd(FileManagerContext* context)
{
	context->pwd_level -= 1;
	calculate_pwd(context);
}

void list_part(const FileManagerContext* context)
{
	display_partition_info(context->mbr);
}

void select_part(FileManagerContext* context, char* arg)
{
	int32_t input;
	if (string_to_int(arg, &input))
		printf("Not a valid partition number.\n\n");

	else if (!check_valid_part_index(context->mbr, input))
		printf("Partition number out of range.\n\n");

	else
	{
		context->selected_part = (uint32_t)input;
		context->part = &context->mbr->partitions[context->selected_part];
		context->part_info = get_part_info(context->file, context->part);
		context->part_offsets = get_part_offsets(context->part, context->part_info);
		context->current_dir = get_dir(context->file, context->part_offsets->root_dir, &context->dir_entries);
		while (context->pwd_level != 0)
			pop_pwd(context);
		calculate_pwd(context);
	}

}

void list_directory(const FileManagerContext* context)
{
	if (!context->current_dir)
	{
		printf("No directory selected.\n");
		return;
	}
	display_records(context->current_dir, &context->dir_entries);
}

void change_directory(FileManagerContext* context, char* arg)
{
	if (!context->current_dir)
	{
		printf("No directory selected.\n");
		return;
	}

	const FileRecord* dir = name_to_record(context->current_dir, context->dir_entries, arg);
	if (dir)
	{
		if (dir->directory)
		{	// get dir at cluster
			const uint32_t dir_cluster_number = get_cluster_number(dir, context->part->type);
			const uint32_t offset = get_cluster_offset(context->part_info, context->part_offsets, dir_cluster_number);
			context->current_dir = get_dir(context->file, offset, &context->dir_entries);
			if (strchr(arg, '.') == NULL)
			{
				append_pwd(context, dir->filename);
			}
			else
			{	// subtract from pwd on cd ..
				pop_pwd(context);
			}

		}
		else
			printf("File is not a directory.\n\n");
	}
	else
		printf("Invalid directory.\n\n");
}

void nested_change_directory(FileManagerContext* context, char* arg)
{
	/* get the first token */
	const char separator = get_path_separator(arg);
	const char* token = strtok(arg, &separator);

	while (token != NULL)
	{
		change_directory(context, token);
		token = strtok(NULL, &separator);
	}
}

void cat_file(const FileManagerContext* context, const char* arg)
{
	if (!context->current_dir)
	{
		printf("No directory selected.\n\n");
		return;
	}

	const FileRecord* selected_file = name_to_record(context->current_dir, context->dir_entries, arg);
	if (!selected_file)
	{
		printf("Cannot find file!\n\n");
		return;
	}

	const uint32_t cluster_num = get_cluster_number(selected_file, context->part->type);


	const uint8_t* data = read_file(context->file, cluster_num, context->part_info,
		context->part_offsets, context->part->type, selected_file->file_size);
	printf("%s\n\n", (const char*)data);
}

void display_pwd(const FileManagerContext* context)
{
	if (!context->current_dir)
	{
		printf("No directory selected.\n\n");
		return;
	}
	printf("%s\n\n", context->pwd);

}

void export_to_file(const FileManagerContext* context, const char* arg)
{
	if (!context->current_dir)
	{
		printf("No directory selected.\n\n");
		return;
	}

	const FileRecord* selected_file = name_to_record(context->current_dir, context->dir_entries, arg);
	if (!selected_file)
	{
		printf("Cannot find file!\n\n");
		return;
	}
	const uint32_t cluster_num = get_cluster_number(selected_file, context->part->type);


	const uint8_t* data = read_file(context->file, cluster_num, context->part_info,
		context->part_offsets, context->part->type, selected_file->file_size);

	// export txt file
	FILE* export_file = fopen(get_short_filename(selected_file), "wb");
	assert(export_file);
	assert(fwrite(data, selected_file->file_size, 1, export_file));
	assert(!fclose(export_file));
}
