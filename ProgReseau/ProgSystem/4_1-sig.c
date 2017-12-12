/* 
 * Auteur(s):
 */

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
typedef void (*sighandler_t)(int);
void (*sig_avant)(int);		/* pour la question 4.3 */
void hdl_sys1(int n) {
  printf("message numero 1\n");

}
void hdl_sys2(int n) {
  printf("message numero 2\n");

}

void hdl_sysQuit(int n) {
  //sig_avant = signal(SIGINT, sig_avant);
}


void travail() {
  /* Je travaille tres intensement !    */
  /* Ne cherchez pas a comprendre ;-) */
  /* Il n'y a rien a modifier ici     */
  const char msg[] = "-\\|/";
  const int sz = strlen(msg);
  int i = 0;

  for (;;) {
    write(STDOUT_FILENO, "\r", 1);
    usleep(100000);
    write(STDOUT_FILENO, " => ", 4);
    write(STDOUT_FILENO, &msg[i++], 1);
    if (i == sz) i = 0;
  }
}
void travail() __attribute__((noreturn));
/* Petit raffinement pour le compilateur: cette fonction ne termine pas */


int main() {
  printf("PID: %d\n", getpid());
  /*
  signal(SIGINT,hdl_sys1);
  sig_avant = signal(SIGINT, hdl_sys2);
  
  signal(SIGQUIT, hdl_sysQuit);*/

  struct sigaction act;
  act.sa_sigaction = &hdl_sys1;
  act.sa_flags = SA_SIGINFO;

  sigaction(SIGINT, act, SIG_DFL);

  struct sigaction act_quit;
  act_quit.sa_sigaction = &hdl_sysQuit;
  act_quit.sa_flags = SA_SIGINFO;

  sigaction(SIGQUIT, act_quit, SIG_DFL);

  struct sigaction act_2;
  act_2.sa_sigaction = &hdl_sys2;
  act_2.sa_flags = SA_SIGINFO;

  /* ? ? ? ? ? ? */
  
  travail();
}
