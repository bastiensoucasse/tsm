#include <complex.h>
#include <fftw3.h>
#include <math.h>
#include <sndfile.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "gnuplot_i.h"

#define PLOT true

#define FRAME_SIZE 2048
#define HOP_SIZE 2048
#define O 50

static fftw_plan fft_plan, ifft_plan;
static gnuplot_ctrl* plot;

static void
usage(const char* const progname)
{
    fprintf(stderr, "Usage: %s FILE.\n", progname);
    exit(EXIT_FAILURE);
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

static void
fft_init(fftw_complex* const fft_in, fftw_complex* const fft_out)
{
    fft_plan = fftw_plan_dft_1d(FRAME_SIZE, fft_in, fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
}

static void
fft_process()
{
    fftw_execute(fft_plan);
}

static void
fft_exit()
{
    fftw_destroy_plan(fft_plan);
}

static void
ifft_init(fftw_complex* const ifft_in, fftw_complex* const ifft_out)
{
    ifft_plan = fftw_plan_dft_1d(FRAME_SIZE, ifft_in, ifft_out, FFTW_BACKWARD, FFTW_ESTIMATE);
}

static void
ifft_process()
{
    fftw_execute(ifft_plan);
}

static void
ifft_exit()
{
    fftw_destroy_plan(ifft_plan);
}

static int
hpb(int const n)
{
    if (n == 0 || n == (double)O / 2)
        return 1;
    if (1 <= n && n < (double)O / 2)
        return 2;
    return 0;
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

    const int channels = input_info.channels;

    double hop_buffer[HOP_SIZE];
    double frame_buffer[FRAME_SIZE];

    for (int sample = 0; sample < FRAME_SIZE / HOP_SIZE - 1; sample++) {
        if (read_samples(hop_buffer, input_file, channels))
            fill_frame_buffer(frame_buffer, hop_buffer);
        else {
            fprintf(stderr, "Not enough samples.\n");
            exit(EXIT_FAILURE);
        }
    }

    plot = gnuplot_init();
    gnuplot_setstyle(plot, "lines");

    fftw_complex fft_in[FRAME_SIZE], fft_out[FRAME_SIZE];
    fft_init(fft_in, fft_out);
    fftw_complex ifft_in[FRAME_SIZE], ifft_out[FRAME_SIZE];
    ifft_init(ifft_in, ifft_out);
    double amplitudes[FRAME_SIZE];

    int frame_id = 0;
    while (read_samples(hop_buffer, input_file, channels)) {
        fill_frame_buffer(frame_buffer, hop_buffer);

        // Rectangular Window
        for (int i = 0; i < FRAME_SIZE; i++)
            fft_in[i] = frame_buffer[i];

        // FFT
        fft_process();

        // Amplitudes
        for (int i = 0; i < FRAME_SIZE; i++)
            amplitudes[i] = log(cabs(fft_out[i]));

        if (PLOT) {
            gnuplot_resetplot(plot);
            gnuplot_plot_x(plot, amplitudes, FRAME_SIZE / 2, "Amplitudes Spectrum");
        }

        for (int i = 0; i < FRAME_SIZE; i++)
            ifft_in[i] = amplitudes[i];

        // IFFT
        ifft_process();

        // Filter
        for (int i = 0; i < FRAME_SIZE; i++)
            fft_in[i] = hpb(i) * ifft_out[i];

        // FFT
        fft_process();

        // Amplitudes
        for (int i = 0; i < FRAME_SIZE; i++)
            amplitudes[i] = creal(fft_out[i]) / FRAME_SIZE;

        if (PLOT) {
            gnuplot_plot_x(plot, amplitudes, FRAME_SIZE / 2, "Sprectal Envelope");
            sleep(1);
        }

        frame_id++;
    }

    fft_exit();
    ifft_exit();
    sf_close(input_file);
    return EXIT_SUCCESS;
}
