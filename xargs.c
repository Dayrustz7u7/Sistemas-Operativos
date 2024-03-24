#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#ifndef NARGS
#define NARGS 4
#endif


void
ejecutarComandoAlEmpaquetado(char *argumentos[])
{
	pid_t pid = fork();
	if (pid == 0) {  // PROCESO DEL HIJO
		execvp(argumentos[0], argumentos);
	} else {
		// espero el proceso del hijo para que ejecute los comandos
		wait(NULL);
	}
}

char *
procesarLinea(char *linea)
{
	// Elimino el \n de la linea
	linea[strcspn(linea, "\n")] = '\0';
	return strdup(linea);
}

int
main(int argc, char *argv[])
{
	// Referencia de entrada -> [ARGUMENTOS] | ./xargs <Comando>
	if (argc < 2) {
		exit(1);
	}

	// Crearemos nuestro array de punteros donde estara el comando y los argumentos
	// esto para cumplir la sintaxis de la documantacion de execvp
	char *comandoLeido = argv[1];
	char *argumentos[NARGS + 2] = {
		NULL
	};  // 2 posiciones mas debido al comando y al ultimo elemento NULL
	argumentos[0] =
	        comandoLeido;  // [comando, NULL] . A medida que se va añadiendo
	                       // elementos.El NULL se ira trasladando en la ultima posicion

	// VENTAJA: IREMOS ACTUALIZANDO EL CONTENIDO DE DICHO ARRAY con los
	// nuevos argumentos segun el LIMITE DE NARGS
	//          EXCEPTO EL PRIMER ELEMENTO, QUE ES EL COMANDO
	char *Linea = NULL;
	size_t len = 0;
	size_t cantidadDeArgumentosLeidos = 0;
	while (getline(&Linea, &len, stdin) !=
	       -1) {  // Aqui comenzara la lectura de los argumentos
		argumentos[cantidadDeArgumentosLeidos + 1] = procesarLinea(Linea);
		cantidadDeArgumentosLeidos++;
		if (cantidadDeArgumentosLeidos ==
		    NARGS) {  // Cuando llegue a completar el empaquetado se ejecutara el comando
			ejecutarComandoAlEmpaquetado(argumentos);
			for (int i = 0; i < NARGS + 1; i++) {
				free(argumentos[i + 1]);  // al tratarse de un stdup, liberamos la memoria con free
				argumentos[i + 1] =
				        NULL;  // no se libera el primer elemento ya que es el comando
			}
			cantidadDeArgumentosLeidos = 0;
		}
	}


	if (cantidadDeArgumentosLeidos >
	    0) {  // si aun quedo argumentos que falta ejecutar pero no llego alNARGS
		for (int i = cantidadDeArgumentosLeidos + 1; i < NARGS + 1; i++) {
			free(argumentos[i]);
			argumentos[i] = NULL;
		}
		ejecutarComandoAlEmpaquetado(argumentos);
	}
	free(Linea);

	return 0;
}
