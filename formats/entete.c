#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>	// pour gestion des fichiers
#include <string.h>	// pour gestion chaînes de caractères
#include "define.h"

void
lect_entete_fich_wav(TEXT *nom_fich)
{
  BYTE *entete;
  int i, df;
  ULONG *freq_ech, *taille;
  UWORD *n_bits, *n_ech;
  
  if((df=open(nom_fich, O_RDONLY, 0)) == -1)
    {
      perror(nom_fich);
      exit(-1);
    }

  entete=(BYTE*)malloc(sizeof(BYTE)*44);
  read(df, entete, 44);
  
  printf("\nFichier : %s\n", nom_fich);
  
  /* A COMPLETER */

  close(df);
  free(entete);
}

int
main(int argc, char *argv[])
{
  TEXT nom_fich1[200];

  if(argc<1!=0)
    {
      printf("Syntaxe : ./e fichier_a_traiter.wav\n");
      exit(0);
    }
  
  strcpy(nom_fich1, argv[1]);
  lect_entete_fich_wav(nom_fich1);
}

