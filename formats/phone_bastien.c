#include <complex.h>
#include <fftw3.h>
#include <float.h>
#include <math.h>
#include <sndfile.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define FRAME_SIZE 2646 // 8820 for calibration.
#define HOP_SIZE 2646 // 4410 for calibration.

static const char keys[4][4] = {
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' }
};

static const int line_frequencies[4] = { 697, 770, 852, 941 };
static const int column_frequencies[4] = { 1209, 1336, 1477, 1633 };

static fftw_plan fft_plan;

static void
usage(const char* const program_name)
{
    fprintf(stderr, "Usage: %s ‹input_file›.\n", program_name);
}

static bool
read_samples(double* const hop_buffer, SNDFILE* const input_file, const char channels)
{
    if (channels == 2) {
        double tmp[2 * HOP_SIZE];
        const int read_count = sf_readf_double(input_file, tmp, HOP_SIZE);
        for (int sample = 0; sample < read_count; sample++)
            hop_buffer[sample] = (tmp[sample * 2] + tmp[sample * 2 + 1]) / 2.;
        return read_count == HOP_SIZE;
    }

    if (channels == 1) {
        const int read_count = sf_readf_double(input_file, hop_buffer, HOP_SIZE);
        return read_count == HOP_SIZE;
    }

    fprintf(stderr, "Channel format error.\n");
    return false;
}

static void
fill_frame_buffer(double* const frame_buffer, const double* const hop_buffer)
{
    double tmp[FRAME_SIZE - HOP_SIZE];

    for (int sample = 0; sample < FRAME_SIZE - HOP_SIZE; sample++)
        tmp[sample] = frame_buffer[sample + HOP_SIZE];

    for (int sample = 0; sample < FRAME_SIZE - HOP_SIZE; sample++)
        frame_buffer[sample] = tmp[sample];

    for (int sample = 0; sample < HOP_SIZE; sample++)
        frame_buffer[FRAME_SIZE - HOP_SIZE + sample] = hop_buffer[sample];
}

static bool
frame_is_useful(const double* const frame_buffer)
{
    const double threshold = .00001;
    double energy = 0.;

    for (int sample = 0; sample < FRAME_SIZE; sample++)
        energy += pow(frame_buffer[sample], 2);
    energy *= 1. / FRAME_SIZE;

    return energy > threshold;
}

static void
fft_init(fftw_complex* const fft_in, fftw_complex* const fft_out)
{
    fft_plan = fftw_plan_dft_1d(FRAME_SIZE, fft_in, fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
}

static void
fft(fftw_complex* const fft_in, const double* const signal)
{
    for (int sample = 0; sample < FRAME_SIZE; sample++)
        fft_in[sample] = signal[sample];

    fftw_execute(fft_plan);
}

static void
cartesian_to_polar(double* const amplitudes, double* const phases, const double complex* const fft_out)
{
    for (int sample = 0; sample < FRAME_SIZE; sample++) {
        amplitudes[sample] = cabs(fft_out[sample]);
        phases[sample] = carg(fft_out[sample]);
    }
}

static void
get_peak_frequencies(double* const peak_frequencies, const double* const amplitudes, const int sample_rate)
{
    int peak_samples[2] = { 1, 1 };

    for (int sample = 1; sample < FRAME_SIZE / 2 - 1; sample++)
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
        peak_frequencies[peak] = (peak_samples[peak] + delta) * sample_rate / FRAME_SIZE;
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

int main(const int argc, const char* const* const argv)
{
    if (argc != 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char* const input_file_name = argv[1];

    SNDFILE* input_file = NULL;
    SF_INFO input_info;
    if ((input_file = sf_open(input_file_name, SFM_READ, &input_info)) == NULL) {
        fprintf(stderr, "Not able to open input file %s.\n", input_file_name);
        puts(sf_strerror(NULL));
        return EXIT_FAILURE;
    }

    const double sample_rate = input_info.samplerate;
    const char channels = input_info.channels;

    double hop_buffer[HOP_SIZE];
    double frame_buffer[FRAME_SIZE];

    for (int sample = 0; sample < FRAME_SIZE / HOP_SIZE - 1; sample++) {
        if (read_samples(hop_buffer, input_file, channels))
            fill_frame_buffer(frame_buffer, hop_buffer);
        else {
            fprintf(stderr, "Not enough samples.\n");
            return EXIT_FAILURE;
        }
    }

    fftw_complex fft_in[FRAME_SIZE], fft_out[FRAME_SIZE];
    double amplitudes[FRAME_SIZE], phases[FRAME_SIZE];
    fft_init(fft_in, fft_out);

    int frame_id = 0;
    int number_capacity = 10, number_size = 0;
    char* number = (char*)malloc(number_capacity * sizeof(char));
    bool next = true;
    while (read_samples(hop_buffer, input_file, channels)) {
        fill_frame_buffer(frame_buffer, hop_buffer);

        if (!frame_is_useful(frame_buffer)) {
            next = true;
            frame_id++;
            continue;
        }

        if (!next) {
            frame_id++;
            continue;
        }

        fft(fft_in, frame_buffer);
        cartesian_to_polar(amplitudes, phases, fft_out);

        double peak_frequencies[2];
        get_peak_frequencies(peak_frequencies, amplitudes, sample_rate);
        if (number_size == number_capacity) {
            number_capacity += 1;
            number = (char*)realloc(number, number_capacity * sizeof(char));
        }
        number[number_size] = get_key(peak_frequencies);

        next = false;
        frame_id++, number_size++;
    }

    printf("Number: ");
    for (int i = 0; i < number_size; i++)
        printf("%c", number[i]);
    printf(".\n");

    free(number);
    fft_exit();
    sf_close(input_file);
    return EXIT_SUCCESS;
}
