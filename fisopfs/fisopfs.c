
#include <errno.h>
#include <fcntl.h>
#define FUSE_USE_VERSION 30


#include <fuse/fuse.h>
#include <time.h>
#include "structures.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


/*
 *****************************
 * ----- Inicializacion -----*
 *****************************
 */

struct superblock superblock;                    // Superbloque.
int ibitmap[TOTAL_INODES];                       // Bitmap de inodos.
struct inode inodes[TOTAL_INODES];               // Arreglo de inodos.


/*
 *-------------------------------------------------*
 *			Operaciones Auxiliares				   *
 *-------------------------------------------------*
 */

// Dado un path buscamos en el bitmap de inodos aquellos
// que esten ocupados y su nombre sea el mismo que el path.
// De no haber ningun inodo que coincida devolvemos -1.
int
get_inode(const char *path)
{
	for (int i = 0; i < TOTAL_INODES; i++) {
		if (ibitmap[i] == FREE) {
			continue;
		}
		if (strcmp(inodes[i].name, path) == 0) {
			return i;
		}
	}
	return -1;
}


// Devuelve 1 si el path pasado corresponde al root, 0 si no.
int
is_root(const char *path)
{
	if (strcmp(path, ROOT) == 0) {
		return 1;
	}
	return 0;
}


// Obtiene la posicion del primer inodo libre.
// Devuelve -1 de no haber ningun inodo libre.
int
get_free_inode()
{
	for (int i = 0; i < TOTAL_INODES; i++) {
		if (ibitmap[i] == FREE) {
			return i;
		}
	}
	return -1;
}


// Inicializa un inodo.
// En caso de exito devuelvo 0, caso contrario -1.
int
initialize_inode(const char *path, int type, mode_t mode)
{
	int inode_idx = get_free_inode();
	if ((inode_idx == -1) || (strlen(path) > NAME_SIZE)){
		return -1;
	}
	ibitmap[inode_idx] = OCCUPIED;
	struct inode *current_inode = &inodes[inode_idx];
	time_t right_now = time(NULL);
	nlink_t tot_links = 0;
	if (type == TYPE_DIRECTORY) {
		tot_links = 2;
	} else {
		tot_links = 1;
	}

	current_inode->inum = inode_idx;
	current_inode->type = type;
	strcpy(current_inode->name, path);
	current_inode->mode = mode;
	current_inode->owner = getuid();
	current_inode->group = getgid();
	current_inode->size = 0;
	current_inode->ctime = right_now;
	current_inode->mtime = right_now;
	current_inode->atime = right_now;
	current_inode->links_count = tot_links;
	memset(current_inode->data,0,BLOCK_SIZE);
	return 0;
}


// Obtiene todas las dentries de un path dado.
int
get_total_dentries(const char *path)
{
	int tot_dentries = 0;
	int length = strlen(path);
	for (int i = 0; i < length; i++) {
		if (path[i] == '/') {
			tot_dentries++;
		}
	}
	return tot_dentries;
}


// Obtiene la posicion dentro del path, a partir de la cual empieza el
// nombre del archivo o directorio.
// Devuelve -1 si es que no tiene.
int
get_name_offset(const char *path)
{
	int position = -1;
	int length = strlen(path);
	for (int i = 0; i < length; i++) {
		if (path[i] == '/') {
			// Posicion de la primera letra del nombre.
			position = i + 1;  
		}
	}
	return position;
}


// A partir de la posicion del inodo en el bitmap, borra y limpia el inodo.
void
delete_inode(int i_pos)
{
	ibitmap[i_pos] = FREE;
	memset(&inodes[i_pos], 0, sizeof(struct inode));
}


// Indica mediante el path (de un directorio o archivo) si el mismo pertenece
// al directorio cuyo path es "dir_path".
// Retorna -1 en caso de que no este, 1 si esta.
int
in_dir(const char *path, const char *dir_path)
{
	if (!(strlen(path) > strlen(dir_path) && strlen(dir_path) > 0)) {
		return -1;
	}

	if (strncmp(dir_path, path, strlen(dir_path)) == 0) {
		return 1;
	}

	return -1;
}


// Indica si un path pertenece al root.
// Devuelve 1 en caso de que pertenezca, 0 si no.
int
in_root(const char *path)
{
	if (get_total_dentries(path) == 1) {
		return 1;
	}
	return 0;
}


