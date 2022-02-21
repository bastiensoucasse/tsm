#include <complex.h>
#include <fftw3.h>
#include <float.h>
#include <math.h>
#include <sndfile.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define FRAME_SIZE 2205
#define HOP_SIZE 2205

#define EVENT_FREQUENCIES 18000

static const char* event_types[3] = { "Event A", "Event B", "Event C" };
static const int event_frequencies[3] = { 19122, 19581, 20034 };

static fftw_plan fft_plan;

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

static void
fft_init(fftw_complex* const fft_in, fftw_complex* const fft_out, const int fft_size)
{
    fft_plan = fftw_plan_dft_1d(fft_size, fft_in, fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
}

static void
fft(fftw_complex* const fft_in, const double* const signal, const int fft_size, const int frame_size)
{
    for (int sample = 0; sample < fft_size; sample++)
        fft_in[sample] = sample < frame_size ? signal[sample] : 0.;

    fftw_execute(fft_plan);
}

static void
cartesian_to_polar(double* const amplitudes, double* const phases, const double complex* const fft_out, const int fft_size)
{
    for (int sample = 0; sample < fft_size; sample++) {
        amplitudes[sample] = cabs(fft_out[sample]);
        phases[sample] = carg(fft_out[sample]);
    }
}

static int
get_event_type(const double event_frequency)
{
    double delta = fabs(event_frequencies[0] - event_frequency);
    int type = 0;
    for (int i = 1; i < 3; i++)
        if (delta > fabs(event_frequencies[i] - event_frequency)) {
            delta = fabs(event_frequencies[i] - event_frequency);
            type = i;
        }
    return type;
}

static int
is_frame_event(const double* const amplitudes, const int sample_rate, const int fft_size)
{
    const double threshold = 3.;

    for (int sample = 1; sample < fft_size / 2 - 1; sample++)
        if (amplitudes[sample] >= amplitudes[sample - 1] && amplitudes[sample] > amplitudes[sample + 1] && amplitudes[sample] > threshold) {
            const double left = 20 * log(amplitudes[sample - 1]);
            const double current = 20 * log(amplitudes[sample]);
            const double right = 20 * log(amplitudes[sample + 1]);
            const double delta = .5 * (left - right) / (left - 2 * current + right);
            const double frequency = (sample + delta) * sample_rate / fft_size;
            if (frequency > EVENT_FREQUENCIES)
                return get_event_type(frequency);
        }

    return -1;
}

static void
fft_exit()
{
    fftw_destroy_plan(fft_plan);
}

static void
handle_events(const char* const input_file_name, const int frame_size, const int hop_size)
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
        if (read_samples(hop_buffer, input_file, hop_size, channels))
            fill_frame_buffer(frame_buffer, hop_buffer, frame_size, hop_size);
        else {
            fprintf(stderr, "Not enough samples.\n");
            exit(EXIT_FAILURE);
        }
    }

    int fft_size = frame_size;
    fftw_complex fft_in[fft_size], fft_out[fft_size];
    double amplitudes[fft_size], phases[fft_size];
    fft_init(fft_in, fft_out, fft_size);

    int frame_id = 0;
    while (read_samples(hop_buffer, input_file, hop_size, channels)) {
        fill_frame_buffer(frame_buffer, hop_buffer, frame_size, hop_size);

        if (!frame_is_useful(frame_buffer, frame_size)) {
            frame_id++;
            continue;
        }

        fft(fft_in, frame_buffer, fft_size, frame_size);
        cartesian_to_polar(amplitudes, phases, fft_out, fft_size);

        int event_type = is_frame_event(amplitudes, sample_rate, fft_size);
        if (event_type >= 0) {
            const double time = frame_id * frame_size / sample_rate;
            const double time_precision = frame_size / (2 * sample_rate);
            printf("  - %.2lf s (± %.3lf s): %s.\n", time, time_precision, event_types[event_type]);
        }

        frame_id++;
    }

    fft_exit();
    sf_close(input_file);
}

int main(const int argc, const char* const* const argv)
{
    printf("Events in sounds/flux1.wav:\n");
    handle_events("sounds/flux1.wav", FRAME_SIZE, HOP_SIZE);

    printf("\nEvents in sounds/flux2.wav:\n");
    handle_events("sounds/flux2.wav", FRAME_SIZE, HOP_SIZE);

    printf("\nEvents in sounds/flux3.wav:\n");
    handle_events("sounds/flux3.wav", FRAME_SIZE, HOP_SIZE);

    printf("\nEvents in sounds/flux4.wav:\n");
    handle_events("sounds/flux4.wav", FRAME_SIZE, HOP_SIZE);

    return EXIT_SUCCESS;
}
