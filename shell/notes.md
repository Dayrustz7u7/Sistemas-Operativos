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


2.2 Dentro de runCmd al momento de hacer el parseline, creara 2 cmd tanto el izquierdo y el derecho, para finalizar creando un structura pipe y conectando estos dos cmd y devolviendo un solo cmd
eargv y eargc se completa en -> parse_environ_var
OJO -> ESTO SOLO SI SE PASAN MAS DE 2 COMANDOS

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
//---------------------------------------------------------------------------------------------
PARA EL PUNTO 3.1

Seguimiento y explicacion: Usaremos getenv(3) -> Devuelve el valor de la Variable De entorno
                                             -> Ejemplo get("HOME") -> devolvera el directorio
                                                principal del usuario -> /home/marioRafael
Trabajaremos en parse_exec()-> expand_environ_var() de parcing.c
                            -> get_token(buf,indx) lo que hace es devolver una cadena de 
                                                   caracteres hasta el espacio encontrado o el 
                                                   "\0" del buf
                                                ejmp-> buf -> "ls -l /home/user"
                                                    -> getToken -> "ls" 
                            -> se ira añdiendo a argv si solo si no es vacio -> ""
PRUEBAS:
 1.env_unset_variable: true
 2.env_unset_variable_middle: error / no se porque no me anda esta prueba pero en local si
                              pasa
 3.env_substitution: true
 4.env_magic_variable: false // ESTO ES PORQUE AUN FALTA LA VERIFICACION DE -> "$?"
 5.env_large_variable: true
 6.env_empty_variable: true
 7.env_empty_middle_variable: error // esta es la misma prueba que el segundo y si pasa. Sera
                             porque no encuentra el archivo?
  
