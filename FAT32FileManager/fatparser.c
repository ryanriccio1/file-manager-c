#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fatparser.h"
#include "utilties.h"


const char* get_file_attributes(const FileRecord* record)
{
	// check if file is (D)ir, (A)rch, (V)ol ID, (S)ystem, (H)idden, (R)ead-only, or LFN
	// return human readable string
	static char description[7] = { 0 };
	strcpy(description, "------");

	if (record->directory) description[1] = 'D';
	if (record->archive) description[0] = 'A';

	// Special case for the Long File Name attribute
	if (record->readonly &&
		record->hidden &&
		record->system &&
		record->volume_id)
	{
		description[2] = 'L';
		description[3] = 'O';
		description[4] = 'N';
		description[5] = 'G';
	}
	else
	{
		if (record->volume_id) description[2] = 'V';
		if (record->system) description[3] = 'S';
		if (record->hidden) description[4] = 'H';
		if (record->readonly) description[5] = 'R';
	}

	return description;
}

const char* get_short_filename(const FileRecord* record)
{
	// make sure there are no trailing spaces, and string is null terminated
	// then return short filename
	static char full_filename[13];
	memset(full_filename, 0, sizeof(full_filename));

	remove_trailing_spaces(record->filename);
	strncat(full_filename, record->filename, 8);

	remove_trailing_spaces(record->extension);

	if (record->extension[0] != '\0')
	{
		strcat(full_filename, ".");
		strncat(full_filename, (const char*)record->extension, 3);
	}
	return full_filename;
}

bool check_valid_part(const Partition* part)
{
	// make sure it is a type we know about
	if (part->type == FAT32_LBA ||
		part->type == FAT16_LBA ||
		part->type == FAT16_B ||
		part->type == FAT32_CHS ||
		part->type == FAT12)
		return true;
	return false;
}

bool check_valid_part_index(const MBR* mbr, const size_t index)
{
	// make sure the index selected is something we can parse
	if (check_valid_part(&mbr->partitions[index]) &&
		mbr->partitions[index].type != FAT32_CHS &&
		mbr->partitions[index].type != FAT12)
		return true;
	return false;
}

PartitionInfo* get_part_info(FILE* fp, const Partition* part)
{
	// read partition boot record
	PartitionInfo* part_info = calloc(1, sizeof(PartitionInfo));
	assert(!fseek(fp, part->lba_offset * SECTOR_SIZE + 0x0b, SEEK_SET));
	assert(fread(part_info, sizeof(uint8_t), sizeof(PartitionInfo), fp));
	return part_info;
}

PartitionLocations* get_part_offsets(const Partition* part, PartitionInfo* part_info)
{
	// based on FAT type, calculate the offsets needed 
	PartitionLocations* part_offsets = calloc(1, sizeof(PartitionLocations));
	part_offsets->FAT[0] = (size_t)part_info->reserved_sector_count * part_info->bytes_per_sector;

	part_offsets->data_dir = 0;
	part_offsets->root_dir = 0;

	if (part->type == FAT16_LBA || part->type == FAT16_B)
	{
		part_offsets->FAT[1] = part_offsets->FAT[0] + (size_t)part_info->fat16_table_size * part_info->bytes_per_sector;
		part_offsets->root_dir = part_offsets->FAT[1] + (size_t)part_info->fat16_table_size * part_info->bytes_per_sector;

		part_offsets->data_dir = part_info->root_dir_entries * 32;
	}
	else if (part->type == FAT32_LBA)
	{
		part_offsets->FAT[1] = part_offsets->FAT[0] + (size_t)part_info->fat32_table_size * part_info->bytes_per_sector;
		part_offsets->root_dir = part_offsets->FAT[1] + (size_t)part_info->fat32_table_size * part_info->bytes_per_sector;
	}
	part_offsets->FAT[0] += (size_t)part->lba_offset * part_info->bytes_per_sector;
	part_offsets->FAT[1] += (size_t)part->lba_offset * part_info->bytes_per_sector;
	part_offsets->root_dir += (size_t)part->lba_offset * part_info->bytes_per_sector;

	part_offsets->data_dir += part_offsets->root_dir;


	return part_offsets;
}

