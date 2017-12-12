#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>



#define BUFSIZE 512
enum TypeFichier { NORMAL, REPERTOIRE, ERREUR };

const char* OK200 = "HTTP/1.1 200 OK\r\n\r\n";
const char* ERROR403 = "HTTP/1.1 403 Forbidden\r\n\r\nAccess denied\r\n";
const char* ERROR404 = "HTTP/1.1 404 Not Found\r\n\r\nFile or directory not found\r\n";

// fonctions ajoutees
int status;
int fils;

void fin_fils(int n) {
  fils = wait(&status);
  printf("Fils numero: %d\n", fils);
  //exit(EXIT_SUCCESS);
}

/* Fonction typeFichier()
 * argument: le nom du fichier
 * rend une valeur de type enumeration delaree en tete du fichier
 * NORMAL, ou REPERTOIRE ou ERRREUR
 */
enum TypeFichier typeFichier(char *fichier) {
  struct stat status;
  int r;

  r = stat(fichier, &status);
  if (r < 0)
    return ERREUR;
  if (S_ISREG(status.st_mode))
    return NORMAL;
  if (S_ISDIR(status.st_mode))
    return REPERTOIRE;
  /* si autre type, on envoie ERREUR (a fixer plus tard) */
  return ERREUR;
}


/* envoiFichier()
 * Arguments: le nom du fichier, la socket
 * valeur renvoyee: true si OK, false si erreur
 */
#define BUSIZE 1048
bool envoiFichier(char *fichier, int soc) {
  int fd;
  char *buf;
  ssize_t nread;
  int *len = BUSIZE;

  /* A completer.
   * On peut se poser la question de savoir si le fichier est
   * accessible avec l'appel systeme access();
   * Si oui, envoie l'entete OK 200 puis le contenu du fichier
   * Si non, envoie l'entete ERROR 403
   *
   * Note: le fichier peut etre plus gros que votre buffer,
   * de meme il peut etre plus petit...
   */
   fd = access(fichier, R_OK);
   if (fd == -1){
     perror("error accessiblity\n");
     write(soc, ERROR403, strlen(ERROR403));
     return false;
   }else{
     printf("start reading file ...\n");
     /*
     FILE *f = fopen(fichier, "rb");
     if(f){
       fseek(f, 0, SEEK_END);
       nread = ftell(f);
       fseek(f, 0, SEEK_SET);
       buf = malloc(nread + 1);
       if(buf){
         fread(buf, 1, nread, f);
       }
       fclose(f);
       buf[nread] = '\0';
     }*/
     fd = open(fichier, buf, BUSIZE);
     write(soc, OK200, strlen(OK200));
     //sendfile(fd, soc, 0, len, NULL, 0);
     sendfile(soc, fd, NULL, BUSIZE);
     /*
     printf("file content: %s\n", buf);
     write(soc, OK200, strlen(OK200));
     write(soc, buf, strlen(buf));*/
     return true;
   }
}


/* envoiRep()
 * Arguments: le nom du repertoire, la socket
 * valeur renvoyee: true si OK, false si erreur
 */
bool envoiRep(char *rep, int soc) {
  DIR *dp;
  struct dirent *pdi;
  char buf[1024], nom[1024];

  dp = opendir(rep);
  if (dp == NULL)
    return false;

  write(soc, OK200, strlen(OK200));
  sprintf(buf, "<html><title>Repertoire %s</title><body>", rep);
  write(soc, buf, strlen(buf));

  while ((pdi = readdir(dp)) != NULL) {
    /* A completer
     * Le nom de chaque element contenu dans le repertoire est retrouvable a
     * l'adresse pdi->d_name. Faites man readdir pour vous en convaincre.
     * Dans un premier temps, on se contentera de la liste simple.
     * Dans une petite amelioration on poura prefixer chaque element avec
     * l'icone folder ou generic en fonction du type du fichier.
     * (Tester le nom de l'element avec le chemin complet.) */
    if(pdi->d_type == DT_REG){
      sprintf(buf, "<br><img src='icons/generic.gif' alt='file '/>");
    }else if(pdi->d_type == DT_DIR){
      sprintf(buf, "<br><img src='icons/folder.gif' alt='folder '/>");
    } 
    write(soc, buf, strlen(buf));
    write(soc, pdi->d_name, strlen(pdi->d_name));
    write(soc, "\n\r", 2);
  }
  write(soc, "\n\r", 2);
  return true;
}


