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

int main(int argc, char** argv)
{
    if (argc != 1) {
        printf("Usage: %s\n.", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE* const sound_file = sound_file_open_write(RAW_FILENAME);

    sinusoid c = create_sinusoid(.4, 261.63);
    sinusoid g = create_sinusoid(.3, 391.995);
    sinusoid e = create_sinusoid(.3, 329.628);

    int frame = 0;
    double frame_buffer[FRAME_SIZE];
    while (frame * FRAME_SIZE < DURATION * SAMPLE_RATE) {
        for (int sample = 0; sample < FRAME_SIZE; sample++) {
            frame_buffer[sample] = get_sinusoid_value(c, frame, FRAME_SIZE, sample, SAMPLE_RATE);
            frame_buffer[sample] += get_sinusoid_value(g, frame, FRAME_SIZE, sample, SAMPLE_RATE);
            frame_buffer[sample] += get_sinusoid_value(e, frame, FRAME_SIZE, sample, SAMPLE_RATE);
        }

        sound_file_write(sound_file, frame_buffer, FRAME_SIZE);
        frame++;
    }

    sound_file_close_write(sound_file, RAW_FILENAME, OUT_FILENAME, CHANNELS, SAMPLE_RATE);
    return EXIT_SUCCESS;
}
