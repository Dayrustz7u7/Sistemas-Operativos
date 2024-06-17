#!bin/bash

# Se crean tres directorios, uno dentro de otro, y luego se 
# elimina el directorio padre.

cd prueba

echo "creo un directorio"

mkdir directorio1

ls

cd directorio1

echo "creo otro directorio dentro"

mkdir directorio1.1

ls

cd directorio1.1

echo "creo un archivo dentro del nuevo directorio"

touch otro_archivo.txt

ls

echo "nos movemos a root y eliminamos el directorio"

cd ..
cd ..

echo "Borramos el directorio directorio1"

rmdir directorio1

ls

