#include <cstdint>
#include <fuse/fuse.h>
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
// Omitimos la posicion 0, pues si una posicion en el blck_bitmap 
// del inodo es 0 significa que esta vacio.
// Devuelve -1 de no haber ningun bloque libre.
int get_free_block(){
	for (int i = 1; i < TOTAL_DATABLOCKS; i++){
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
	int inode_idx = get_inode(path);

	if (inode_idx == -1) {
		return -ENOENT;
	}
	struct inode inode = inodes[inode_idx];
	st->st_dev = 0;
	st->st_ino = inode_idx;
	st->st_uid = inode.owner;
	st->st_mode = inode.mode;
	st->st_atime = inode.atime;
	st->st_mtime = inode.mtime;
	st->st_ctime = inode.ctime;
	st->st_size = inode.size;
	st->st_gid = inode.group;
	st->st_blksize = BLOCK_SIZE;
	st->st_blocks = inode.blocks;

	if (inode.type == TYPE_FILE) {
		st->st_nlink = 1;
		st->st_mode = __S_IFREG | 0644;
	} else {
		st->st_nlink = 2;
		st->st_mode = __S_IFDIR | 0755;
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



static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	if (offset < 0 || size < 0){
		return -1;
	}

	int inode_idx = get_inode(path);
	if (inode_idx == -1){
		return -1;
	}
	if (inodes[inode_idx].type == DIRECTORY){
		return -1;
	}
	if (offset > inodes[inode_idx].blocks*BLOCK_SIZE){
		return -1;
	}
	if (offset+size > inodes[inode_idx].blocks*BLOCK_SIZE){
		size = (inodes[inode_idx].blocks*BLOCK_SIZE - offset);
	}

	inodes[inode_idx].atime = time(NULL);

	while (size != 0){
		int current_block = offset/BLOCK_SIZE;
		if (inodes[inode_idx].blck_bitmap[current_block]==FREE){
			return -1;
		}
		int offset_in_block = offset % BLOCK_SIZE;
		int in_block_to_read = BLOCK_SIZE-offset_in_block;
		int read = (size < in_block_to_read) ? size : in_block_to_read;
		memcpy(buffer, inodes[inode_idx].blockptr[current_block]->data + offset_in_block, read);
		size = size - read;
		offset = offset + read;
	}
	return 0;
}


/*
 * path 	-> Ruta del archivo a escribir
 * buffer	-> Mensaje a escribir
 * size		-> Tamanio del mensaje a escribir
*/
static int
fisopfs_write(const char *path,
              const char *buffer,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{	
	int max_size = BLOCKS_PER_INODE*BLOCK_SIZE;
	int size_after_writing = size + offset;
	if (size_after_writing > max_size){
		return -1;
	}
	int inode_idx = get_inode(path);

	
	if (inode_idx == -1){
		// Si el archivo no existe, lo creamos.
		int initialization = initialize_inode(path, TYPE_FILE, 0644);
		if (initialization == -1){
			return -1;
		}
		inode_idx = get_inode(path);
		if (inode_idx == -1){
			return -1;
		}
	}
	if (inodes[inode_idx].type == TYPE_DIRECTORY){
		return -1;
	}

	inodes[inode_idx].size = (max_size > size_after_writing) ? size_after_writing : max_size;
	inodes[inode_idx].atime = time(NULL);
	inodes[inode_idx].mtime = time(NULL);
	
	// Modifico el tamanio de size a lo que realmente vamos a escribir.
	if (size_after_writing > max_size){
		// Obtengo "lo que sobra" del mensaje, lo que no se puede escribir.
		int unwritable = size_after_writing - max_size;
		size = size - unwritable;
	}

	// Cualquiera de las dos condiciones las considero validas.
	int buffer_offset = 0;
	while (offset < inodes[inode_idx].size || size != 0){
		int current_block = offset/BLOCK_SIZE;
		if (inodes[inode_idx].blck_bitmap[current_block] == FREE){
			// Si el datablock esta lleno, reservo uno nuevo.
			int new_dblock = add_block(inode_idx);
			if (new_dblock == -1){
				return -1;
			}
		} else {
			int offset_in_block = offset % BLOCK_SIZE;
			int size_available = BLOCK_SIZE - offset_in_block;
			int bytes_to_write = (size > size_available) ? size_available : size;
			memcpy(inodes[inode_idx].blockptr[current_block]->data + offset_in_block, buffer + buffer_offset, bytes_to_write);
			buffer_offset = buffer_offset + bytes_to_write;
			size = size - bytes_to_write;
		}
	}
	// Retorna la cantidad de bytes, del buffer, escritas.
	return buffer_offset;
}

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi){
	return initialize_inode(path, TYPE_FILE, mode);
}


void 
filesystem_persistence(const char *file_name)
{
	FILE *file;
	file = fopen(file_name, "wb");
	if (!file){
		return;
	}
	// Debemos guardar el Superbloque, bitmap de datablocks,
	// bitmap de inodos, inodos y datablocks en el archivo.
	fwrite(&superblock, sizeof(struct superblock), 1, file);
	fwrite(dbitmap, sizeof(int), TOTAL_DATABLOCKS, file);
	fwrite(ibitmap, sizeof(int), TOTAL_INODES, file);
	fwrite(inodes, sizeof(struct inode), TOTAL_INODES, file);
	fwrite(data_blocks, sizeof(struct datablock), TOTAL_DATABLOCKS, file);	
	fclose(file);
}


void
initialize_filesystem()
{
	superblock.dblocks 	= 	TOTAL_DATABLOCKS;
	superblock.dbitmap 	= 	dbitmap;
	superblock.inodes 	= 	TOTAL_INODES;
	superblock.ibitmap 	= 	ibitmap;
	filesystem_persistence("fs.fisopfs");
}


static int
deserialize_file(FILE *file_name){

	int result;
	result = fread(&superblock, sizeof(struct superblock), 1, file_name);
	if (result < 0)
		return -1;
	result = fread(ibitmap, sizeof(int), TOTAL_INODES, file_name);
	if (result < 0)
		return -1;
	result = fread(dbitmap, sizeof(int), TOTAL_DATABLOCKS, file_name);
	if (result < 0)
		return -1;
	result = fread(inodes, sizeof(struct inode), TOTAL_INODES, file_name);
	if (result < 0)
		return -1;
	result = fread(data_blocks, sizeof(struct datablock), TOTAL_DATABLOCKS, file_name);
	if (result < 0)
		return -1;
	superblock.ibitmap = ibitmap;
	superblock.dbitmap = dbitmap;
	return 0;
}


static void *
fisopfs_init(struct fuse_conn_info *conn_info){
	FILE *file = fopen("fs.fisopfs", "rb");
	if (!file){
		initialize_filesystem();

	} else {
		int deserialize = deserialize_file(file);
		if (deserialize == -1){
			return NULL;
		}
		fclose(file);
	}
	return NULL;
}


static void *
fisopfs_destroy(){
	filesystem_persistence("fs.fisopfs");
}

static int
fisopfs_rmdir(const char *path)
{
	int inode_idx = get_inode(path);
	if (inode_idx == -1){
		return -1;
	}

	if (inodes[inode_idx].type == TYPE_FILE){
		return -1;
	}

	for (int i = 0; i < TOTAL_INODES; i++){
		if (ibitmap[i] == FREE){
			// Inodo libre.
			continue;
		}
		if (in_dir(inodes[i].name, path) == -1){
			// Inodo que no pertenece al directorio a borrar.
			continue;
		}
		if (inodes[i].type == TYPE_FILE){
			fisopfs_unlink(inodes[i].name);
		} else if (inodes[i].type == TYPE_DIRECTORY) {
			fisopfs_rmdir(inodes[i].name);
		} else {
			continue;
		}
	}

	
	for (int i = 0; i < inodes[inode_idx].blocks; i++){
		if (inodes[inode_idx].blck_bitmap[i] == FREE){
			continue;
		}
		memset(inodes[inode_idx].blockptr[i], NULL, sizeof(struct datablock));
		dbitmap[inodes[inode_idx].blck_bitmap[i]] = FREE;
	}
	memset(&inodes[inode_idx], NULL, sizeof(struct inode));
	ibitmap[inode_idx] = FREE;
	return 0;
}


static int
fisopfs_truncate(const char *path, off_t offset)
{
	int inode_idx = get_inode(path);
	if (inode_idx == -1){
		return -1;
	}
	// Del bloque actual en adlente tengo que borrar todo.
	int current_block = offset/BLOCK_SIZE;
	int offset_in_block = offset % BLOCK_SIZE;
	memset(&inodes[inode_idx].blockptr[current_block]->data[offset_in_block], 0, (BLOCK_SIZE - offset_in_block) * sizeof(char));
	inodes[inode_idx].size = inodes[inode_idx].size - (BLOCK_SIZE - offset_in_block);
	current_block ++;

	for (int i = current_block; i < BLOCKS_PER_INODE; i++){
		if (inodes[inode_idx].blck_bitmap[i] == FREE){
			continue;
		}
		memset(inodes[inode_idx].blockptr[i], 0, sizeof(struct datablock));
		int blck_idx = inodes[inode_idx].blck_bitmap[i];
		dbitmap[blck_idx] = FREE;
		inodes[inode_idx].blck_bitmap[i] = FREE;
		inodes[inode_idx].size = inodes[inode_idx].size - BLOCK_SIZE;
	}
	return 0;
}


static int
fisopfs_utimens(const char *path, const struct timespec time[2])
{
	int inode_idx = get_inode(path);
	if (inode_idx == -1){
		return -1;
	}
	if (time == NULL) {
		errno = EACCES;
		return -EACCES;
	}
	inodes[inode_idx].atime = time[0].tv_sec;
	inodes[inode_idx].mtime = time[1].tv_sec;
	return 0;
}


static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	filesystem_persistence("fs.fisopfs");
	return 0;
}


static struct fuse_operations operations = {
	.init = fisopfs_init,        
	.destroy = fisopfs_destroy,  
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.mkdir = fisopfs_mkdir,
	.unlink = fisopfs_unlink,
	.rmdir = fisopfs_rmdir,
	.truncate = fisopfs_truncate,
	.utimens = fisopfs_utimens,
	.create = fisopfs_create,
	.read = fisopfs_read,
	.write = fisopfs_write,
	.flush = fisopfs_flush,
};

int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
