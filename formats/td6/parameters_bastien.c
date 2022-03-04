#include <complex.h>
#include <fftw3.h>
#include <math.h>
#include <sndfile.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "gnuplot_i.h"

#define PLOT true

#define FRAME_SIZE 1024
#define HOP_SIZE 1024
#define ENERGY_THRESHOLD .015

// static fftw_plan fft_plan;
static gnuplot_ctrl* plot;

static void
usage(const char* const progname)
{
    fprintf(stderr, "Usage: %s <file>.\n", progname);
    exit(EXIT_FAILURE);
}

static bool
read_samples(double* const hop_buffer, SNDFILE* const input_file, const int hop_size, const char channels)
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
fill_frame_buffer(double* const frame_buffer, const double* const hop_buffer, const int frame_size, const int hop_size)
{
    double tmp[frame_size - hop_size];

    for (int sample = 0; sample < frame_size - hop_size; sample++)
        tmp[sample] = frame_buffer[sample + hop_size];

    for (int sample = 0; sample < frame_size - hop_size; sample++)
        frame_buffer[sample] = tmp[sample];

    for (int sample = 0; sample < hop_size; sample++)
        frame_buffer[frame_size - hop_size + sample] = hop_buffer[sample];
}

static double
get_energy(const double* const frame_buffer, const int frame_size)
{
    double energy = 0.;
    for (int sample = 0; sample < frame_size; sample++)
        energy += pow(frame_buffer[sample], 2) / frame_size;
    return energy;
}

static bool
frame_is_useful(const double frame_energy)
{
    return frame_energy > ENERGY_THRESHOLD;
}

static void
hann(double* const frame_buffer, const int frame_size)
{
    for (int sample = 0; sample < frame_size; sample++)
        frame_buffer[sample] *= .5 - .5 * cos(2. * M_PI * sample / frame_size);
}

// static void
// fft_init(fftw_complex* const fft_in, fftw_complex* const fft_out, const int fft_size)
// {
//     fft_plan = fftw_plan_dft_1d(fft_size, fft_in, fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
// }

// static void
// fft(fftw_complex* const fft_in, const double* const signal, const int fft_size, const int frame_size)
// {
//     for (int sample = 0; sample < fft_size; sample++)
//         fft_in[sample] = sample < frame_size ? signal[sample] : 0.;

//     fftw_execute(fft_plan);
// }

// static void
// cartesian_to_polar(double* const amplitudes, double* const phases, const double complex* const fft_out, const int fft_size)
// {
//     for (int sample = 0; sample < fft_size; sample++) {
//         amplitudes[sample] = cabs(fft_out[sample]);
//         phases[sample] = carg(fft_out[sample]);
//     }
// }

// static void
// fft_exit()
// {
//     fftw_destroy_plan(fft_plan);
// }

// static int
// get_max_amplitude_sample(const double* const amplitudes, const int fft_size)
// {
//     int max_amplitude_sample = 0;
//     for (int sample = 1; sample < fft_size / 2; sample++)
//         if (amplitudes[max_amplitude_sample] < amplitudes[sample])
//             max_amplitude_sample = sample;
//     return max_amplitude_sample;
// }

// static int
// get_frequency(const double* const amplitudes, const int sample, const double sample_rate, const int fft_size, const bool interpolate)
// {
//     if (!interpolate)
//         return sample * sample_rate / fft_size;

//     double left, current, right, delta;
//     left = 20 * log(amplitudes[sample - 1]);
//     current = 20 * log(amplitudes[sample]);
//     right = 20 * log(amplitudes[sample + 1]);
//     delta = .5 * (left - right) / (left - 2 * current + right);
//     return (sample + delta) * sample_rate / fft_size;
// }

static int
get_pitch(const double frequency)
{
    const double H0 = 57., F0 = 440.;
    return ((int)round(H0 + 12 * log2(frequency / F0))) % 12;
}

int main(const int argc, const char* const* const argv)
{
    if (argc != 2)
        usage(argv[0]);

    SNDFILE* input_file = NULL;
    SF_INFO input_info;
    if ((input_file = sf_open(argv[1], SFM_READ, &input_info)) == NULL) {
        fprintf(stderr, "Not able to open input file %s.\n", argv[1]);
        puts(sf_strerror(NULL));
        exit(EXIT_FAILURE);
    }

    const double sample_rate = input_info.samplerate;
    const int channels = input_info.channels;
    const int size = input_info.frames;

    double hop_buffer[HOP_SIZE];
    double frame_buffer[FRAME_SIZE];

    for (int sample = 0; sample < FRAME_SIZE / HOP_SIZE - 1; sample++) {
        if (read_samples(hop_buffer, input_file, HOP_SIZE, channels))
            fill_frame_buffer(frame_buffer, hop_buffer, FRAME_SIZE, HOP_SIZE);
        else {
            fprintf(stderr, "Not enough samples.\n");
            exit(EXIT_FAILURE);
        }
    }

    plot = gnuplot_init();
    gnuplot_setstyle(plot, "lines");

    double energies[size / FRAME_SIZE];

    // int fft_size = FRAME_SIZE;
    // fftw_complex fft_in[fft_size], fft_out[fft_size];
    // double amplitudes[fft_size], phases[fft_size];
    // fft_init(fft_in, fft_out, fft_size);

    int frame_id = 0;
    while (read_samples(hop_buffer, input_file, HOP_SIZE, channels)) {
        fill_frame_buffer(frame_buffer, hop_buffer, FRAME_SIZE, HOP_SIZE);

        energies[frame_id] = get_energy(frame_buffer, FRAME_SIZE);
        if (!frame_is_useful(energies[frame_id])) {
            frame_id++;
            continue;
        }

        hann(frame_buffer, FRAME_SIZE);

        // fft(fft_in, frame_buffer, fft_size, FRAME_SIZE);
        // cartesian_to_polar(amplitudes, phases, fft_out, fft_size);

        int max_sample = 0;
        double r[FRAME_SIZE];
        for (int sample = 0; sample < FRAME_SIZE; sample++) {
            double peak = 0.;
            for (int i = 0; i < FRAME_SIZE - sample; i++)
                peak += frame_buffer[i] * frame_buffer[i + sample];
            r[sample] = peak / FRAME_SIZE;
            if (r[sample] > r[max_sample])
                max_sample = sample;
        }

        const double frequency = sample_rate / max_sample;
        const int pitch = get_pitch(frequency);
        printf("Pitch: %d.\n", pitch);

        if (PLOT) {
            gnuplot_resetplot(plot);
            gnuplot_plot_x(plot, r, FRAME_SIZE, "…");
            sleep(1);
        }

        frame_id++;
    }

    // if (PLOT) {
    //     gnuplot_resetplot(plot);
    //     gnuplot_plot_x(plot, energies, size / FRAME_SIZE, "Energy according to frame");
    //     sleep(10);
    // }

    // fft_exit();
    sf_close(input_file);
    return EXIT_SUCCESS;
}
