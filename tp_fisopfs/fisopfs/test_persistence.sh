# Se crean 2 archivos, se escriben dentro, luego un directorio con un archivo
# al cual tambien se le escribe, salimos de prueba y volvemos a entrar
# para verificar que haya persistido de forma correcta.

cd prueba

echo "Se crean dos archivos y se los escriben"

echo "Guau guau" > perro.txt
echo "Miau miau" > gato.txt

ls

cat perro.txt
cat gato.txt

echo "Se crean un directorio y un archivo dentro al cual se escribe"

mkdir granja

cd granja

echo "Muuu" > vaca.txt

ls

cat vaca.txt

cd ..

echo "Abandonamos el directorio prueba"

cd ..

echo "Volvemos a entrar al directorio prueba y verificamos que todo siga igual"

cd prueba

ls

cat perro.txt
cat gato.txt
cd granja
ls
cat vaca.txt

cd ..
rmdir granja
unlink perro.txt
unlink gato.txt
ls

echo "That's it folks"
