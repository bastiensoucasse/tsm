#include <complex.h>
#include <fftw3.h>
#include <math.h>
#include <sndfile.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "gnuplot_i.h"

#define VERBOSE false
#define PLOT true

#define FRAME_SIZE 8820
#define HOP_SIZE 4410
#define FFT_SIZE FRAME_SIZE

static gnuplot_ctrl* h; // Plot graph.
static fftw_plan plan; // FFT plan.

static double tab[12][2];


/**
 * @brief Prints usage on the console.
 *
 * @param progname The program name entered in command line.
 */
static void
usage(char* progname)
{
    fprintf(stderr, "Usage: %s <input file>.\n", progname);
}

/**
 * @brief Fills the frame buffer from the hop buffer.
 *
 * @param buffer The frame buffer.
 * @param new_buffer The hop buffer.
 */
static void
fill_buffer(double* buffer, double* new_buffer)
{
    int i;
    double tmp[FRAME_SIZE - HOP_SIZE];

    for (i = 0; i < FRAME_SIZE - HOP_SIZE; i++)
        tmp[i] = buffer[i + HOP_SIZE];

    for (i = 0; i < FRAME_SIZE - HOP_SIZE; i++)
        buffer[i] = tmp[i];

    for (i = 0; i < HOP_SIZE; i++)
        buffer[FRAME_SIZE - HOP_SIZE + i] = new_buffer[i];
}

/**
 * @brief Reads n samples from file into the buffer.
 *
 * @param infile The file (sound) to read.
 * @param buffer The buffer to fill.
 * @param channels The number of channels of the sound.
 * @param n The number of samples to read into buffer.
 * @return True if the program read n samples successfully; false otherwise.
 */
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

/**
 * @brief Reads HOP_SIZE samples from file into the hop buffer.
 *
 * @param infile The file (sound) to read.
 * @param buffer The hop buffer to fill.
 * @param channels The number of channels of the sound.
 * @return True if the program read HOP_SIZE samples successfully; false otherwise.
 */
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
 * @brief Prints the number of local maxima in amplitude spectrum
 * Note: first and last sample cannot be local maxima
 * 
 * @param amp 
 */
static void
retrieve_peaks(double amp[FFT_SIZE])
{
    int n_peaks = 0;
    for (int i = 1; i < FFT_SIZE - 1; i++)
        if (amp[i] >= amp[i-1] && amp[i] > amp[i+1])
            n_peaks++;

    printf("%d peaks found\n", n_peaks);
}

/**
 * @brief Retrieves sample index of the 2 local maxima with maximum amplitude
 * Note: first and last sample cannot be local maxima
 * 
 * @param amp 
 */
static void
retrieve_2_peaks(double amp[FFT_SIZE], int nb, int SAMPLE_RATE)
{
    int peak1_sample = 0, peak2_sample = 0;
    for (int i = 1; i < FFT_SIZE/2 - 1; i++)
        if (amp[i] >= amp[i-1] && amp[i] > amp[i+1])
        {
            if (amp[i] > amp[peak1_sample])
            {
                peak2_sample = peak1_sample;
                peak1_sample = i;
            } 
            else if (amp[i] > amp[peak2_sample])
                peak2_sample = i;
        }
    
    tab[nb][0] = peak1_sample * SAMPLE_RATE / FFT_SIZE;
    tab[nb][1] = peak2_sample * SAMPLE_RATE / FFT_SIZE;
}

// /**
//  * @brief Converts a signal value into Hann window.
//  *
//  * @param n The value to convert.
//  * @return The converted value.
//  */
// static double
// hann(double n)
// {
//     return .5 - .5 * cos(2 * M_PI * n / FRAME_SIZE);
// }

int main(int argc, char** argv)
{
    // Init file accessing.
    char *progname, *infilename;
    SNDFILE* infile = NULL;
    SF_INFO sfinfo;

    // Retrieve program name.
    progname = strrchr(argv[0], '/');
    progname = progname ? progname + 1 : argv[0];

    // Check correct usage.
    if (argc != 2) {
        usage(progname);
        return 1;
    }

    // Retrieve input file name.
    infilename = argv[1];

    // Open input file.
    if ((infile = sf_open(infilename, SFM_READ, &sfinfo)) == NULL) {
        fprintf(stderr, "Not able to open input file %s.\n", infilename);
        puts(sf_strerror(NULL));
        return EXIT_FAILURE;
    }

    // Init file reading.
    int nb_frames = 0;
    double new_buffer[HOP_SIZE];
    double buffer[FRAME_SIZE];

    // Init ploting.
    h = gnuplot_init();
    gnuplot_setstyle(h, "lines");

    // Check wether FRAME_SIZE & HOP_FILE are correct for file.
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
    const unsigned int SIZE = (int)sfinfo.frames; // 132300.

    // Display file info.
    printf("Sample Rate: %d.\n", SAMPLE_RATE);
    printf("Channels: %d.\n", NUM_CHANNELS);
    printf("Size: %d.\n", SIZE);

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
        for (int i = 0; i < FFT_SIZE; i++) {
            // fft_buffer[i] = i < FRAME_SIZE ? buffer[i] * hann(i) : 0.; // For using w/ Hann.
            fft_buffer[i] = i < FRAME_SIZE ? buffer[i] : 0.;
        }

        // Execute FFT.
        fft(fft_buffer, data_in);
        cartesian_to_polar(data_out, amp, phs);
<<<<<<< HEAD
=======
        // printf("ok\n"), exit(1);
>>>>>>> e3ae2f447893a8bb458f91e7010d368a87c2af65

        // Normalize amplitude signal (values between 0 and 1).
        // for (int i = 0; i < FFT_SIZE; i++)
        //     amp[i] *= 2. / FRAME_SIZE;

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

        // if (VERBOSE)
        //     printf("Max amplitude: %lf.\n", max_amp);

        // // Define FFT frequency precision.
        // double freq_prec = SAMPLE_RATE / (2. * FFT_SIZE);

        // // Compute maximum amplitude frequency.
        // double max_amp_freq = max_amp_i * SAMPLE_RATE / FFT_SIZE;
        // printf("Max amplitude frequency: %lf (± %lf) Hz.\n", max_amp_freq, freq_prec);

        // // Estimate peak frequency (parabolic interpolation).
        // double al = 20 * log(amp[max_amp_i - 1]);
        // double ac = 20 * log(amp[max_amp_i]);
        // double ar = 20 * log(amp[max_amp_i + 1]);
        // double delta = .5 * (al - ar) / (al - 2 * ac + ar);
        // double peak_freq = (max_amp_i + delta) * SAMPLE_RATE / FFT_SIZE;
        // printf("Estimated peak frequency: %lf Hz.\n", peak_freq);

        // retrieve_peaks(amp);

        int nb;
        if (nb_frames % 3 == 0)
        {
            nb = nb_frames / 3;
            retrieve_2_peaks(amp, nb, SAMPLE_RATE);
        }

        // Display the frame.
        if (PLOT) {
            gnuplot_resetplot(h);
            // gnuplot_plot_x(h, buffer, FRAME_SIZE, "Temporal Frame");
            gnuplot_plot_x(h, amp, FRAME_SIZE / 10, "Spectral Frame");
            sleep(1);
        }

        // Increase frame id for next frame.
        nb_frames++;
    }

    for (int i = 0; i < 12; i++)
        printf("%d = (%lf, %lf)\n", (i+1)%10, tab[i][0], tab[i][1]);


    // Shut down FFT, close file and exit program.
    fft_exit();
    sf_close(infile);
    return EXIT_SUCCESS;
}
