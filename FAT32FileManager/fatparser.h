#pragma once
#ifdef __GNUC__
#define PACK(__Declaration__) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK(__Declaration__) __pragma(pack(push, 1)) __Declaration__ __pragma(pack(pop))
#endif

#include <stdint.h>
#include <stdbool.h>

#define SECTOR_SIZE 512

typedef enum PartitionType
{
	EMPTY = 0,
	FAT12 = 0x01,
	FAT16_B = 0x06,
	FAT32_CHS = 0x0b,
	FAT32_LBA = 0x0c,
	FAT16_LBA = 0x0e
} PartitionType;

PACK(
	typedef struct Partition {
		uint8_t bootable;
		uint8_t first_chs[3];
		uint8_t type;
		uint8_t last_chs[3];
		uint32_t lba_offset;
		uint32_t sector_count;
	} Partition;)

PACK(
	typedef struct MBR {
		uint8_t bootcode[446];
		Partition partitions[4];
		uint16_t magic;
	} MBR;)

PACK(
	typedef struct PartitionInfo {
		uint16_t bytes_per_sector;
		uint8_t sectors_per_cluster;
		uint16_t reserved_sector_count;
		uint8_t num_fat_tables;
		uint16_t root_dir_entries;
		uint8_t unused1[3];
		uint16_t fat16_table_size;
		uint32_t unused2[3];
		uint32_t fat32_table_size;
		uint32_t unused3;
		uint32_t root_dir_first_cluster;
	} PartitionInfo;)

typedef struct PartitionLocations
{
	size_t FAT[2];
	size_t root_dir;
	size_t data_dir;
} PartitionLocations;

PACK(
	typedef struct FileRecord {
		unsigned char filename[8];
		unsigned char extension[3];
		unsigned int readonly : 1;
		unsigned int hidden : 1;
		unsigned int system : 1;
		unsigned int volume_id : 1;
		unsigned int directory : 1;
		unsigned int archive : 1;
		unsigned int unused1 : 2;
		uint8_t unused2[5];
		uint16_t first_cluster_hi;
		uint16_t time;
		uint16_t date;
		uint16_t first_cluster_lo;
		uint32_t file_size;
	} FileRecord);

/**
 * @brief Get the file attributes in a readable string
 *
 * @param record Directory to read
 * @return const char* Readable attributes
 */
const char *get_file_attributes(const FileRecord *record);
/**
 * @brief Get the short filename (FAT 8.3 style)
 *
 * @param record Directory entry
 * @return const char* Readable filename
 */
const char *get_short_filename(const FileRecord *record);
/**
 * @brief Make sure partition meets our critieria
 */
bool check_valid_part(const Partition *part);
/**
 * @brief Check if index is valid partition
 */
bool check_valid_part_index(const MBR *mbr, const size_t index);
/**
 * @brief Read the partition boot sector
 */
PartitionInfo *get_part_info(FILE *fp, const Partition *part);
/**
 * @brief Calculate global offsets for partition
 */
PartitionLocations *get_part_offsets(const Partition *part, PartitionInfo *part_info);
/**
 * @brief Convert cluster to offset
 */
uint32_t get_cluster_offset(const PartitionInfo *part_info, const PartitionLocations *part_offsets, uint32_t cluster_number);
/**
 * @brief Parse cluster number based on filesystem type
 */
uint32_t get_cluster_number(const FileRecord *record, const PartitionType part_type);
/**
 * @brief Get a parsed array of all directory entries in the current dir at an offset
 */
FileRecord *get_dir(FILE *fp, const size_t offset, size_t *num_entries);
/**
 * @brief Get readable date and time from file record
 */
char *get_date_time(const FileRecord *record);
/**
 * @brief Read file using FAT lookups at given cluster
 */
uint8_t *read_file(const FILE *fp, const uint32_t start_cluster_number, const PartitionInfo *part_info,
				   const PartitionLocations *part_offsets, const PartitionType part_type, const size_t file_size);
/**
 * @brief String parser for directory
 */
size_t name_to_idx(const FileRecord *current_directory, size_t directory_entries, const char *name);
/**
 * @brief String parser for directory
 */
FileRecord *name_to_record(FileRecord *current_directory, size_t directory_entries, const char *name);
/**
 * @brief list part handler
 */
void display_partition_info(const MBR *mbr);
/**
 * @brief Display a directory
 */
void display_records(const FileRecord *records, const size_t *num_records);