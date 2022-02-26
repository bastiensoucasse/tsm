#include <complex.h>
#include <fftw3.h>
#include <math.h>
#include <sndfile.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "gnuplot_i.h"

#define PLOT false

#define FRAME_SIZE 2205
#define HOP_SIZE 2205

#define AMP_THRESHOLD 40.

static gnuplot_ctrl* h; // Plot graph.
static fftw_plan plan; // FFT plan.


// Correspondance table.
static char event_name[3] = {'A', 'B', 'C'};
static double event_freq[3] = {19126., 19584., 20032.};

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
cartesian_to_polar(double complex S[FRAME_SIZE], double amp[FRAME_SIZE], double phs[FRAME_SIZE])
{
    for (int n = 0; n < FRAME_SIZE; n++) {
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
fft_init(fftw_complex data_in[FRAME_SIZE], fftw_complex data_out[FRAME_SIZE])
{
    plan = fftw_plan_dft_1d(FRAME_SIZE, data_in, data_out, FFTW_FORWARD, FFTW_ESTIMATE);
}

/**
 * @brief Executes the FFT.
 *
 * @param s The input signal (that will be stored into the pre FFT buffer).
 * @param data_in The pre FFT buffer.
 */
static void
fft(double s[FRAME_SIZE], fftw_complex data_in[FRAME_SIZE])
{
    for (int i = 0; i < FRAME_SIZE; i++)
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
 * @brief Converts a signal value into Hann window.
 *
 * @param n The value to convert.
 * @return The converted value.
 */
static double
hann(double n)
{
    return .5 - .5 * cos(2 * M_PI * n / FRAME_SIZE);
}

/**
 * @brief Estimates peak frequency from previous and next samples values.
 * 
 * @param amp
 * @param sample
 * @param SAMPLE_RATE
 * 
 * @return The peak frequency computed.
 */
static double
parabolic_interpolation(double amp[FRAME_SIZE], int sample, int SAMPLE_RATE)
{
    double al = 20 * log(amp[sample - 1]);
    double ac = 20 * log(amp[sample]);
    double ar = 20 * log(amp[sample + 1]);
    double delta = .5 * (al - ar) / (al - 2 * ac + ar);
    return (sample + delta) * SAMPLE_RATE / FRAME_SIZE;
}

/**
 * @brief Checks if there is a peak above 18000 Hz in a portion of the signal.
 * If so, return the value of the first one found,
 * and put the sample index in parameter sample_index.
 * 
 * @param amp 
 * @param SAMPLE_RATE
 * @param sample_index
 * 
 * @return Frequence if found.
 *         -1 otherwhise.
 */
static double
inaudible_peak(double amp[FRAME_SIZE], int SAMPLE_RATE, int *sample_index)
{
    double freq;
    for (int i = 0; i < FRAME_SIZE/2; i++)

        // Check if there is a peak
        if (amp[i] >= amp[i-1] && amp[i] > amp[i+1])
        {
            freq = round(parabolic_interpolation(amp, i, SAMPLE_RATE));

            // Check if the frequence is inaudible
            if (freq > 18000. && amp[i] > AMP_THRESHOLD) 
            {
                // printf("amplitude: %lf\n", amp[i]);
                *sample_index = i;
                return freq;
            }   
        }
    return -1;
}

/**
 * @brief Checks if freq corresponds to an existing event. 
 * If so, puts the event name in event parameter and computes time code.
 * 
 * @param freq
 * @param sample_index
 * @param nb_frames
 * @param event
 * @param time_code
 * 
 * @return True if it exists a corresponding event
 *         False otherwise.
 */
static bool
is_watermark(double freq, int sample_index, int nb_frames, int SAMPLE_RATE, char *event, double *time_code)
{
    for (int i = 0; i < 3; i++)
        if (event_freq[i] == freq)
        {
            *event = event_name[i];
            *time_code = ((double) FRAME_SIZE/SAMPLE_RATE) * nb_frames + (sample_index/SAMPLE_RATE);
            return true;
        }
    return false;
}


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

    // Check whether FRAME_SIZE & HOP_FILE are correct for file.
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
    const unsigned int SIZE = (int)sfinfo.frames; 

    // Display file info.
    printf("Sample Rate: %d.\n", SAMPLE_RATE);
    printf("Channels: %d.\n", NUM_CHANNELS);
    printf("Size: %d.\n", SIZE);

    // Initialize FFT.
    double amp[FRAME_SIZE], phs[FRAME_SIZE];
    fftw_complex data_in[FRAME_SIZE], data_out[FRAME_SIZE];
    fft_init(data_in, data_out);

    // bool is_prev_silence = false;
    // char prev_key = ' ';

    // Loop over each frame.
    while (read_samples(infile, new_buffer, sfinfo.channels) == 1) {
        // printf("\nProcessing frame %d…\n", nb_frames);

        // Fill frame buffer (original signal during the frame).
        fill_buffer(buffer, new_buffer);

        // Hann window.
        for (int i = 0; i < FRAME_SIZE; i++)
            buffer[i] *= hann(i);

        // Execute FFT.
        fft(buffer, data_in);
        cartesian_to_polar(data_out, amp, phs);

        // Normalize amplitude signal (values between 0 and 1).
        // for (int i = 0; i < FRAME_SIZE; i++)
        //     amp[i] *= 2. / FRAME_SIZE;

        // Retrieve maximum amplitude, and position associated.
        double max_amp = amp[0];
        int max_amp_i = 0;
        for (int i = 1; i < FRAME_SIZE / 2; i++) { // No need to go further, the other half is symetrical.
            if (max_amp < amp[i]) {
                max_amp = amp[i];
                max_amp_i = i;
            }
        }

        // Find watermark.
        int sample_index;
        double freq = inaudible_peak(amp, SAMPLE_RATE, &sample_index);
        // if (freq > 0) printf("%lf\n", freq);

        char event;
        double time_code;
        if (is_watermark(freq, sample_index, nb_frames, SAMPLE_RATE, &event, &time_code))
            printf("%.2f s: %c\n", time_code, event);

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


    // Shut down FFT, close file and exit program.
    fft_exit();
    sf_close(infile);
    return EXIT_SUCCESS;
}