## Notes on sh.c

1. init shell 
    - Crea buffer de 1024 (BUFLEN)
    - Obtiene la ruta de home 
    - chdir cambia de directorio y te manda a al home 
    - en otro caso falla y muestra por pantalla 
2. run shell
    - Genera un cmd
    - lee del hace readline del promp y si cmd es igual a salir exit cierra el programa 

Para el punto 1.1

Se llega al punto de ejecucion de la siguiente manera:

1. Init Shell, que te deberia dejar parado en el directorio home (/)
2. Run cmd que decide que toca ejecutarse, en este caso llega hasta exec_cmd()
3. exec_cmd que recibe un cmd que tiene este struct 
```c 
    struct cmd {
        int type;
        pid_t pid;
        char scmd[BUFLEN];
    };
```

Supongo que se debe leer ese scmd que trae toda la data adentro e ir desempaquetandolo. YA la data viene parseada asi que no deberia significar un problema, en si tenemos el cmd hecho

3. Hacer el casteo de struct cmd a struct execcmd 
5. llamar a execvp()

