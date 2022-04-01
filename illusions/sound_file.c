#include "sound_file.h"

#include <stdlib.h>

FILE* sound_file_open_write(const char* const raw_filename)
{
    return fopen(raw_filename, "wb");
}

void sound_file_write(FILE* const raw_file, const double* const frame_buffer, const int frame_size)
{
    short tmp[frame_size];
    for (int i = 0; i < frame_size; i++)
        tmp[i] = (short)(frame_buffer[i] * 32765);
    fwrite(tmp, sizeof(short), frame_size, raw_file);
}

void sound_file_close_write(FILE* const raw_file, const char* const raw_filename, const char* const out_filename, const int num_channels, const int sample_rate)
{
    fclose(raw_file);

    char cmd[256];
    snprintf(cmd, 256, "sox -c %d -r %d -e signed-integer -b 16 %s %s", num_channels, sample_rate, raw_filename, out_filename);
    system(cmd);
}
