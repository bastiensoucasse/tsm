#include <complex.h>
#include <fftw3.h>
#include <float.h>
#include <math.h>
#include <sndfile.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define FRAME_SIZE 1024
#define HOP_SIZE 1024

// static fftw_plan fft_plan;

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

// static void
// hann(double* const frame_buffer, const int frame_size)
// {
//     for (int sample = 0; sample < frame_size; sample++)
//         frame_buffer[sample] *= .5 - .5 * cos(2. * M_PI * sample / frame_size);
// }

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

static int
get_frequency(const double* const frame_buffer, const int frame_size, const double sample_rate)
{
    double max_amp_1 = 0, max_amp_2 = 0;
    double max_amp_sample_1 = 0, max_amp_sample_2 = 0;

    for (int sample = 0; sample < frame_size; sample++) {
        if (max_amp_1 < frame_buffer[sample]) {
            max_amp_2 = max_amp_1;
            max_amp_sample_2 = max_amp_sample_1;
            max_amp_1 = frame_buffer[sample];
            max_amp_sample_1 = sample;
        } else if (max_amp_2 < frame_buffer[sample]) {
            max_amp_2 = frame_buffer[sample];
            max_amp_sample_2 = sample;
        }
    }

    return max_amp_sample_2;
}

static int
get_pitch(const double frequency)
{
    const int H0 = 57, F0 = 440;
    return ((int)round(H0 + 12 * log2(frequency / F0))) % 12;
}

int main(const int argc, const char* const* const argv)
{
    SNDFILE* input_file = NULL;
    SF_INFO input_info;
    if ((input_file = sf_open(argv[1], SFM_READ, &input_info)) == NULL) {
        fprintf(stderr, "Not able to open input file %s.\n", argv[1]);
        puts(sf_strerror(NULL));
        exit(EXIT_FAILURE);
    }

    const double sample_rate = input_info.samplerate;
    const int channels = input_info.channels;

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

    // int fft_size = FRAME_SIZE;
    // fftw_complex fft_in[fft_size], fft_out[fft_size];
    // double amplitudes[fft_size], phases[fft_size];
    // fft_init(fft_in, fft_out, fft_size);

    int frame_id = 0;
    while (read_samples(hop_buffer, input_file, HOP_SIZE, channels)) {
        fill_frame_buffer(frame_buffer, hop_buffer, FRAME_SIZE, HOP_SIZE);

        if (!frame_is_useful(frame_buffer, FRAME_SIZE)) {
            frame_id++;
            continue;
        }

        // hann(frame_buffer, FRAME_SIZE);
        // fft(fft_in, frame_buffer, fft_size, FRAME_SIZE);
        // cartesian_to_polar(amplitudes, phases, fft_out, fft_size);

        // double max_amp = 0;
        // double max_amp_freq = 0;
        // for (int sample = 0; sample < fft_size / 2; sample++)
        //     if (max_amp < amplitudes[sample]) {
        //         max_amp = amplitudes[sample];
        //         double left = 20 * log(amplitudes[sample - 1]);
        //         double current = 20 * log(amplitudes[sample]);
        //         double right = 20 * log(amplitudes[sample + 1]);
        //         double delta = .5 * (left - right) / (left - 2 * current + right);
        //         max_amp_freq = (sample + delta) * sample_rate / fft_size;
        //     }

        // printf("Max Amplitude Frequency: %.2lf.\n", max_amp_freq);

        // int pitch = get_pitch(max_amp_freq);
        // printf("Pitch: %d.\n", pitch);

        int frequency = get_frequency(frame_buffer, FRAME_SIZE, sample_rate);
        printf("New Max Amplitude Frequency: %d.\n", frequency);

        int pitch = get_pitch(frequency);
        printf("New Pitch: %d.\n", pitch);

        frame_id++;
    }

    // fft_exit();
    sf_close(input_file);
    return EXIT_SUCCESS;
}
