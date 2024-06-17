
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
*/

/*
 * Con la nueva implementacion dejarian de ser utiles los datablocks
*/

// struct datablock {
// 	char data[BLOCK_SIZE];
// };

struct inode {
	int inum;                           // Low level file name.
	int type;                           // Type of file (File | Directory).
	char name[NAME_SIZE];  // Name of the file (Including path).
	mode_t mode;   // Wether this file can be read/ written/ executed.
	uid_t owner;   // Who owns this file (User id).
	gid_t group;   // Group id.
	off_t size;    // Size of the file.
	time_t ctime;  // Time the file was created.
	time_t mtime;  // Time the file was last modified.
	time_t atime;  // Time the file was last acceded.
	nlink_t links_count;  // How many hard links are there on this file.

	/*
	 * En la nueva implementacion se hara que c/inodo tenga su propia data.
	 * De funcionar luego se creara una variable que defina mejor a BLOCK_SIZE*2
	*/
	char data[BLOCK_SIZE*2];				//ahora el inodo no apunta a data, la tiene dentro.

	/*
	 * atributos que quedan sin utilizar con la nueva implementacion
	 -  int blck_bitmap[BLOCKS_PER_INODE];  // Position of blocks in datablock bitmap.
	 -	blkcnt_t blocks;  // How many blocks have been allocated to this file.
	 -	struct datablock *blockptr[DISK_PTRS];  // Set of disk pointers (15 total).
	*/
};


struct superblock {
	int inodes;    // Amount of inodes in the filesystem.
	int *ibitmap;  // Pointer to inode bitmap start.
	

	/*
	 * Con la nueva implementacion no se utilizarian mas los siguientes atributos:
	 - int dblocks;   // Amount of datablocks in the filesystem.
	 - int *dbitmap;  // Pointer to data bitmap start.
	*/
};
