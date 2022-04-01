#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define FE 44100
#define DUREE 1
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
      tmp[i] = (short)(s[i]*32765);
    }
  fwrite (tmp, sizeof(short), N, fp);
}

void
silence(double* s, FILE* output)
{
  int i;

  for(i=0;i<N;i++)
    s[i] = 0;

  sound_file_write (s, output);
}

void
note_shepard(double* s, double f, FILE* output)
{
  // A COMPLETER

  sound_file_write (s, output);
}

void
gamme_shepard_12_up(double* s, FILE* fo)
{
  note_shepard(s, 261.626, fo); // C4
  silence(s, fo);
  
  note_shepard(s, 277.183, fo); // C#4
  silence(s, fo);
  
  note_shepard(s, 293.665, fo); // D4
  silence(s, fo);           
  
  note_shepard(s, 311.127, fo); // D#4
  silence(s, fo);
  
  note_shepard(s, 329.628, fo); // E4
  silence(s, fo);
  
  note_shepard(s, 349.228, fo); // F4
  silence(s, fo);           
  
  note_shepard(s, 369.994, fo); // F#4
  silence(s, fo);
  
  note_shepard(s, 391.995, fo); // G4
  silence(s, fo);           
  
  note_shepard(s, 415.305, fo); // G#4
  silence(s, fo);
  
  note_shepard(s, 440, fo); // A4
  silence(s, fo);       
  
  note_shepard(s, 466.164, fo); // A#4
  silence(s, fo);
  
  note_shepard(s, 493.883, fo); // B4
  silence(s, fo);
}

int
main (int argc, char *argv[])
{
  int i;
  double s[N];
  FILE* fo = sound_file_open_write ();
  
  for(i=0;i<4;i++)
    {
      gamme_shepard_12_up(s, fo);
    }
  
  sound_file_close_write (fo);
  exit (EXIT_SUCCESS);
}