uint32_t get_cluster_offset(const PartitionInfo* part_info, const PartitionLocations* part_offsets, uint32_t cluster_number)
{
	// used to convert cluster to fseek offset using FAT algorithm
	// 0 is root dir
	if (cluster_number == 0)
		return part_offsets->root_dir;
	return part_offsets->data_dir + ((cluster_number - 2) * part_info->sectors_per_cluster * part_info->bytes_per_sector);
}

uint32_t get_cluster_number(const FileRecord* record, const PartitionType part_type)
{
	// read record and return cluster number using proper bytes based on type
	return part_type == FAT32_LBA ? (record->first_cluster_hi << 16) | (record->first_cluster_lo) : record->first_cluster_lo;
}

FileRecord* get_dir(FILE* fp, const size_t offset, size_t* num_entries)
{
	assert(!fseek(fp, offset, SEEK_SET));
	*num_entries = 0;
	// start with 4 records
	FileRecord* records = calloc(4, sizeof(FileRecord));

	FileRecord tmp_record;

	while (true)
	{
		assert(fread(&tmp_record, sizeof(FileRecord), 1, fp));

		// if 0, we have reached end of dir
		if (tmp_record.filename[0] != 0x00)
		{
			if (tmp_record.filename[0] != 0xE5)	// this means file was deleted
			{
				// make sure everything is null terminated
				char* fn_end = strchr(tmp_record.filename, ' ');
				if (fn_end)
					*fn_end = '\0';

				char* ext_end = strchr(tmp_record.extension, ' ');
				if (ext_end)
					*ext_end = '\0';

				// if we loop around to a new directory in a cluster directly after the one we are reading
				// break
				if (strcmp(tmp_record.filename, ".") == 0 && tmp_record.directory && *num_entries > 0) break;

				// copy temp to real array
				memcpy(&records[*num_entries], &tmp_record, sizeof(FileRecord));
				*num_entries += 1;
			}
		}
		else
			break;
		// realloc if we have more than 4 records
		if (*num_entries % 4 == 0)
		{
			records = (FileRecord*)realloc(records, sizeof(FileRecord) * (*num_entries + 4));
		}
	}
	return records;
}

uint8_t* read_file(const FILE* fp, const uint32_t start_cluster_number, const PartitionInfo* part_info,
	const PartitionLocations* part_offsets, const PartitionType part_type, const size_t file_size)
{
	const size_t fat_entry_size = part_type == FAT32_LBA ? 4 : 2;
	const size_t end_of_chain = part_type == FAT32_LBA ? 0xFFFFFFF0 : 0xFFF0;

	// num cluster chain steps needed to follow
	const size_t cluster_max_len = file_size / (part_info->bytes_per_sector * part_info->sectors_per_cluster);

	// enough to store first entry and chain
	uint32_t* cluster_chain = malloc((cluster_max_len + 1) * sizeof(uint32_t));
	cluster_chain[0] = start_cluster_number;	// set first element to given cluster

	// follow cluster chain only if we have more than 1 cluster
	if (cluster_max_len > 0)
	{
		uint32_t next_cluster = 0;
		size_t idx = 1;

		// see to first cluster
		assert(!fseek(fp, part_offsets->FAT[0] + start_cluster_number * fat_entry_size, SEEK_SET));

		while (true)
		{
			assert(fread(&next_cluster, fat_entry_size, 1, fp));
			assert(!fseek(fp, part_offsets->FAT[0] + next_cluster * fat_entry_size, SEEK_SET));
			if (next_cluster >= end_of_chain) break; // make sure we haven't reached EOF
			cluster_chain[idx] = next_cluster;
			idx++;
		}

	}

	// allocate enough space for the data (read entire file)
	uint8_t* data = calloc(1, file_size + 1);
	const size_t cluster_size = part_info->bytes_per_sector * part_info->sectors_per_cluster;

	size_t current_byte_offset = 0;	// offset into data
	for (size_t idx = 0; idx < cluster_max_len + 1; idx++)	// idx = idx of cluster chain
	{
		assert(!fseek(fp, get_cluster_offset(part_info, part_offsets, cluster_chain[idx]), SEEK_SET));

		if (idx == cluster_max_len)
		{
			assert(fread(&data[current_byte_offset], file_size % cluster_size, 1, fp));
			break;
		}

		assert(fread(&data[current_byte_offset], cluster_size, 1, fp));
		current_byte_offset += cluster_size;
	}
	data[file_size] = 0;

	free(cluster_chain);

	return data;

}

