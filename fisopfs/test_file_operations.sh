#!bin/bash

# Se crean 2 archivos, uno mediante touch y otro mediante write, ademas 
# se los escribe a ambos, se lee su contendio, se hace un touch y un stat.
# Por ultimo se los elimina.

cd prueba

echo "------------------------------"
echo "Creo un archivo mediante touch"
echo "------------------------------"

touch archivo_touch.txt

echo "Verifico que se haya creado mediante ls y viendo sus stats"

ls

stat archivo_touch.txt

echo "Escribo dentro del archivo"

echo "Este archivo fue creado con Touch!" > archivo_touch.txt

echo "Leemos el archivo"

cat archivo_touch.txt

echo "Por ultimo vemos sus stats actualmente, luego realizamos un touch y vemos como se modifican los tiempos"

stat archivo_touch.txt

touch archivo_touch.txt

stat archivo_touch.txt

echo "Eliminamos el archivo"

unlink archivo_touch.txt

ls

echo "------------------------------"
echo "Por ultimo creamos otro archivo mediante write y leemos su contenido"
echo "------------------------------"

echo "Este archivo fue creado con Write!!" > archivo_write.txt

ls

stat archivo_write.txt

cat archivo_write.txt

echo "Borramos el archivo creado mediante Write"

unlink archivo_write.txt

ls