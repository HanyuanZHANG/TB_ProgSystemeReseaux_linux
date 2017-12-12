/*
 * Auteur(s):
 *
 * Cet programme refait ce que fait la commande "ls". Il donne des
 * informnations sur les caracteristiques de fichiers dont le nom est passe
 * en parametre.
 *
 * Utilisation de la primitive stat(2) et de la fonction getpwuid(3).
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>

/* Petite fonction qui se charge d'envoyer les messages d'erreur
   et qui ensuite "suicide" le processus. */

void erreur_grave(char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

/* Fonction principale (fournie avec erreur(s?)) */

int main(int argc, char **argv) {
  //struct stat buffer;
  struct stat *buffer = malloc(sizeof(struct stat));
  struct stat *buffer_tmp = malloc(sizeof(struct stat));
  int r;
  r = stat(argv[1], buffer);
  if (r < 0)
    erreur_grave("Stat");

  struct passwd *pw = getpwuid(buffer->st_uid);
  struct passwd *pw2 = getpwuid(1000);

  printf("Fichier %s:  mode: %X  Taille: %ld  Proprietaire: %ld\n",
	argv[1], buffer->st_mode, buffer->st_size, buffer->st_uid); // question 1

  printf("Fichier %s:  mode: %X  Taille: %ld  Proprietaire: %s\n",
	argv[1], buffer->st_mode, buffer->st_size, pw->pw_name); // question 2

  printf("Fichier %s:  mode: %X  Taille: %ld  Proprietaire: %s\n",
	argv[1], buffer->st_mode, buffer->st_size, pw2->pw_name); // question 3 SEGMENTATION FAULT

  exit(EXIT_SUCCESS);
}

//Question 4
//The getpwuid() function returns a pointer to a structure containing the broken-out 
//fields of the record in the password database that matches the user ID uid.
//getpwuid_r() functions obtain the same information as getpwnam() and getpwuid(), 
//but store the retrieved passwd structure in the space pointed to by pwd. 
