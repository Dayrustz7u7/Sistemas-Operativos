#include "pipehandler.h"
#include "parsing.h"


#define READ 0  //File descriptor de lectura.
#define WRITE 1 //FIle descriptor de escritura.

bool has_other_pipe(const char* buf){
    //Se fija si tiene otro pipe mas dentro de la linea de comandos ingresados.
    for (int i=0; i<strlen(buf); i++){
        if (buf[i]=='|'){
            return true;
        }
    }
    return false;
}


void parse_arguments(char *buf, const char *delim, const char **args) {
    //Parsea los argumentos para poder pasarlos al execvp.
    char *token = strtok(buf, delim);
    int i = 0;
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, delim);
    }
    args[i] = NULL;
}


void last_leaf(struct pipecmd *cmd, int readingPipe[2]){
    pid_t pid2 = fork();
    
    if (pid2<0){
        perror("fork");

    } else if(pid2==0){
        //Hijo (rama derecha)
        
        dup2(readingPipe[READ], STDIN_FILENO);   //Reemplaza stdin con el pipe.
        close(readingPipe[READ]);                //cierra el pipe (fd del pipe)

        const char* args[100];
        parse_arguments(cmd->rightcmd->scmd, " ", args);
        execvp(args[0], args);
        perror("execvp");

    } else{
        //Padre, termina el proceso sin hacer nada mas.
        pid2 = wait((int *) 0);
    }
}

void handle_multiple_pipes(struct pipecmd *cmd, int readingPipe[2]){
    int pipefd[2];  //El padre crea un pipe que sera usado por los hijos.
    pipe(pipefd);   //El pipe va a usarse para pasar informacion de un hijo a otro (un solo pipe)

    pid_t pid = fork();

    if (pid<0){
        perror("fork");
    } else if (pid==0){
        //Hijo (Trabajo con rama izquierda que ahora recibe argumentos del pipe)

        close(pipefd[READ]);                        //Va a escribir su salida en el pipe, elimino el READ.
        dup2(readingPipe[READ], STDIN_FILENO);      //Reemplaza el stdin con el pipe del anterior.
        dup2(pipefd[WRITE], STDOUT_FILENO);         //Reemplaza stdout con el pipe.
        close(readingPipe[READ]);                   //cierra el pipe (fd del pipe)
        close(pipefd[WRITE]);                       //cierra el pipe (fd del pipe)

        const char* args[100];
        parse_arguments(cmd->leftcmd->scmd, " ", args);
        execvp(args[0], args);
        perror("execvp");
        
    } else{
        //Padre
        pid = wait((int *) 0);
        close(pipefd[WRITE]); 
        struct pipecmd *p;

        if (has_other_pipe(cmd->rightcmd->scmd)){  //Si tiene otro pipe, llamamos recursivamente
            p = (struct pipecmd *) parse_line(cmd->rightcmd->scmd);
            handle_multiple_pipes(p, pipefd);
        } else{                                    //Si NO tiene otro pipe, trabajamos sobre la ultima hoja.
            p = (struct pipecmd *) cmd ->rightcmd;
            last_leaf(p, pipefd);
        }

    }


}

void handle_pipe(struct pipecmd *cmd){

    int pipefd[2];  //El padre crea un pipe que sera usado por los hijos.
    pipe(pipefd);   //El pipe va a usarse para pasar informacion de un hijo a otro (un solo pipe)

    pid_t pid = fork();

    if (pid < 0){
        perror("fork");
    }else if (pid == 0){
        //Hijo (Trabajo con la rama izquierda aca)
        close(pipefd[READ]);                    //Va a escribir su salida en el pipe, elimino el READ.
        dup2(pipefd[WRITE], STDOUT_FILENO);     //Reemplaza stdout con el pipe.
        close(pipefd[WRITE]);                   //cierra el pipe (fd del pipe)

        const char* args[100];
        parse_arguments(cmd->leftcmd->scmd, " ", args);
        execvp(args[0], args);
        perror("execvp");

    }else{
        //Padre
        pid = wait((int *) 0);
        close(pipefd[WRITE]);   //No va a escribir nunca en este pipe, ni el ni sus hijos.
       struct pipecmd *p;

        if (has_other_pipe(cmd->rightcmd->scmd)){  //Si tiene otro pipe, llamamos recursivamente
            p = (struct pipecmd *) parse_line(cmd->rightcmd->scmd);
            handle_multiple_pipes(p, pipefd);
        } else{                                    //Si NO tiene otro pipe, trabaamos sobre la ultima hoja.
            p = (struct pipecmd *) cmd ->rightcmd;
            last_leaf(p, pipefd);
        }

    }
    
    
}