void display_partition_info(const MBR* mbr)
{
	// print info about partitions on MBR
	printf("%10s%10s%12s%12s%12s%13s\n", "Part", "Boot", "Start", "End", "Size", "Type");
	for (size_t idx = 0; idx < 4; idx++)
	{
		const Partition part = mbr->partitions[idx];

		if (check_valid_part(&part))
		{
			const char* size = get_human_readable_size(part.sector_count * SECTOR_SIZE);
			const char* start = get_human_readable_size(part.lba_offset * SECTOR_SIZE);
			const char* end = get_human_readable_size(part.lba_offset * SECTOR_SIZE + part.sector_count * SECTOR_SIZE - 1);
			char type[16];
			switch (part.type)
			{
			case FAT32_LBA:
				strcpy(type, "*FAT32_LBA");
				break;
			case FAT16_LBA:
				strcpy(type, "*FAT16_LBA");
				break;
			case FAT16_B:
				strcpy(type, "*FAT16_B");
				break;
			case FAT32_CHS:
				strcpy(type, "FAT32_CHS");
				break;
			case FAT12:
				strcpy(type, "FAT12");
				break;
			default:
				strcpy(type, "INVALID");
			}
			printf("%10llu%10s%12s%12s%12s%13s\n", idx, part.bootable ? "Y" : "N", start, end, size, type);
			free(start);
			free(end);
			free(size);
		}
	}
}

char* get_date_time(const FileRecord* record)
{
	// Given a file record, calculate the human readable date and time
	char* date_time_string = malloc(sizeof(char) * 20);
	const int hours = record->time >> 11;
	const int min = (record->time & 0x7E0) >> 5;
	const int sec = (record->time & 0x1F) * 2;

	const int year = 1980 + (record->date >> 9);
	const int month = (record->date & 0x1E0) >> 5;
	const int day = record->date & 0x1F;

	sprintf(date_time_string, "%02d:%02d:%02d %02d/%02d/%02d", hours, min, sec, month, day, year);

	return date_time_string;
}

void display_records(const FileRecord* records, const size_t* num_records)
{
	// Print a directory
	printf("%-8s%-9s%-15s%15s%22s\n", "Type", "Attrib", "Name", "Size", "Date Modified");
	for (size_t idx = 0; idx < *num_records; idx++)
	{
		if (!records[idx].volume_id)
		{
			if (records[idx].directory)
				printf("%-8s", "<DIR>");
			else
				printf("%-8s", "");

			printf("%-9s", get_file_attributes(&records[idx]));
			printf("%-15s", get_short_filename(&records[idx]));
			if (!records[idx].directory)
				printf("%15s", get_human_readable_size(records[idx].file_size));
			else
				printf("%15s", "");
			char* date_time_string = get_date_time(&records[idx]);
			printf("%22s\n", date_time_string);
			free(date_time_string);
		}
	}
}

size_t name_to_idx(const FileRecord* current_directory, size_t directory_entries, const char* name)
{
	for (size_t idx = 0; idx < directory_entries; idx++)
	{
		if (strcmp(name, get_short_filename(&current_directory[idx])) == 0)
		{
			return idx;
		}
	}
	return SIZE_MAX;
}

FileRecord* name_to_record(FileRecord* current_directory, size_t directory_entries, const char* name)
{
	for (size_t idx = 0; idx < directory_entries; idx++)
	{
		if (strcmp(name, get_short_filename(&current_directory[idx])) == 0)
		{
			return &current_directory[idx];
		}
	}
	return NULL;
}

