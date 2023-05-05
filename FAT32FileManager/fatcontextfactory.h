#pragma once
#include "fatparser.h"

typedef struct FileManagerContext
{
	FILE *file;
	MBR *mbr;
	Partition *part;
	PartitionInfo *part_info;
	PartitionLocations *part_offsets;
	FileRecord *current_dir;
	uint32_t selected_part;
	size_t dir_entries;
	char *pwd;
	char *pwd_chain[64];
	size_t pwd_level;
} FileManagerContext;

/**
 * @brief Set the up file manager context object
 *
 * @param filename Filename to load disk image from
 * @return FileManagerContext* Data context
 */
FileManagerContext *setup_file_manager_context(const char *filename);
/**
 * @brief Destroy file manager
 *
 * @param context Context to destroy
 * @param arg Possible error message
 */
void exit_file_manager(FileManagerContext *context, char *arg);

/**
 * @brief List partition handler
 */
void list_part(const FileManagerContext *context);
/**
 * @brief Select partition handler
 */
void select_part(FileManagerContext *context, char *arg);
/**
 * @brief LS handler
 */
void list_directory(const FileManagerContext *context);
/**
 * @brief CD handler
 */
void change_directory(FileManagerContext *context, char *arg);
/**
 * @brief Tokenized CD handler
 */
void nested_change_directory(FileManagerContext *context, char *arg);
/**
 * @brief CAT handler
 */
void cat_file(const FileManagerContext *context, const char *arg);
/**
 * @brief PWD handler
 */
void display_pwd(const FileManagerContext *context);
/**
 * @brief Exportsss handler
 */
void export_to_file(const FileManagerContext *context, const char *arg);
