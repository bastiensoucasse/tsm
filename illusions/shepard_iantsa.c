#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sinusoid.h"
#include "sound_file.h"

#define DUREE 0.5
#define FE 44100
#define N (int) (DUREE * FE)

static const char* const RAW_FILENAME = "outputs/raw_iantsa.raw";
static const char* const OUT_FILENAME = "outputs/out_iantsa.wav";

static void
silence(double* s, FILE* output)
{
    for (int i = 0; i < N; i++)
        s[i] = 0;

    sound_file_write(output, s, N);
}

static void
note_shepard(double* s, double f, FILE* output)
{
    // version 1
    // double a;
    // for (int i = 0; i < N; i++)
    // {
    //     a = f / 261.626 - 1;
    //     s[i] = (1-a) * sin(2 * M_PI * f * (i * 1. / FE));
    //     s[i] += a * sin(2 * M_PI * (f/2) * (i * 1. / FE)); // lower octave
    // }

    // version 2
    for (int i = 0; i < N; i++)
    {
        s[i] = 0;
        for (int a = 0; a < 10; a++)
            s[i] += sin(2 * M_PI) * pow(2, a) * f;
    }

    sound_file_write(output, s, N);
}

static void
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

int main(int argc, char** argv)
{
    if (argc != 1) {
        printf("Usage: %s\n.", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE* const sound_file = sound_file_open_write(RAW_FILENAME);

    double s[N];
    for (int i = 0; i < 4; i++)
        gamme_shepard_12_up(s, sound_file);

    sound_file_close_write(sound_file, RAW_FILENAME, OUT_FILENAME, 1, FE);
    return EXIT_SUCCESS;
}
