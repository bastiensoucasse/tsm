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

#define VERBOSE true

#define FRAME_SIZE 1024
#define HOP_SIZE FRAME_SIZE
#define FFT_SIZE FRAME_SIZE

static gnuplot_ctrl* h;
static fftw_plan plan;

static void
usage(char* progname)
{
    fprintf(stderr, "Usage: %s <input file>.\n", progname);
}

static void
fill_buffer(double* buffer, double* new_buffer)
{
    int i;
    double tmp[FRAME_SIZE - HOP_SIZE];

    for (i = 0; i < FRAME_SIZE - HOP_SIZE; i++)
        tmp[i] = buffer[i + HOP_SIZE];

    for (i = 0; i < (FRAME_SIZE - HOP_SIZE); i++)
        buffer[i] = tmp[i];

    for (i = 0; i < HOP_SIZE; i++)
        buffer[FRAME_SIZE - HOP_SIZE + i] = new_buffer[i];
}

static int
read_n_samples(SNDFILE* infile, double* buffer, int channels, int n)
{
    if (channels == 1) {
        int readcount;
        readcount = sf_readf_double(infile, buffer, n);
        return readcount == n;
    } else if (channels == 2) {
        double buf[2 * n];
        int readcount;
        readcount = sf_readf_double(infile, buf, n);
        for (int k = 0; k < readcount; k++)
            buffer[k] = (buf[k * 2] + buf[k * 2 + 1]) / 2.;
        return readcount == n;
    } else
        printf("Channel format error.\n");
    return 0;
}

static int
read_samples(SNDFILE* infile, double* buffer, int channels)
{
    return read_n_samples(infile, buffer, channels, HOP_SIZE);
}

/**
 * @brief Converts a complex signal from the FFT into two signals of amplitude and phase.
 *
 * @param S The complex signal.
 * @param amp The amplitude signal.
 * @param phs The phase signal.
 */
static void
cartesian_to_polar(double complex S[FFT_SIZE], double amp[FFT_SIZE], double phs[FFT_SIZE])
{
    for (int n = 0; n < FFT_SIZE; n++) {
        amp[n] = cabs(S[n]);
        phs[n] = carg(S[n]);
    }
}

/**
 * @brief Initializes the FFT by declaring the data in and out buffers.
 *
 * @param data_in The complex signal before FFT.
 * @param data_out The complex signal after FFT.
 */
static void
fft_init(fftw_complex data_in[FFT_SIZE], fftw_complex data_out[FFT_SIZE])
{
    plan = fftw_plan_dft_1d(FFT_SIZE, data_in, data_out, FFTW_FORWARD, FFTW_ESTIMATE);
}

/**
 * @brief Executes the FFT.
 *
 * @param s The input signal (that will be stored into the pre FFT buffer).
 * @param data_in The pre FFT buffer.
 */
static void
fft(double s[FFT_SIZE], fftw_complex data_in[FFT_SIZE])
{
    for (int i = 0; i < FFT_SIZE; i++)
        data_in[i] = s[i];

    fftw_execute(plan);
}

/**
 * @brief Cleans up memory used for the FFT.
 */
static void
fft_exit()
{
    fftw_destroy_plan(plan);
}

/**
 * @brief HANN
 * TODO: Function Documentation.
 *
 * @param n
 * @return double
 */
static double
hann(double n)
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
        return 1;
    }

    infilename = argv[1];

    if ((infile = sf_open(infilename, SFM_READ, &sfinfo)) == NULL) {
        fprintf(stderr, "Not able to open input file %s.\n", infilename);
        puts(sf_strerror(NULL));
        return EXIT_FAILURE;
    }

    int nb_frames = 0;
    double new_buffer[HOP_SIZE];
    double buffer[FRAME_SIZE];

    h = gnuplot_init();
    gnuplot_setstyle(h, "lines");

    for (int i = 0; i < FRAME_SIZE / HOP_SIZE - 1; i++) {
        if (read_samples(infile, new_buffer, sfinfo.channels) == 1)
            fill_buffer(buffer, new_buffer);
        else {
            fprintf(stderr, "Not enough samples.\n");
            return EXIT_FAILURE;
        }
    }

    // Retrieve file info.
    const unsigned int SAMPLE_RATE = sfinfo.samplerate; // 44100 Hz.
    const unsigned char NUM_CHANNELS = sfinfo.channels; // 1 (mono).
    const unsigned int NUM_FRAMES = (int)sfinfo.frames; // 132300.

    // Display file info.
    printf("Sample rate: %d.\n", SAMPLE_RATE);
    printf("Number of channels: %d.\n", NUM_CHANNELS);
    printf("Number of frames: %d.\n", NUM_FRAMES);

    // Initialize FFT.
    double fft_buffer[FFT_SIZE], amp[FFT_SIZE], phs[FFT_SIZE];
    fftw_complex data_in[FFT_SIZE], data_out[FFT_SIZE];
    fft_init(data_in, data_out);

    // Loop over each frame.
    while (read_samples(infile, new_buffer, sfinfo.channels) == 1) {
        printf("\nProcessing frame %d…\n", nb_frames);

        // Fill frame buffer (original signal during the frame).
        fill_buffer(buffer, new_buffer);

        // Fill FFT buffer (frame buffer and zeros if necessary).
        for (int i = 0; i < FFT_SIZE; i++)
            fft_buffer[i] = i < FRAME_SIZE ? buffer[i] * hann(i) : 0.;

        // Execute FFT.
        fft(fft_buffer, data_in);
        cartesian_to_polar(data_out, amp, phs);

        // Normalize amplitude signal (values between 0 and 1).
        for (int i = 0; i < FFT_SIZE; i++)
            amp[i] *= 2. / FRAME_SIZE;

        // Retrieve maximum amplitude, and position associated.
        double max_amp = amp[0];
        int max_amp_i = 0;
        for (int i = 1; i < FFT_SIZE / 2; i++) { // No need to go further, the other half is symetrical.
            if (max_amp < amp[i]) {
                max_amp = amp[i];
                max_amp_i = i;
            }
        }

        // Check if signal is not null.
        if (max_amp == 0) {
            printf("Null frame, skipping…\n");
            nb_frames++;
            continue;
        }

        if (VERBOSE)
            printf("Max amplitude: %lf.\n", max_amp);

        // Define FFT frequency precision.
        double freq_prec = SAMPLE_RATE / (2. * FFT_SIZE);

        // Compute maximum amplitude frequency.
        double max_amp_freq = max_amp_i * SAMPLE_RATE / FFT_SIZE;
        printf("Max amplitude frequency: %lf (± %lf) Hz.\n", max_amp_freq, freq_prec);

        // Estimate peak frequency (parabolic interpolation).
        double al = 20 * log(amp[max_amp_i - 1]);
        double ac = 20 * log(amp[max_amp_i]);
        double ar = 20 * log(amp[max_amp_i + 1]);
        double delta = .5 * (al - ar) / (al - 2 * ac + ar);
        double peak_freq = (max_amp_i + delta) * SAMPLE_RATE / FFT_SIZE;
        printf("Estimated peak frequency: %lf Hz.\n", peak_freq);

        // Increase frame id for next frame.
        nb_frames++;
    }

    // Shut down FFT, close file and exit program.
    fft_exit();
    sf_close(infile);
    return EXIT_SUCCESS;
}
