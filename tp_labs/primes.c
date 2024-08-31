#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>  //USO DEL WAIT para esperar al el proceso del hijo se muera


#define WRITE 1
#define READ 0

void
filtrador(int pipeIzquierdo[])
{
	// AHORA LA LECTURA GENERAL ESTARA DENTRO DE LA FUNCION
	int primo;

	// CONDICION DE CORTE AL NO TENER NADA MAS PARA LEER
	if (read(pipeIzquierdo[READ], &primo, sizeof primo) == 0) {
		close(pipeIzquierdo[READ]);
		close(pipeIzquierdo[WRITE]);
		return;
	}

	printf("primo %d\n", primo);
	fflush(stdout);  // Me di cuenta que en la salida se repite 2 veces, el numero. Esta linea soluciona esto.

	// CREACION DEL PIPE DERECHO PARA CONECTAR A LOS HIJOS SECUENCIALES
	int pipeDerecho[2];
	if (pipe(pipeDerecho) < 0) {
		printf("error al crear el pipe Derecho\n");
		exit(-1);
	}

	pid_t pideSecuenciales = fork();

	if (pideSecuenciales ==
	    0) {  // SON LOS HIJOS QUE SE ENCARGARA DE CREAR SU PROPIO PIPE Y VERIFICACION

		close(pipeDerecho[WRITE]);
		close(pipeIzquierdo[READ]);
		filtrador(pipeDerecho);
		close(pipeIzquierdo[WRITE]);
		close(pipeDerecho[READ]);
		exit(0);
	} else {  // SON LOS PADRES, QUE SE ENCARGARA DE ESCRIBIR EN EL PIPE DERECHO

		close(pipeDerecho[READ]);
		int candidatoAPrimo;

		while (read(pipeIzquierdo[READ],
		            &candidatoAPrimo,
		            sizeof(candidatoAPrimo)) != 0) {
			if (candidatoAPrimo % primo != 0) {
				write(pipeDerecho[WRITE],
				      &candidatoAPrimo,
				      sizeof(candidatoAPrimo));
			}
		}
		close(pipeDerecho[WRITE]);
		close(pipeIzquierdo[WRITE]);
		close(pipeIzquierdo[READ]);
		wait(NULL);
	}
}

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		exit(1);
	}

	//----Creacion del primer pipe para la comunicacion entre padre e hijo ----
	int padreHijoFds[2];

	if (pipe(padreHijoFds) < 0) {
		printf("error al crear el pipe inicial\n");
		exit(-1);
	}
	//-------------------------------------------------------------------------
	int totalNumeros = 0;
	totalNumeros = atoi(argv[1]);

	pid_t pid_inicial = fork();


	if (pid_inicial == 0) {  // LOGICA DEL PRIMER HIJO
		close(padreHijoFds[WRITE]);  // NO VAS A ESCRIBIR A TU PADRE, SINO LEER, POR ESO LO CIERRO

		filtrador(padreHijoFds);  // PASAMOS  PIPE PARA QUE LEA LOS NUMEROS QUE YA ESTAN CARGADOS

		close(padreHijoFds[READ]);  // Cerramos el extremo de lectura del pipe en el proceso hijo inicial
		exit(0);
	} else {                            // LOGICA DEL PRIMER PADRE
		close(padreHijoFds[READ]);  // NO VAS A LEER, SINO ESCRIBIR, POR ESO LO CIERRO
		for (int i = 2; i <= totalNumeros; i++) {
			// printf("Escribo el valor <%i> a traves del pipe fd=%d \n", i, padreHijoFds[WRITE]);
			write(padreHijoFds[WRITE], &i, sizeof(i));
		}

		close(padreHijoFds[WRITE]);
		wait(NULL);
	}


	return 0;
}