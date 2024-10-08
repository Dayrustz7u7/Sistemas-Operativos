
#include <sys/types.h>
#include <time.h>

#define FUSE_USE_VERSION 30
#define BLOCK_SIZE 8192
#define DISK_PTRS 15
#define NAME_SIZE 225
#define TOTAL_INODES 80
#define TOTAL_DATABLOCKS 56
#define BLOCKS_PER_INODE 15
#define OCCUPIED 1
#define FREE 0
#define TYPE_DIRECTORY 0
#define TYPE_FILE 1
#define SERIALIZATION_FILE "fs.fisopfs"
#define ROOT "/"


/*
 *------------------------------------*
 *			 ESTRUCTURAS			  *
 *------------------------------------*
 */

struct inode {
	int inum;              // Low level file name.
	int type;              // Type of file (File | Directory).
	char name[NAME_SIZE];  // Name of the file (Including path).
	mode_t mode;   // Wether this file can be read/ written/ executed.
	uid_t owner;   // Who owns this file (User id).
	gid_t group;   // Group id.
	off_t size;    // Size of the file.
	time_t ctime;  // Time the file was created.
	time_t mtime;  // Time the file was last modified.
	time_t atime;  // Time the file was last acceded.
	nlink_t links_count;    // How many hard links are there on this file.
	char data[BLOCK_SIZE];  // Inode data.
};


struct superblock {
	int inodes;    // Amount of inodes in the filesystem.
	int *ibitmap;  // Pointer to inode bitmap start.
};
