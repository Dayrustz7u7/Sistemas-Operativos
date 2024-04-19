#include "pipehandler.h"


#define READ 0  //File descriptor de lectura.
#define WRITE 1 //FIle descriptor de escritura.




/*
ESTUVE MEDIA HORA TRATANDO DE QUE ENTRE Y FUNQUE Y NO FUNCIONABA.
bool has_delim(char buf, const char* delim){

    for (int i=0; i<strlen(buf); i++){
        printf("%c\n",buf[i]);
        if (buf[i]==delim){
            return true;
        }
    }
    return false;

}
*/

void parse_arguments(char buf, const char* delim, const char** args){
    //Parsea los argumentos para poder pasarlos al execvp.
    char* token = strtok(buf, delim);
    int i = 0;
    while (token != NULL){
        args[i++]=token;
        token = strtok(NULL, delim);
    }
    args[i] = NULL;
}


void handle_pipe(struct pipecmd *cmd){

    int pipefd[2];  //El padre crea un pipe que sera usado por los hijos.
    pipe(pipefd);   //El pipe va a usarse para pasar informacion de un hijo a otro (un solo pipe)

    pid_t pid = fork();

    if (pid < 0){
        perror("fork");
    }else if (pid == 0){
        //Hijo (Trabajo con la rama izquierda aca)
        close(pipefd[READ]); //Va a escribir su salida en el pipe, elimino el READ.
        dup2(pipefd[WRITE], STDOUT_FILENO); //Reemplaza stdout con el pipe.
        close(pipefd[WRITE]); //cierra el pipe (fd del pipe)

        const char* args[100];
        parse_arguments(cmd->leftcmd->scmd, " ", args);
        execvp(args[0], args);
        perror("execvp");

    }else{
        //Padre
        pid = wait((int *) 0);

        //Busco si el siguiente elemento tiene otro pipe.
        struct pipecmd *p;
        bool another_pipe = false;
        for (int i=0; i<strlen(cmd->rightcmd->scmd) ; i++ ){
            if (cmd->rightcmd->scmd[i]=='|'){
                p = (struct pipecmd *) parse_line(cmd->rightcmd->scmd);
                another_pipe = true;
            }
        }
        if (another_pipe){
            handle_pipe(p);
        } else{
            pid_t pid2 = fork();
            if (pid2<0){
                perror("fork");
            } else if(pid2==0){
                //Hijo (rama derecha)
                close(pipefd[WRITE]); //Va a recibir su stdin en el pipe, elimino el WRITE.
                dup2(pipefd[READ], STDIN_FILENO); //Reemplaza stdin con el pipe.
                close(pipefd[READ]); //cierra el pipe (fd del pipe)

                const char* args[100];
                parse_arguments(cmd->rightcmd->scmd, " ", args);
                execvp(args[0], args);
                perror("execvp");

            } else{
                pid2 = wait((int *) 0);
            }
        }

    }
    
    
}

