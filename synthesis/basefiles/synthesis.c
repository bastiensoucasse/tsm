#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define FE 44100
#define DUREE 5
#define N DUREE*FE

/* output */
static char *RAW_OUT = "tmp-out.raw";
static char *FILE_OUT = "out.wav";

FILE *
sound_file_open_write (void)
{
 return fopen (RAW_OUT, "wb");
}

void
sound_file_close_write (FILE *fp)
{
  char cmd[256];
  fclose (fp);
  snprintf(cmd, 256, "sox -c 1 -r %d -e signed-integer -b 16 %s %s", (int)FE, RAW_OUT, FILE_OUT);
  system (cmd);
}

void
sound_file_write (double *s, FILE *fp)
{
  int i;
  short tmp[N];
  for (i=0; i<N; i++)
    {
      tmp[i] = (short)(s[i]*32768);
    }
  fwrite (tmp, sizeof(short), N, fp);
}

int
main (int argc, char *argv[])
{
  int i;
  FILE *output;
  double s[N];

  if (argc != 1)
    {
      printf ("usage: %s\n", argv[0]);
      exit (EXIT_FAILURE);
    }

  output = sound_file_open_write ();
      
  for (i=0; i<N; i++)
    s[i] = 0; // ECHANTILLONS
      
  sound_file_write (s, output);
  sound_file_close_write (output);
  exit (EXIT_SUCCESS);
}