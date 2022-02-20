#include <complex.h>
#include <fftw3.h>
#include <float.h>
#include <math.h>
#include <sndfile.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define FRAME_SIZE 2646
#define HOP_SIZE 2646

static const char keys[4][4] = {
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' }
};

static const int line_frequencies[4] = { 697, 770, 852, 941 };
static const int column_frequencies[4] = { 1209, 1336, 1477, 1633 };

static fftw_plan fft_plan;

static bool
read_samples(double* const hop_buffer, const int hop_size, SNDFILE* const input_file, const char channels)
{
    if (channels == 2) {
        double tmp[2 * hop_size];
        const int read_count = sf_readf_double(input_file, tmp, hop_size);
        for (int sample = 0; sample < read_count; sample++)
            hop_buffer[sample] = (tmp[sample * 2] + tmp[sample * 2 + 1]) / 2.;
        return read_count == hop_size;
    }

    if (channels == 1) {
        const int read_count = sf_readf_double(input_file, hop_buffer, hop_size);
        return read_count == hop_size;
    }

    fprintf(stderr, "Channel format error.\n");
    return false;
}

static void
fill_frame_buffer(const double* const hop_buffer, const int hop_size, double* const frame_buffer, const int frame_size)
{
    double tmp[frame_size - hop_size];

    for (int sample = 0; sample < frame_size - hop_size; sample++)
        tmp[sample] = frame_buffer[sample + hop_size];

    for (int sample = 0; sample < frame_size - hop_size; sample++)
        frame_buffer[sample] = tmp[sample];

    for (int sample = 0; sample < hop_size; sample++)
        frame_buffer[frame_size - hop_size + sample] = hop_buffer[sample];
}

static bool
frame_is_useful(const double* const frame_buffer, const int frame_size)
{
    const double threshold = .00001;
    double energy = 0.;

    for (int sample = 0; sample < frame_size; sample++)
        energy += pow(frame_buffer[sample], 2);
    energy *= 1. / frame_size;

    return energy > threshold;
}

