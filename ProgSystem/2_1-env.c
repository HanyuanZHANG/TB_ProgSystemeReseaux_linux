#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
    extern char **environ;
    char *env_value;
    int i = 0;

    while(environ[i] != NULL){
        printf("%s\n", environ[i]);
        i++;
    }
    //return EXIT_SUCCESS;

    if (argc == 2){
        env_value = getenv(argv[1]);
        
        if (env_value == NULL){
            printf("non exist !");        
        }
        else{
            printf("%s\n", env_value);
        }
    return EXIT_SUCCESS;    
    }
}