void communication(int soc, struct sockaddr *from, socklen_t fromlen) {
  int s;
  char buf[BUFSIZE];
  ssize_t nread;
  char host[NI_MAXHOST];
  bool result;
  char *pf;
  enum op { GET, PUT } operation;
  
  

  /* Eventuellement, inserer ici le code pour la reconnaissance de la
   * machine appelante */
    s = getnameinfo(from, fromlen,
		    host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
    if (s == 0)
      printf("Debut avec client '%s'\n", host);
    else
      fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));


  /* Reconnaissance de la requete */
  nread = read(soc, buf, BUFSIZE);

  printf("request socket %s\n", buf);

  if (nread > 0) {
    buf[nread] = '\0';
    if (strncmp(buf, "GET", 3) == 0)
      operation = GET;
  } else {
    perror("Erreur lecture socket");
    return;
  }

  //printf("operation detected : %s\n", operation);

  switch (operation) {
    case GET:
      pf = strtok(buf + 4, " ");
      /* On pointe alors sur le / de "GET /xxx HTTP...
       * strtok() rend l'adresse du premier caractere
       * apres l'espace et termine le mot par '\0'
       */
      pf++; /* pour pointer apres le slash */
      /* pf pointe sur le nom du fichier suivant le / de la requete.
       * Si la requete etait "GET /index.html ...", alors pf pointe sur
       * le "i" de "index.html"
       */
      /* si le fichier est un fichier ordinaire, on l'envoie avec la fonction
       * envoiFichier().
       * Si c'est un repertoire, on envoie son listing avec la fonction
       * envoiRep().
       * Vous pouvez utiliser la fonction typeFichier() ci-dessous pour tester
       * le type du fichier.
       */
      printf("name of file: %s\n", pf);
      printf("type of file : %d\n", typeFichier(pf));
      switch(typeFichier(pf)){
        case NORMAL:
          printf("envoi fichier start !\n");
          envoiFichier(pf, soc);
          break;
        case REPERTOIRE:
          printf("envoi repo start !\n");
          envoiRep(pf, soc);
          break;
        default:
          perror("cannot treat file!\n");
          break;
      }
      break;
    default:
      write(soc, ERROR403, strlen(ERROR403));
      break;
  }
  close(soc);
}


int main(int argc, char **argv) {
  int sfd, s, ns, r, pid;
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  struct sockaddr_storage from;
  socklen_t fromlen;
  char buf[BUFSIZE];
  ssize_t nread, nwrite;
  char host[NI_MAXHOST];
  char *message = "Message a envoyer: ";


  if (argc != 2) {
    printf("Usage: %s  port_serveur\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Inserer ici le code d'un serveur TCP concurent */
  
  /* Construction de l'adresse locale (pour bind) */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;           /* Force IPv6 */
  hints.ai_socktype = SOCK_STREAM;      /* Stream socket */
  hints.ai_flags = AI_PASSIVE;          /* Adresse IP joker */
  hints.ai_flags |= AI_V4MAPPED|AI_ALL; /* IPv4 remapped en IPv6 */
  hints.ai_protocol = 0;                /* Any protocol */

  s = getaddrinfo(NULL, argv[1], &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  /* getaddrinfo() retourne une liste de structures d'adresses.
     On essaie chaque adresse jusqu'a ce que bind(2) reussisse.
     Si socket(2) (ou bind(2)) echoue, on (ferme la socket et on)
     essaie l'adresse suivante. cf man getaddrinfo(3) */
  for (rp = result; rp != NULL; rp = rp->ai_next) {

    /* Creation de la socket */
    sfd = socket(rp->ai_family, rp->ai_socktype, 0);
    if (sfd == -1)
      continue;

    /* Association d'un port a la socket */
    r = bind(sfd, rp->ai_addr, rp->ai_addrlen);
    if (r == 0)
      break;            /* Succes */
    close(sfd);
  }

  if (rp == NULL) {     /* Aucune adresse valide */
    perror("bind");
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(result); /* Plus besoin */

  /* Positionnement de la machine a etats TCP sur listen */
  if(listen(sfd, 5) == -1){
    perror("error listen!");
    exit(EXIT_FAILURE);
  }

  for (;;) {
    /* Acceptation de connexions */
    fromlen = sizeof(from);
    ns = accept(sfd, (struct sockaddr *)&from, &fromlen);
    if (ns == -1) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    /* Reconnaissance de la machine cliente */
    /*
    s = getnameinfo((struct sockaddr *)&from, fromlen,
			host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
    if (s == 0)
      printf("Debut avec client '%s'\n", host);
    else
      fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));*/


    // concurrent
    pid = fork();
    switch(pid){
        case -1: 
            perror("error fils creation\n");
            exit(EXIT_FAILURE);
        case 0:
            close(sfd);
            //for (;;) {
              /*
              nwrite = write(ns, message, strlen(message));
              if (nwrite < 0) {
	            perror("write");
	            break;
              }*/
              /*
              nread = read(ns, buf, BUFSIZE);
              if (nread == 0) {
	            printf("Fin avec client '%s'\n", host);
	            break;
              } else if (nread < 0) {
	            perror("read");
	            break;
              }
              buf[nread] = '\0';
              printf("Message recu '%s'\n", buf);*/
            communication(ns, (struct sockaddr *)&from, &fromlen);
            //}
            exit(EXIT_SUCCESS);
        default: 
            close(ns);
            signal(SIGCHLD, fin_fils);
    
    }
}
}