static void
fft_init(fftw_complex* const fft_in, fftw_complex* const fft_out, const int frame_size)
{
    fft_plan = fftw_plan_dft_1d(frame_size, fft_in, fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
}

static void
fft(fftw_complex* const fft_in, const double* const signal, const int frame_size)
{
    for (int sample = 0; sample < frame_size; sample++)
        fft_in[sample] = signal[sample];

    fftw_execute(fft_plan);
}

static void
cartesian_to_polar(double* const amplitudes, double* const phases, const double complex* const fft_out, const int frame_size)
{
    for (int sample = 0; sample < frame_size; sample++) {
        amplitudes[sample] = cabs(fft_out[sample]);
        phases[sample] = carg(fft_out[sample]);
    }
}

static void
get_peak_frequencies(double* const peak_frequencies, const double* const amplitudes, const int sample_rate, const int frame_size)
{
    int peak_samples[2] = { 1, 1 };

    for (int sample = 1; sample < frame_size / 2 - 1; sample++)
        if (amplitudes[sample] >= amplitudes[sample - 1] && amplitudes[sample] > amplitudes[sample + 1]) {
            if (amplitudes[sample] > amplitudes[peak_samples[0]]) {
                peak_samples[1] = peak_samples[0];
                peak_samples[0] = sample;
            } else if (amplitudes[sample] > amplitudes[peak_samples[1]])
                peak_samples[1] = sample;
        }

    double left, current, right, delta;
    for (int peak = 0; peak < 2; peak++) {
        left = 20 * log(amplitudes[peak_samples[peak] - 1]);
        current = 20 * log(amplitudes[peak_samples[peak]]);
        right = 20 * log(amplitudes[peak_samples[peak] + 1]);
        delta = .5 * (left - right) / (left - 2 * current + right);
        peak_frequencies[peak] = (peak_samples[peak] + delta) * sample_rate / frame_size;
    }
}

static char
get_key(const double* const key_frequencies)
{
    const double line_frequency = key_frequencies[1] > key_frequencies[0] ? key_frequencies[0] : key_frequencies[1];
    const double column_frequency = key_frequencies[1] > key_frequencies[0] ? key_frequencies[1] : key_frequencies[0];

    int line, column;
    double delta;

    delta = fabs(line_frequencies[0] - line_frequency);
    line = 0;
    for (int i = 1; i < 4; i++)
        if (delta > fabs(line_frequencies[i] - line_frequency)) {
            delta = fabs(line_frequencies[i] - line_frequency);
            line = i;
        }

    delta = fabs(column_frequencies[0] - column_frequency);
    column = 0;
    for (int i = 1; i < 4; i++)
        if (delta > fabs(column_frequencies[i] - column_frequency)) {
            delta = fabs(column_frequencies[i] - column_frequency);
            column = i;
        }

    return keys[line][column];
}

static void
fft_exit()
{
    fftw_destroy_plan(fft_plan);
}

static void
calibrate(const char* const input_file_name, const int hop_size, const int frame_size)
{
    SNDFILE* input_file = NULL;
    SF_INFO input_info;
    if ((input_file = sf_open(input_file_name, SFM_READ, &input_info)) == NULL) {
        fprintf(stderr, "Not able to open input file %s.\n", input_file_name);
        puts(sf_strerror(NULL));
        exit(EXIT_FAILURE);
    }

    const double sample_rate = input_info.samplerate;
    const int channels = input_info.channels;

    double hop_buffer[hop_size];
    double frame_buffer[frame_size];

    for (int sample = 0; sample < frame_size / hop_size - 1; sample++) {
        if (read_samples(hop_buffer, hop_size, input_file, channels))
            fill_frame_buffer(hop_buffer, hop_size, frame_buffer, frame_size);
        else {
            fprintf(stderr, "Not enough samples.\n");
            exit(EXIT_FAILURE);
        }
    }

    const int keys_pressed[12][2] = { { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1, 0 }, { 1, 1 }, { 1, 2 }, { 2, 0 }, { 2, 1 }, { 2, 2 }, { 3, 1 }, { 3, 0 }, { 3, 2 } };

    fftw_complex fft_in[frame_size], fft_out[frame_size];
    double amplitudes[frame_size], phases[frame_size];
    fft_init(fft_in, fft_out, frame_size);

    int frame_id = 0;
    while (read_samples(hop_buffer, hop_size, input_file, channels)) {
        fill_frame_buffer(hop_buffer, hop_size, frame_buffer, frame_size);

        if (frame_id % 3 != 0) {
            frame_id++;
            continue;
        }

        printf("Calibrating key %c…\n", keys[keys_pressed[frame_id / 3][0]][keys_pressed[frame_id / 3][1]]);

        fft(fft_in, frame_buffer, frame_size);
        cartesian_to_polar(amplitudes, phases, fft_out, frame_size);

        double peak_frequencies[2];
        get_peak_frequencies(peak_frequencies, amplitudes, sample_rate, frame_size);
        printf("Key Frequencies: %.2lf Hz & %.2lf Hz.\n", peak_frequencies[0], peak_frequencies[1]);

        frame_id++;
    }

    printf("\n");
    fft_exit();
    sf_close(input_file);
}

static char*
get_number(int* const number_size, const char* const input_file_name, const int hop_size, const int frame_size)
{
    SNDFILE* input_file = NULL;
    SF_INFO input_info;
    if ((input_file = sf_open(input_file_name, SFM_READ, &input_info)) == NULL) {
        fprintf(stderr, "Not able to open input file %s.\n", input_file_name);
        puts(sf_strerror(NULL));
        exit(EXIT_FAILURE);
    }

    const double sample_rate = input_info.samplerate;
    const int channels = input_info.channels;

    double hop_buffer[hop_size];
    double frame_buffer[frame_size];

    for (int sample = 0; sample < frame_size / hop_size - 1; sample++) {
        if (read_samples(hop_buffer, hop_size, input_file, channels))
            fill_frame_buffer(hop_buffer, hop_size, frame_buffer, frame_size);
        else {
            fprintf(stderr, "Not enough samples.\n");
            exit(EXIT_FAILURE);
        }
    }

    fftw_complex fft_in[frame_size], fft_out[frame_size];
    double amplitudes[frame_size], phases[frame_size];
    fft_init(fft_in, fft_out, frame_size);

    int frame_id = 0;
    int number_capacity = 10;
    *number_size = 0;
    char* number = (char*)malloc(number_capacity * sizeof(char));
    bool next = true;
    while (read_samples(hop_buffer, hop_size, input_file, channels)) {
        fill_frame_buffer(hop_buffer, hop_size, frame_buffer, frame_size);

        if (!frame_is_useful(frame_buffer, frame_size)) {
            next = true;
            frame_id++;
            continue;
        }

        if (!next) {
            frame_id++;
            continue;
        }

        fft(fft_in, frame_buffer, frame_size);
        cartesian_to_polar(amplitudes, phases, fft_out, frame_size);

        double peak_frequencies[2];
        get_peak_frequencies(peak_frequencies, amplitudes, sample_rate, frame_size);
        if (*number_size == number_capacity) {
            number_capacity += 1;
            number = (char*)realloc(number, number_capacity * sizeof(char));
        }
        number[*number_size] = get_key(peak_frequencies);

        next = false;
        frame_id++, (*number_size)++;
    }

    fft_exit();
    sf_close(input_file);
    return number;
}

static void
print_number(const char* const number, const int number_size, const char* const input_file_name)
{
    printf("Number in %s: ", input_file_name);
    for (int i = 0; i < number_size; i++)
        printf("%c", number[i]);
    printf(".\n");
}

int main(const int argc, const char* const* const argv)
{
    calibrate("sounds/telbase.wav", 4410, 8820);

    char* number;
    int number_size;

    number = get_number(&number_size, "sounds/telA.wav", HOP_SIZE, FRAME_SIZE);
    print_number(number, number_size, "sounds/telA.wav");

    number = get_number(&number_size, "sounds/telB.wav", HOP_SIZE, FRAME_SIZE);
    print_number(number, number_size, "sounds/telB.wav");

    number = get_number(&number_size, "sounds/telC.wav", HOP_SIZE, FRAME_SIZE);
    print_number(number, number_size, "sounds/telC.wav");

    number = get_number(&number_size, "sounds/telD.wav", HOP_SIZE, FRAME_SIZE);
    print_number(number, number_size, "sounds/telD.wav");

    return EXIT_SUCCESS;
}
