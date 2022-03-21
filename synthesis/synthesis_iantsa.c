#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// #include "sinusoid.h"
#include "sound_file.h"

#define FE 44100
#define DUREE 5 // 60
#define N DUREE* FE // 1024
// #define DUREE_TRAME N / FE

static const char* const RAW_FILENAME = "outputs/raw_iantsa.raw";
static const char* const OUT_FILENAME = "outputs/out_iantsa.wav";

int main(int argc, char** argv)
{
    if (argc != 1) {
        printf("Usage: %s\n.", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE* const sound_file = sound_file_open_write(RAW_FILENAME);
    double s[N];

    double amp1 = 0.6;
    // double amp2 = 0.4;
    double f1 = 440;
    double f2 = 700;
    // double phi = 0;
    // double d_phi1, d_phi2, d_phi3;
    // double c = 1;
    double I = 5;

    // int nb_frame = 0;
    // while (nb_frame * DUREE_TRAME * 1. < DUREE) {
    for (int i = 0; i < N; i++) {
        // s[i] = amp * sin(2 * M_PI * f1 * (i*1./FE) + phi);
        // s[i+1] = amp * sin(2 * M_PI * f2 * (i*1./FE) + phi);
        // if (i % 1000 == 0) f1 += 10, f2 += 10;

        // s[i] = 0.4 * sin(2 * M_PI * 261.63 * (i*1./FE) + phi);
        // s[i] += 0.3 * sin(2 * M_PI * 329.628 * (i*1./FE) + phi);
        // s[i] += 0.3 * sin(2 * M_PI * 391.995 * (i*1./FE) + phi);

        // d_phi1 = 2 * M_PI * 261.63 * DUREE_TRAME, d_phi2 = 2 * M_PI * 329.628 * DUREE_TRAME, d_phi3 = 2 * M_PI * 391.995 * DUREE_TRAME;
        // s[i] = 0.4 * sin(2 * M_PI * 261.63 * (i * 1. / FE) + nb_frame * d_phi1);
        // s[i] += 0.3 * sin(2 * M_PI * 329.628 * (i * 1. / FE) + nb_frame * d_phi2);
        // s[i] += 0.3 * sin(2 * M_PI * 391.995 * (i * 1. / FE) + nb_frame * d_phi3);

        // ---------

        // Additive
        // s[i] = amp1 * sin(2 * M_PI * f1 * (i * 1. / FE)) + c;
        // s[i] *= amp2 * sin(2 * M_PI * f2 * (i * 1. / FE));

        // AM
        // s[i] = amp1 * sin(2 * M_PI * f1 * (i * 1. / FE));
        // s[i] *= c + amp2 * sin(2 * M_PI * f2 * (i * 1. / FE));

        // FM
        // s[i] = amp1 * sin(2 * M_PI * (c + amp2 * sin(2 * M_PI * f2 * (i * 1. / FE)) * (i * 1. / FE)));
        s[i] = amp1 * sin(2 * M_PI * f1 * (i * 1. / FE));
        s[i] += I * sin(2 * M_PI * f2 * (i * 1. / FE));
    }

    sound_file_write(sound_file, s, N);
    // nb_frame++;
    //}

    sound_file_close_write(sound_file, RAW_FILENAME, OUT_FILENAME, 1, FE);
    return EXIT_SUCCESS;
}