// Realiza la persistencia del filesystem, escribiendo sobre un archivo.
void
filesystem_persistence(const char *file_name)
{
	FILE *file;
	file = fopen(file_name, "w");
	if (!file) {
		printf("No se pudo abrir archivo para la persitencia\n");
		return;
	}
	// Debemos guardar el Superbloque, bitmap de inodos e inodos en el archivo.
	fwrite(&superblock, sizeof(struct superblock), 1, file);
	fwrite(ibitmap, sizeof(int), TOTAL_INODES, file);
	fwrite(inodes, sizeof(struct inode), TOTAL_INODES, file);
	fclose(file);
}


// Inicializa el filesystem.
void
initialize_filesystem()
{
	superblock.inodes = TOTAL_INODES;
	superblock.ibitmap = ibitmap;
	filesystem_persistence(SERIALIZATION_FILE);
}


// Deserializa el filesystem, lo lee del archivo donde se guardo.
static int
deserialize_file(FILE *file_name)
{
	int result_superblock =
	        fread(&superblock, sizeof(struct superblock), 1, file_name);
	if (result_superblock < 0)
		return -1;

	int result_ibitmap =
	        fread(&ibitmap, sizeof(int), TOTAL_INODES, file_name);
	if (result_ibitmap < 0)
		return -1;

	int result_inodes =
	        fread(&inodes, sizeof(struct inode), TOTAL_INODES, file_name);
	if (result_inodes < 0)
		return -1;

	superblock.ibitmap = ibitmap;
	return 0;
}


/*
 ********************************************
 *				FUSE OPERATIONS				*
 ********************************************
 */

// Inicializa el filesystem.
static void *
fisopfs_init(struct fuse_conn_info *conn_info)
{
	printf("[debug] fisopfs_init\n");
	FILE *file = fopen(SERIALIZATION_FILE, "r");
	if (!file) {
		initialize_filesystem();

	} else {
		int deserialize = deserialize_file(file);
		if (deserialize == -1) {
			return NULL;
		}
		fclose(file);
	}
	return NULL;
}



// Hace persistencia sobre el filesystem, lo guarda en un archivo.
static void
fisopfs_destroy()
{
	printf("[debug] fisopfs_destroy\n");
	filesystem_persistence(SERIALIZATION_FILE);
}



// Obtiene los atributos relacionados a un inodo (status)
static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	if (is_root(path)) {
		st->st_uid = 1717;
		st->st_mode = __S_IFDIR | 0755;
		st->st_nlink = 2;
		return 0;
	}
	
	int inode_idx = get_inode(path);
	if (inode_idx == -1) {
		printf("El path no esta bien, inodo no encontrado\n");
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
	st->st_nlink = inode.links_count;

	if (inode.type == TYPE_FILE) {
		st->st_mode = __S_IFREG | 0644;
	} else {
		st->st_mode = __S_IFDIR | 0755;
	}

	return 0;
}



// Muestra todos el contenido de un directorio (ls)
static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);
	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	if (is_root(path)) {
		for (int i = 0; i < TOTAL_INODES; i++) {
			if (ibitmap[i] == FREE) {
				continue;
			}
			if (in_root(inodes[i].name)) {
				filler(buffer, inodes[i].name + 1, NULL, 0);
			}
		}
		return 0;
	}

	int position = get_inode(path);
	if (position == -1) {
		printf("El path no esta bien, inodo no encontrado\n");
		return -ENOENT;
	}
	struct inode directory = inodes[position];
	if (directory.type != TYPE_DIRECTORY) {
		printf("El path pasado era de tipo directorio\n");
		return -EISDIR;
	}

	directory.atime = time(NULL);

	for (int i = 0; i < TOTAL_INODES; i++) {
		if (ibitmap[i] == FREE) {
			continue;
		}
		if (in_dir(inodes[i].name, directory.name) == -1) {
			continue;
		}
		int name_position = get_name_offset(inodes[i].name);
		filler(buffer, inodes[i].name + name_position, NULL, 0);
	}
	return 0;
}



// Creamos un directorio.
static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_mkdir - path: %s\n", path);
	return initialize_inode(path, TYPE_DIRECTORY, mode);
}



// Hace unlink sobre un inodo.
static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);
	int position = get_inode(path);
	if (position == -1) {
		return -ENOENT;
	}
	if (inodes[position].type == TYPE_DIRECTORY) {
		return -EISDIR;
	}
	inodes[position].links_count--;

	if (inodes[position].links_count == 0) {
		delete_inode(position);
	}

	return 0;
}



