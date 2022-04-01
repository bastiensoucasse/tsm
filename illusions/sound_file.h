#ifndef SOUND_FILE_H
#define SOUND_FILE_H

#include <stdio.h>

/**
 * @brief Opens a sound file for writing.
 *
 * @param raw_filename The name of the file to open.
 * @return The file object.
 */
FILE* sound_file_open_write(const char* const raw_filename);

/**
 * @brief Writes a sound frame into a file.
 *
 * @param raw_file The file object.
 * @param frame_buffer The frame to write.
 * @param frame_size The size of the frame to write.
 */
void sound_file_write(FILE* const raw_file, const double* const frame_buffer, const int frame_size);

/**
 * @brief Closes a sound file after writing, and renders it.
 *
 * @param raw_file The file object.
 * @param raw_filename The name of the file before rendering.
 * @param out_filename The name of the file after rendering.
 * @param num_channels The number of channels for the rendering.
 * @param sample_rate The sample rate for the rendering.
 */
void sound_file_close_write(FILE* const raw_file, const char* const raw_filename, const char* const out_filename, const int num_channels, const int sample_rate);

#endif // SOUND_FILE_H
