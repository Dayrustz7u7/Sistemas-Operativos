
#include <sys/types.h>
#include <time.h>

#define FUSE_USE_VERSION 30
#define BLOCK_SIZE 4096
#define DISK_PTRS 15
#define NAME_SIZE 225
#define TOTAL_INODES 80
#define TOTAL_DATABLOCKS 56
#define BLOCKS_PER_INODE 15
#define OCCUPIED 1
#define FREE 0
#define DIRECTORY 1
#define TYPE_DIRECTORY 0
#define TYPE_FILE 1


/*
 *------------------------------------*
 *			 ESTRUCTURAS			  *
 *------------------------------------*
 
 Se utilizaran 64 Bloques de 4 KB c/u.
 C/Inodo ocupara 256 bytes.

 El filesystem estara organizado de la siguiente manera:
	- Superbloque 		-> 1 Bloque.
	- Bitmap de inodos 	-> 1 Bloque.
	- Bitmap de datos 	-> 1 Bloque.
	- Inodos			-> 5 Bloques.
	- Datablocks		-> 56 Bloques.
*/

struct datablock {
	char data[BLOCK_SIZE];
};

struct inode {
	int inum;								// Low level file name.
	int type;								// Type of file (File | Directory).
	int blck_bitmap[BLOCKS_PER_INODE];		// Position of blocks in datablock bitmap.
	char name[NAME_SIZE];					// Name of the file (Including path).
	mode_t mode;							// Wether this file can be read/ written/ executed.
	uid_t owner;							// Who owns this file (User id).
	gid_t group;							// Group id.
	off_t size;								// Size of the file.
	time_t ctime;							// Time the file was created.
	time_t mtime;							// Time the file was last modified.
	time_t atime;							// Time the file was last acceded.
	time_t dtime;							// Time the inode was deleted.
	nlink_t links_count;					// How many hard links are there on this file.
	blkcnt_t blocks;						// How many blocks have been allocated to this file.
	struct datablock *blockptr[DISK_PTRS];	// Set of disk pointers (15 total).
};

struct superblock{
	int dblocks;							// Amount of datablocks in the filesystem.
	int inodes;								// Amount of inodes in the filesystem.
	int *ibitmap;							// Pointer to inode bitmap start.
	int *dbitmap;							// Pointer to data bitmap start.
};
