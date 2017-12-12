/* 
 * Auteur(s):
 */

#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

typedef void (*sighandler_t)(int);

void hdl1(int n){
    wait(NULL);
}
void travail() {
  /* Je travaille tres intensement !    */
  /* Ne cherchez pas a comprendre ;-) */
  /* Il n'y a rien a modifier ici     */
  for (;;) {
    printf("%s", random() > RAND_MAX>>1 ? "." : "\b \b");
    fflush(stdout);
    usleep(100000);
  }
}
void travail() __attribute__((noreturn));
/* Petit raffinement pour le compilateur: cette fonction ne termine pas */


int main() {
  int pid, longueur;
  char tab[256];

  int r;

  pid = fork();

  if (pid != 0) {	/* Processus Pere */
    signal(SIGCHLD, hdl1);
    travail();
  } else {		/* Processus Fils */
    sleep(5);
    printf("Ahrg!!! je suis le fils et je meurs\n");
    exit(EXIT_SUCCESS);
  }
}
