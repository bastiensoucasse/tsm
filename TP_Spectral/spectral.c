#include <complex.h>
#include <ctype.h>
#include <fftw3.h>
#include <math.h>
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gnuplot_i.h"

#define FRAME_SIZE 1024
#define HOP_SIZE 1024

static gnuplot_ctrl* h;
static fftw_plan plan;

static void print_usage(char* progname)
{
    printf("\nUsage : %s <input file> \n", progname);
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

static void dft(const double s[FRAME_SIZE], double complex S[FRAME_SIZE])
{
    for (unsigned int m = 0; m < FRAME_SIZE; m++) {
        S[m] = 0.;
        for (unsigned int n = 0; n < FRAME_SIZE; n++)
            S[m] += s[n] * cexp(-I * 2 * M_PI * n * m / FRAME_SIZE);
    }
}

// static void cartesian_to_polar(const double complex S[FRAME_SIZE], double amp[FRAME_SIZE], double phs[FRAME_SIZE])
// {
//     for (unsigned int m = 0; m < FRAME_SIZE; m++) {
//         amp[m] = cabs(S[m]);
//         phs[m] = carg(S[m]);
//     }
// }

static void fft_init(fftw_complex data_in[FRAME_SIZE], fftw_complex data_out[FRAME_SIZE])
{
    plan = fftw_plan_dft_1d(FRAME_SIZE, data_in, data_out, FFTW_FORWARD, FFTW_ESTIMATE);
}

static void fft(const double s[FRAME_SIZE], fftw_complex data_in[FRAME_SIZE])
{
    for (unsigned int i = 0; i < FRAME_SIZE; i++)
        data_in[i] = s[i];

    fftw_execute(plan);
}

static void fft_exit()
{
    fftw_destroy_plan(plan);
}

int main(int argc, char** argv)
{
    char *progname, *infilename;
    SNDFILE* infile = NULL;
    SF_INFO sfinfo;

    progname = strrchr(argv[0], '/');
    progname = progname ? progname + 1 : argv[0];

    if (argc != 2) {
        print_usage(progname);
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

    // For the DFT
    double complex S[FRAME_SIZE];

    // For the FFT
    fftw_complex data_in[FRAME_SIZE], data_out[FRAME_SIZE];
    fft_init(data_in, data_out);

    // The results
    // double amp[FRAME_SIZE], phs[FRAME_SIZE];

    // The durations
    clock_t dft_single_duration, dft_full_duration = 0, fft_single_duration, fft_full_duration = 0;

    while (read_samples(infile, new_buffer, sfinfo.channels) == 1) {
        printf("Processing frame %d…\n", nb_frames);

        fill_buffer(buffer, new_buffer);

        // DFT
        dft_single_duration = clock();
        dft(buffer, S);
        dft_single_duration = clock() - dft_single_duration;
        dft_full_duration += dft_single_duration;
        printf("DFT Duration: %fs.\n", (double)dft_single_duration / CLOCKS_PER_SEC);
        // cartesian_to_polar(S, amp, phs);

        // FFT
        fft_single_duration = clock();
        fft(buffer, data_in);
        fft_single_duration = clock() - fft_single_duration;
        fft_full_duration += fft_single_duration;
        printf("FFT Duration: %fs.\n", (double)fft_single_duration / CLOCKS_PER_SEC);
        // cartesian_to_polar(data_out, amp, phs);

        // Plot
        // gnuplot_resetplot(h);
        // gnuplot_plot_x(h, amp, FRAME_SIZE, "temporal frame");
        // sleep(1);

        nb_frames++;
    }

    printf("Full DFT Duration: %fs.\n", (double)dft_full_duration / CLOCKS_PER_SEC);
    printf("Full FFT Duration: %fs.\n", (double)fft_full_duration / CLOCKS_PER_SEC);

    printf("Average DFT Duration: %fs.\n", ((double)dft_full_duration / CLOCKS_PER_SEC) / nb_frames);
    printf("Average FFT Duration: %fs.\n", ((double)fft_full_duration / CLOCKS_PER_SEC) / nb_frames);

    fft_exit();
    sf_close(infile);
    return EXIT_SUCCESS;
}
