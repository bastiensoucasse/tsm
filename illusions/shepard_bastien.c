#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sinusoid.h"
#include "sound_file.h"

#define DURATION .5
#define SAMPLE_RATE 44100
#define FRAME_SIZE (int)(DURATION * SAMPLE_RATE)

static const char* const RAW_FILENAME = "outputs/raw_bastien.raw";
static const char* const OUT_FILENAME = "outputs/out_bastien.wav";

static void
silence(FILE* const output, double* const s)
{
    for (int i = 0; i < FRAME_SIZE; i++)
        s[i] = 0;

    sound_file_write(output, s, FRAME_SIZE);
}

static void
note_shepard(FILE* const output, double* const s, const double f)
{
    double a = f / 261.626 - 1;
    sinusoid x = create_sinusoid(1 - a, f);
    sinusoid y = create_sinusoid(a, f / 2);

    for (int i = 0; i < FRAME_SIZE; i++) {
        s[i] = get_sinusoid_value(x, 0, FRAME_SIZE, i, SAMPLE_RATE);
        s[i] += get_sinusoid_value(y, 0, FRAME_SIZE, i, SAMPLE_RATE);
    }

    sound_file_write(output, s, FRAME_SIZE);
}

static void
gamme_shepard_12_up(FILE* const fo, double* const s)
{
    note_shepard(fo, s, 261.626); // C4
    silence(fo, s);

    note_shepard(fo, s, 277.183); // C#4
    silence(fo, s);

    note_shepard(fo, s, 293.665); // D4
    silence(fo, s);

    note_shepard(fo, s, 311.127); // D#4
    silence(fo, s);

    note_shepard(fo, s, 329.628); // E4
    silence(fo, s);

    note_shepard(fo, s, 349.228); // F4
    silence(fo, s);

    note_shepard(fo, s, 369.994); // F#4
    silence(fo, s);

    note_shepard(fo, s, 391.995); // G4
    silence(fo, s);

    note_shepard(fo, s, 415.305); // G#4
    silence(fo, s);

    note_shepard(fo, s, 440); // A4
    silence(fo, s);

    note_shepard(fo, s, 466.164); // A#4
    silence(fo, s);

    note_shepard(fo, s, 493.883); // B4
    silence(fo, s);
}

int main(int argc, char** argv)
{
    if (argc != 1) {
        printf("Usage: %s\n.", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE* const sound_file = sound_file_open_write(RAW_FILENAME);

    double s[FRAME_SIZE];
    for (int i = 0; i < 4; i++)
        gamme_shepard_12_up(sound_file, s);

    sound_file_close_write(sound_file, RAW_FILENAME, OUT_FILENAME, 1, SAMPLE_RATE);
    return EXIT_SUCCESS;
}