// Remueve un directorio y todo su contenido.
static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path: %s\n", path);
	int inode_idx = get_inode(path);
	if (inode_idx == -1) {
		return -ENOENT;
	}

	if (inodes[inode_idx].type == TYPE_FILE) {
		return -ENOTDIR;
	}

	for (int i = 0; i < TOTAL_INODES; i++) {
		if (ibitmap[i] == FREE) {
			// Inodo libre.
			continue;
		}
		if (in_dir(inodes[i].name, path) == -1) {
			// Inodo que no pertenece al directorio a borrar.
			continue;
		}
		if (inodes[i].type == TYPE_FILE) {
			// De ser archivo hace unlink.
			fisopfs_unlink(inodes[i].name);
		} else {
			// De ser directorio lo elimina.
			fisopfs_rmdir(inodes[i].name);
		}
	}

	delete_inode(inode_idx);
	return 0;
}



// Truncado de archivos, para reducir su tamanio.
static int
fisopfs_truncate(const char *path, off_t offset)
{
	printf("[debug] fisopfs_truncate - path: %s\n", path);
	int inode_idx = get_inode(path);
	if (inode_idx == -1) {
		printf("El path no esta bien, inodo no encontrado\n");
		return -ENOENT;
	}
	if (inodes[inode_idx].type == TYPE_DIRECTORY){
		printf("El inodo es de tipo directorio.\n");
		return -EISDIR;
	}
	if (offset > (BLOCK_SIZE)){
		printf("No es posible realizar el truncado, el offset es mayor que el tamanio del archivo.\n");
		return 0;
	}
	

	memset(inodes[inode_idx].data + offset, 0, ((BLOCK_SIZE)-offset));
	inodes[inode_idx].size = (inodes[inode_idx].size < offset) ? inodes[inode_idx].size : offset;

	return 0;
}



// Modifica fecha de ultimo acceso y ultima modificacion del archivo.
static int
fisopfs_utimens(const char *path, const struct timespec time[2])
{
	printf("[debug] fisopfs_utimens - path: %s\n", path);
	int inode_idx = get_inode(path);
	if (inode_idx == -1) {
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


// Crea un archivo
static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_create - path: %s\n", path);
	return initialize_inode(path, TYPE_FILE, mode);
}



/*
 * Lee de un archivo
 * path 	-> Ruta del archivo a leer
 * buffer	-> Contenedor donde se guardara lo leido
 * size		-> Tamanio a leer
 * offset	-> Posicion en la cual se leera.
 */
static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s\n", path);
	if (offset < 0 || size < 0) {
		return -1;
	}

	int inode_idx = get_inode(path);
	if (inode_idx == -1) {
		printf("No se encontro el inodo del path %s\n", path);
		return -ENOENT;
	}
	if (inodes[inode_idx].type == TYPE_DIRECTORY) {
		printf("El inodo es de tipo directorio\n");
		return -EISDIR;
	}
	if (offset > inodes[inode_idx].size){
		// Verificamos que el offset no sea mayor que el tamanio del archivo.
		printf("El offset es mayor que el tamanio del archivo\n");
		return -ENOMEM;
	}
	if ( offset + size > inodes[inode_idx].size){
		// Verificamos que el offset mas los bytes a leer no sean mayores que el tamanio del archivo.
		int unreadable = (offset + size) - inodes[inode_idx].size;
		size = size - unreadable;
	}

	memcpy(buffer, inodes[inode_idx].data + offset, size);
	return size;
}



/*
 * Escribe en un archivo, de no existir lo crea.
 * path 	-> Ruta del archivo a escribir
 * buffer	-> Mensaje a escribir
 * size		-> Tamanio del mensaje a escribir
 * offset	-> Posicion del archivo en la que se esta escribiendo
 */
static int
fisopfs_write(const char *path,
              const char *buffer,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_write - path: %s\n", path);
	int max_size = BLOCK_SIZE;
	int size_after_writing = size + offset;

	if (offset > max_size){
		return -ENOMEM;
	}
	
	int inode_idx = get_inode(path);

	if (inode_idx == -1) {
		// Si el archivo no existe, lo creamos.
		int initialization = initialize_inode(path, TYPE_FILE, 0644);
		if (initialization == -1) {
			return -1;
		}
		inode_idx = get_inode(path);
	}
	if (inodes[inode_idx].type == TYPE_DIRECTORY) {
		return -EISDIR;
	}

	inodes[inode_idx].size = size_after_writing;
	inodes[inode_idx].atime = time(NULL);
	inodes[inode_idx].mtime = time(NULL);

	if (size_after_writing > max_size) {
		// Obtengo "lo que sobra" del mensaje, lo que no se puede escribir.
		int unwritable = size_after_writing - max_size;
		size = size - unwritable;
		inodes[inode_idx].size = max_size;
	}
	char message[5];
	memcpy(&message, buffer, size);
	memcpy(inodes[inode_idx].data + offset, buffer, size);
	return size;
}


static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_flush - path: %s\n", path);
	filesystem_persistence(SERIALIZATION_FILE);
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
