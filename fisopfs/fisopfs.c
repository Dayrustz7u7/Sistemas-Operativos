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

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>



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
	// AGREGAR DIRECCIONES DE LOS BLOQUES PARA LIBERARLOS DEL DBITMAP.
};

struct superblock{
	int dblocks;							// Amount of datablocks in the filesystem.
	int inodes;								// Amount of inodes in the filesystem.
	int *ibitmap;							// Pointer to inode bitmap start.
	int *dbitmap;							// Pointer to data bitmap start.
};

/*
 * ----- Inicializacion -----
*/

struct superblock superblock;
int ibitmap[TOTAL_INODES];
int dbitmap[TOTAL_DATABLOCKS];
struct inode inodes[TOTAL_INODES];
struct datablock data_blocks[TOTAL_DATABLOCKS];

/*
 *-------------------------------------------------*
 *			Operaciones Auxiliares				   *
 *-------------------------------------------------*
*/

// Dado un path buscamos en el bitmap de inodos aquellos
// que esten ocupados y su nombre sea el mismo que el path.
// De no haber ningun inodo que coincida devolvemos -1.
int get_inode(const char *path){
	for (int i = 0; i < TOTAL_INODES; i++){
		if (ibitmap[i] == OCCUPIED && strcmp(inodes[i].name, path)==0){
			return i;
		}
	}
	return -1;
}


// Obtiene la posicion del primer bloque libre.
// Devuelve -1 de no haber ningun bloque libre.
int get_free_block(){
	for (int i = 0; i < TOTAL_DATABLOCKS; i++){
		if (dbitmap[i] == FREE){
			return i;
		}
	}
	return -1;
}


// Obtiene la posicion del primer inodo libre.
// Devuelve -1 de no haber ningun inodo libre. 
int get_free_inode(){
	for (int i = 0; i < TOTAL_INODES; i++){
		if (ibitmap[i] == FREE){
			return i;
		}
	}
	return -1;
}


// Obtiene la proxima posicion libre del arreglo de punteros del
// inodo. 
// De estar lleno, devuelve -1. ACA DEBERIAMOS HACER QUE DE ESTAR LLENO DEVUELVA EN REALIDAD UN PUNTERO INDIRECTO?
int get_free_blockptr(int idx_inode){
	for (int i = 0; i < DISK_PTRS; i++){
		if (inodes[idx_inode].blockptr[i] == NULL){
			return i;
		}
	}
	return -1;
}


// Inicializa un inodo. 
// En caso de exito devuelve la posicion del mismo, caso contrario -1.
int initialize_inode(const char *path, int type, mode_t mode){
	int position = get_free_inode();
	int first_block = get_free_block();
	if ( (position == -1) || (first_block == -1) || (strlen(path)> NAME_SIZE) ){
		return -1;
	}
	ibitmap[position] = OCCUPIED;
	dbitmap[first_block] = OCCUPIED;

	struct inode current_inode = inodes[position];
	time_t right_now = time(NULL);
	nlink_t tot_links = 0;
	if (mode == DIRECTORY){
		tot_links = 2;
	} else {
		tot_links = 1;
	}

	current_inode.inum = position;
	current_inode.type = type;
	current_inode.blck_bitmap[0] = first_block;
	strcpy(current_inode.name, path);
	current_inode.mode = mode;
	current_inode.owner = getuid();
	current_inode.group = getgid();
	current_inode.size = 0;
	current_inode.ctime = right_now;
	current_inode.mtime = right_now;
	current_inode.atime = right_now;
	current_inode.links_count = tot_links;
	current_inode.blocks = 1;
	current_inode.blockptr[0] = &data_blocks[first_block];
	return position;
}


// Obtiene todas las dentries de un path dado.
int get_total_dentries(const char *path){
	int tot_dentries = 0;
	for (int i = 0; i < strlen(path); i++){
		if (path[i] == '/'){
			tot_dentries++;
		}
	}
	return tot_dentries;
}


// Obtiene la posicion dentro del path, a partir de la cual empieza el 
// nombre del archivo o directorio. 
// Devuelve -1 si es que no tiene.
int get_name_offset(const char *path){
	int position = -1;
	for (int i = 0; i < strlen(path); i++){
		if (path[i] == '/'){
			position = i+1; 	// Posicion de la primera letra del nombre.
		}
	}
	return position;
}


