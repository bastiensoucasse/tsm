#include <complex.h>
#include <fftw3.h>
#include <math.h>
#include <sndfile.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "gnuplot_i.h"

#define VERBOSE true
#define PLOT false

#define FRAME_SIZE 1024
#define HOP_SIZE FRAME_SIZE

static gnuplot_ctrl* gnuplot;
static fftw_plan fft_plan;

static void
usage(const char* const program_name)
{
    fprintf(stderr, "Usage: %s ‹input_file›.\n", program_name);
}

static bool
read_samples(double* const hop_buffer, SNDFILE* const input_file, const int channels)
{
    if (channels == 2) {
        double tmp[2 * HOP_SIZE];
        const unsigned int read_count = sf_readf_double(input_file, tmp, HOP_SIZE);
        for (unsigned int sample = 0; sample < read_count; sample++)
            hop_buffer[sample] = (tmp[sample * 2] + tmp[sample * 2 + 1]) / 2.;
        return read_count == HOP_SIZE;
    }

    if (channels == 1) {
        const unsigned int read_count = sf_readf_double(input_file, hop_buffer, HOP_SIZE);
        return read_count == HOP_SIZE;
    }

    fprintf(stderr, "Channel format error.\n");
    return false;
}

static void
fill_frame_buffer(double* const frame_buffer, const double* const hop_buffer)
{
    double tmp[FRAME_SIZE - HOP_SIZE];

    for (unsigned int sample = 0; sample < FRAME_SIZE - HOP_SIZE; sample++)
        tmp[sample] = frame_buffer[sample + HOP_SIZE];

    for (unsigned sample = 0; sample < FRAME_SIZE - HOP_SIZE; sample++)
        frame_buffer[sample] = tmp[sample];

    for (unsigned sample = 0; sample < HOP_SIZE; sample++)
        frame_buffer[FRAME_SIZE - HOP_SIZE + sample] = hop_buffer[sample];
}

static void
hann(double* const signal)
{
    for (unsigned int sample = 0; sample < FRAME_SIZE; sample++)
        signal[sample] = .5 - .5 * cos(2 * M_PI * signal[sample] / FRAME_SIZE);
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
cartesian_to_polar(double* const amplitudes, double* const phases, const double complex* const complex_signal)
{
    for (unsigned int sample = 0; sample < FRAME_SIZE; sample++)
        amplitudes[sample] = cabs(complex_signal[sample]), phases[sample] = carg(complex_signal[sample]);
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

    const unsigned int sample_rate = input_info.samplerate;
    const unsigned char channels = input_info.channels;
    const unsigned int frames = input_info.frames;

    printf("Sample rate: %d.\n", sample_rate);
    printf("Number of channels: %d.\n", channels);
    printf("Number of frames: %d.\n", frames);

    int frame_id = 0;
    double hop_buffer[HOP_SIZE];
    double frame_buffer[FRAME_SIZE];

    gnuplot = gnuplot_init();
    gnuplot_setstyle(gnuplot, "lines");

    for (unsigned int sample = 0; sample < FRAME_SIZE / HOP_SIZE - 1; sample++) {
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

    while (read_samples(hop_buffer, input_file, channels)) {
        printf("\nProcessing frame %d…\n", frame_id);
        fill_frame_buffer(frame_buffer, hop_buffer);
        hann(frame_buffer);

        fft(fft_in, frame_buffer);
        cartesian_to_polar(amplitudes, phases, fft_out);

        for (unsigned int sample = 0; sample < FRAME_SIZE; sample++)
            amplitudes[sample] *= 2. / FRAME_SIZE;

        double maximum_amplitude = amplitudes[0];
        int maximum_amplitude_sample = 0;
        for (int sample = 1; sample < FRAME_SIZE / 2; sample++) {
            if (maximum_amplitude < amplitudes[sample]) {
                maximum_amplitude = amplitudes[sample];
                maximum_amplitude_sample = sample;
            }
        }

        if (maximum_amplitude == 0) {
            printf("Null frame, skipping…\n");
            frame_id++;
            continue;
        }

        if (VERBOSE)
            printf("Max amplitude: %.2lf.\n", maximum_amplitude);

        const double frequency_precision = sample_rate / FRAME_SIZE / 2.;

        const double maximum_amplitude_frequency = maximum_amplitude_sample * sample_rate / FRAME_SIZE;
        printf("Max amplitude frequency: %.2lf (± %.2lf) Hz.\n", maximum_amplitude_frequency, frequency_precision);

        const double left = 20 * log(amplitudes[maximum_amplitude_sample - 1]);
        const double current = 20 * log(amplitudes[maximum_amplitude_sample]);
        const double right = 20 * log(amplitudes[maximum_amplitude_sample + 1]);
        const double delta = .5 * (left - right) / (left - 2 * current + right);
        const double peak_frequency = (maximum_amplitude_sample + delta) * sample_rate / FRAME_SIZE;
        printf("Estimated peak frequency: %.2lf Hz.\n", peak_frequency);

        if (PLOT) {
            gnuplot_resetplot(gnuplot);
            gnuplot_plot_x(gnuplot, amplitudes, FRAME_SIZE / 2, "Temporal Frame");
            sleep(1);
        }

        frame_id++;
    }

    fft_exit();
    sf_close(input_file);
    return EXIT_SUCCESS;
}
