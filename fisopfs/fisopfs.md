# fisop-fs

Lugar para respuestas en prosa y documentación del TP.

## Documentación de diseño

# Estructuras

El filesystem implementado consta de 3 estructuras principales:
 - 1 Superbloque.
 - 1 Bitmap de inodos.
 - 80 Inodos.

# Superbloque
El superbloque contiene metadata de todo el filesystem, la cual es, la cantidad total de inodos presentes en el mismo y un puntero al comienzo de bitmap de inodos.

# Bitmap de inodos
El bitmap de inodos es una estructura que mapea cada inodo e indica si estan ocupados o libres mediante 1's y 0's, donde 1 es ocupado y 0 es libre. 
El mismo se implementa mediante un arreglo y se utiliza para saber que inodos estan disponibles, para ser usados, dentro del arreglo de inodos.

# Inodos
Los inodos son estructuras que contienen tanto la data como la metadata de un archivo, para una mayor simplicidad se guarda la data dentro del propio inodo, en lugar de utilizar datablocks, estos se encuentran almacenados en un arreglo de inodos, donde cada posicion del mismo es referenciada por el bitmap de inodos.
Los campos que componen al inodo son los siguientes:
- inum          -> Posicion del inodo dentro del arreglo (numero de inodo).
- tipo          -> Tipo de inodo, si es un archivo o directorio.
- nombre        -> Nombre del inodo, incluyendo ruta completa.
- modo          -> Si el archivo puede ser leido/ escrito/ ejecutado.
- dueño         -> User id del creador del archivo.
- grupo         -> Group id.
- tamaño        -> Tamaño del archivo, cuanto esta en uso realmente.
- ctime         -> Hora en la que fue creado.
- mtime         -> Hora en la que fue modificado.
- atime         -> Hora en la que se accedio por ultima vez.
- links_count   -> Cantidad de hard links.
- data          -> Data del inodo.


## Como se encuentra un archivo especifico a partir de un path.

Para encontrar un archivo mediante un path contamos con la siguiente funcion auxiliar:

```c
int get_inode(const char *path)
```


