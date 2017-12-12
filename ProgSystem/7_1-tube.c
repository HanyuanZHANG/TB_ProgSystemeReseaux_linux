#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

typedef void (*sighhandler_t)(int);

void hd_cld(int n);

int main() {
    int filedes[2];
    char buf;
    int nb_com = 0;
    pipe(filedes);
    int pid = fork();
    switch(pid){
        case -1: 
            perror("error creation of child process!\n");
            break;
        case 0:
            dup2(filedes[0], 0);
            while(read(filedes[0], &buf, 1) >= 0){
                write(1, &buf, 1);
                nb_com ++;
                //printf("nb_com is now : %d\n", nb_com);
                if(nb_com > 10){break;}
            }
            exit(1);
        default:
            signal(SIGCHLD, hd_cld);
            while(read(0, &buf, 1) >= 0){
                //write(filedes[1], &buf, 1);
                dup2(filedes[1], 1);
                write(1, &buf, 1);
            }
    }
}

void hd_cld(int n){
    int status;    
    int nb_fils = wait(&status);
    if(WIFEXITED(status)){
        printf("fils %d est terminé par exit de numéro : %d\n", nb_fils, WEXITSTATUS(status));
    }
    if(WIFSIGNALED(status)){
        printf("fils %d est terminé par un signal de numéro : %d\n", nb_fils, WTERMSIG(status));
    }
    exit(EXIT_SUCCESS);
}
