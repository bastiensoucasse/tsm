#include <complex.h>
#include <ctype.h>
#include <fftw3.h>
#include <math.h>
#include <sndfile.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gnuplot_i.h"

#define SAMPLING_FREQ 44100.
#define FFT_SIZE 1024
#define FRAME_SIZE 1024
#define HOP_SIZE 1024

static gnuplot_ctrl* h;
static fftw_plan plan;

static void usage(char* progname)
{
    printf("\nUsage: %s <input file> \n", progname);
    puts("\n");
}

static void fill_buffer(double* buffer, double* new_buffer)
{
    double tmp[FRAME_SIZE - HOP_SIZE];

    for (int i = 0; i < FRAME_SIZE - HOP_SIZE; i++)
        tmp[i] = buffer[i + HOP_SIZE];

    for (int i = 0; i < (FRAME_SIZE - HOP_SIZE); i++)
        buffer[i] = tmp[i];

    for (int i = 0; i < HOP_SIZE; i++)
        buffer[FRAME_SIZE - HOP_SIZE + i] = new_buffer[i];
}

static int read_n_samples(SNDFILE* infile, double* buffer, int channels, int n)
{

    if (channels == 1) {
        int readcount;

        readcount = sf_readf_double(infile, buffer, n);

        return readcount == n;
    } else if (channels == 2) {
        double buf[2 * n];
        int readcount, k;
        readcount = sf_readf_double(infile, buf, n);
        for (k = 0; k < readcount; k++)
            buffer[k] = (buf[k * 2] + buf[k * 2 + 1]) / 2.0;

        return readcount == n;
    } else
        printf("Channel format error.\n");

    return 0;
}

static int read_samples(SNDFILE* infile, double* buffer, int channels)
{
    return read_n_samples(infile, buffer, channels, HOP_SIZE);
}

// static void dft(const double s[FRAME_SIZE], double complex S[FRAME_SIZE])
// {
//     for (unsigned int m = 0; m < FRAME_SIZE; m++) {
//         S[m] = 0.;
//         for (unsigned int n = 0; n < FRAME_SIZE; n++)
//             S[m] += s[n] * cexp(-I * 2 * M_PI * n * m / FRAME_SIZE);
//     }
// }

/* FFT */

static void
fft_init(fftw_complex data_in[FFT_SIZE], fftw_complex data_out[FFT_SIZE])
{
    plan = fftw_plan_dft_1d(FFT_SIZE, data_in, data_out, FFTW_FORWARD, FFTW_ESTIMATE);
}

static void
fft(const double s[FFT_SIZE], fftw_complex data_in[FFT_SIZE])
{
    for (unsigned int i = 0; i < FFT_SIZE; i++)
        data_in[i] = s[i];

    fftw_execute(plan);
}

static void
fft_exit()
{
    fftw_destroy_plan(plan);
}

static void
cartesian_to_polar(const double complex S[FFT_SIZE], double amp[FFT_SIZE], double phs[FFT_SIZE])
{
    for (unsigned int m = 0; m < FFT_SIZE; m++) {
        amp[m] = cabs(S[m]);
        phs[m] = carg(S[m]);
    }
}

static double
hann(unsigned int n)
{
    return .5 - .5 * cos(2 * M_PI * n / FRAME_SIZE);
}

int main(int argc, char** argv)
{
    char *progname, *infilename;
    SNDFILE* infile = NULL;
    SF_INFO sfinfo;

    progname = strrchr(argv[0], '/');
    progname = progname ? progname + 1 : argv[0];

    if (argc != 2) {
        usage(progname);
        exit(EXIT_FAILURE);
    }

    infilename = argv[1];

    if ((infile = sf_open(infilename, SFM_READ, &sfinfo)) == NULL) {
        printf("Not able to open input file %s.\n", infilename);
        puts(sf_strerror(NULL));
        exit(EXIT_FAILURE);
    }

    int nb_frames = 0;
    double new_buffer[HOP_SIZE];
    double buffer[FRAME_SIZE];

    h = gnuplot_init();
    gnuplot_setstyle(h, "lines");

    for (int i = 0; i < (FRAME_SIZE / HOP_SIZE - 1); i++) {
        if (read_samples(infile, new_buffer, sfinfo.channels) == 1)
            fill_buffer(buffer, new_buffer);
        else {
            printf("not enough samples !!\n");
            return 1;
        }
    }

    printf("Sample rate: %d.\n", sfinfo.samplerate);
    printf("Channels: %d.\n", sfinfo.channels);
    printf("Size: %d.\n", (int)sfinfo.frames);

    double s[FFT_SIZE], amp[FFT_SIZE], phs[FFT_SIZE];
    fftw_complex data_in[FFT_SIZE], data_out[FFT_SIZE];
    fft_init(data_in, data_out);

    while (read_samples(infile, new_buffer, sfinfo.channels) == 1) {
        printf("\nProcessing frame %d…\n", nb_frames);
        fill_buffer(buffer, new_buffer);

        for (unsigned int i = 0; i < FFT_SIZE; i++)
            s[i] = i < FRAME_SIZE ? buffer[i] * hann(i) : 0.;

        fft(s, data_in);
        cartesian_to_polar(data_out, amp, phs);

        for (unsigned int i = 0; i < FFT_SIZE; i++)
            amp[i] *= 2. / FRAME_SIZE;

        double max_amp = amp[0];
        unsigned int max_amp_sample;
        for (unsigned int i = 1; i < FFT_SIZE / 2; i++)
            if (amp[i] > max_amp) {
                max_amp = amp[i];
                max_amp_sample = i;
            }
        double max_amp_freq = max_amp_sample * SAMPLING_FREQ / FFT_SIZE;

        printf("Max amp: %lf, max amp freq: %lf (±%lf).\n", max_amp, max_amp_freq, SAMPLING_FREQ / (2 * FFT_SIZE));

        printf("Parabolic Interpolation…\n");
        const int m = max_amp_sample;
        const double al = 20 * log10(amp[m - 1]);
        const double ac = 20 * log10(amp[m]);
        const double ar = 20 * log10(amp[m + 1]);
        const double d = .5 * (al - ar) / (al - 2 * ac + ar);
        const double m_hat = m + d;
        const double f_hat = m_hat * SAMPLING_FREQ / FFT_SIZE;
        const double a_hat = ac - (al - ar) * d / 4;
        printf("Max amp: %lf, max amp freq: %lf.\n", a_hat, f_hat);

        nb_frames++;
    }

    fft_exit();
    sf_close(infile);
    return EXIT_SUCCESS;
}
