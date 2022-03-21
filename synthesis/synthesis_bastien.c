#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "sinusoid.h"
#include "sound_file.h"

#define DURATION 60
#define CHANNELS 1
#define SAMPLE_RATE 44100
#define FRAME_SIZE 1024

static const char* const RAW_FILENAME = "outputs/raw_bastien.raw";
static const char* const OUT_FILENAME = "outputs/out_bastien.wav";

// static double
// additive_value(const int frame, const int sample)
// {
//     sinusoid a = create_sinusoid(.5, 440.);
//     double a_value = get_sinusoid_value(a, frame, FRAME_SIZE, sample, SAMPLE_RATE);

//     sinusoid b = create_sinusoid(.5, 700.);
//     double b_value = get_sinusoid_value(b, frame, FRAME_SIZE, sample, SAMPLE_RATE);

//     return a_value + b_value;
// }

// static double
// am_value(const int frame, const int sample)
// {
//     const double constant = .5;

//     sinusoid b = create_sinusoid(.5, 10.);
//     double b_value = get_sinusoid_value(b, frame, FRAME_SIZE, sample, SAMPLE_RATE);

//     sinusoid a = create_sinusoid(constant + b_value, 440.);
//     double a_value = get_sinusoid_value(a, frame, FRAME_SIZE, sample, SAMPLE_RATE);

//     return a_value;
// }

static double
fm_value(const int frame, const int sample)
{
    const double constant = 0;

    // sinusoid b = create_sinusoid(M_PI, 220.);
    sinusoid b = create_sinusoid(M_PI, 123.);
    double b_value = get_sinusoid_value(b, frame, FRAME_SIZE, sample, SAMPLE_RATE);

    sinusoid a = create_sinusoid(.5, constant + b_value);
    double a_value = get_sinusoid_value(a, frame, FRAME_SIZE, sample, SAMPLE_RATE);

    return a_value;
}

int main(int argc, char** argv)
{
    if (argc != 1) {
        printf("Usage: %s\n.", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE* const sound_file = sound_file_open_write(RAW_FILENAME);

    int frame = 0;
    double frame_buffer[FRAME_SIZE];
    while (frame * FRAME_SIZE < DURATION * SAMPLE_RATE) {
        for (int sample = 0; sample < FRAME_SIZE; sample++) {
            // frame_buffer[sample] = additive_value(frame, sample);
            // frame_buffer[sample] = am_value(frame, sample);
            frame_buffer[sample] = fm_value(frame, sample);
        }

        sound_file_write(sound_file, frame_buffer, FRAME_SIZE);
        frame++;
    }

    sound_file_close_write(sound_file, RAW_FILENAME, OUT_FILENAME, CHANNELS, SAMPLE_RATE);
    return EXIT_SUCCESS;
}