// A partir de la posicion del inodo en el bitmap, borra y limpia el inodo.
void delete_inode(int i_pos){
	ibitmap[i_pos] = FREE;
	for (int i = 0; i < BLOCKS_PER_INODE; i++){
		if (inodes[i_pos].blockptr[i] == NULL){
			continue;
		}
		memset(inodes[i_pos].blockptr[i], NULL, sizeof(struct datablock));
		dbitmap[inodes[i_pos].blck_bitmap[i]] = FREE;
	}
	memset(&inodes[i_pos], NULL, sizeof(struct inode));
}


// Indica mediante el path (de un directorio o archivo) si el mismo pertenece 
// al directorio cuyo path es "dir_path".
// Retorna -1 en caso de que no este, 1 si esta.
int in_dir(const char *path, const char *dir_path){
	if (!(strlen(path)>strlen(dir_path) && strlen(dir_path)>0)){
		return -1;
	}

	// Directory path should have one less dentry.
	// Unless its in root.
	int dir_dentries = get_total_dentries(dir_path);
	int path_dentries = get_total_dentries(path);
	if ((dir_dentries+1) != path_dentries){ 				// Only case condition isn't valid, yet path belongs to dir_path is root.
		if ((dir_dentries != 1) && (path_dentries != 1)){ 	// If both have only one "/", then its root.
			return -1;
		}
	}

	if (strncmp(dir_path, path, strlen(dir_path))==0){
		return 1;
	}

	return -1;
}



// Le agrega un bloque a un inodo, agregando un puntero al mismo
// en la primer posicion libre dentro del inodo.
// Devuelve 1 en caso de poder llevarse a cabo, -1 en caso contrario.
int add_block(int idx_inode){
	int pos_free_block = get_free_block();
	int pos_free_blckptr = get_free_blockptr(idx_inode);
	if (pos_free_block == -1 || pos_free_blckptr == -1){
		return -1;
	}
	dbitmap[pos_free_block] = OCCUPIED;
	inodes[idx_inode].blockptr[pos_free_blckptr] = &data_blocks[pos_free_block];
	return 1;
}


// Creamos un directorio.
static int fisopfs_mkdir(const char *path, mode_t mode){
	return initialize_inode(path, TYPE_DIRECTORY, mode);
}


// Hacemos unlink.
static int
fisopfs_unlink(const char *path){
	int position = get_inode(path);
	if (position==-1){
		return -1;
	}
	if (inodes[position].type==TYPE_DIRECTORY){
		return -1;
	}
	inodes[position].links_count--;

	if (inodes[position].links_count==0){
		delete_inode(position);
	}

	return 0;
}


static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	if (strcmp(path, "/") == 0) {
		st->st_uid = 1717;
		st->st_mode = __S_IFDIR | 0755;
		st->st_nlink = 2;
	} else if (strcmp(path, "/fisop") == 0) {
		st->st_uid = 1818;
		st->st_mode = __S_IFREG | 0644;
		st->st_size = 2048;
		st->st_nlink = 1;
	} else {
		return -ENOENT;
	}

	return 0;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	int position = get_inode(path);
	if (position == -1){
		return -1;
	}
	struct inode directory = inodes[position];
	if (directory.type != TYPE_DIRECTORY){
		return -1;
	}

	directory.atime = time (NULL);

	for (int i = 0; i < TOTAL_INODES; i++){
		if (ibitmap[i]==FREE){
			continue;
		}
		if (in_dir(inodes[i].name, directory.name)==-1){
			continue;
		}
		int name_position = get_name_offset(inodes[i].name);
		filler(buffer, inodes[i].name + name_position, NULL, 0);		
	}
	return 0;
}

#define MAX_CONTENIDO 100
static char fisop_file_contenidos[MAX_CONTENIDO] = "hola fisopfs!\n";

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	// Solo tenemos un archivo hardcodeado!
	if (strcmp(path, "/fisop") != 0)
		return -ENOENT;

	if (offset + size > strlen(fisop_file_contenidos))
		size = strlen(fisop_file_contenidos) - offset;

	size = size > 0 ? size : 0;

	memcpy(buffer, fisop_file_contenidos + offset, size);

	return size;
}


static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,

	.mkdir = fisopfs_mkdir,
	.rmdir = fisopfs_rmdir,
	.unlink = fisopfs_unlink,
};

int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
