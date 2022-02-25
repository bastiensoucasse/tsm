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

#define FRAME_SIZE 8820 // 2867 // 8820 // 158760
#define HOP_SIZE  1024 // 2867 // 2867 // 4410 // 1024
#define FFT_SIZE FRAME_SIZE

static gnuplot_ctrl* h; // Plot graph.
static fftw_plan plan; // FFT plan.

// static int c_tab[12][2]; // Correspondance table

static double line[4] = {697., 770., 852., 941.};
static double col[3] = {1209., 1336., 1477.};

static char c_tab[4][3] =
{
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};


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
 * @brief Prints the number of local maxima in amplitude spectrum.
 * Note: first and last sample cannot be local maxima.
 * 
 * @param amp 
 */
static void
display_nb_peaks(double amp[FFT_SIZE])
{
    int n_peaks = 0;
    for (int i = 1; i < FFT_SIZE - 1; i++)
        if (amp[i] >= amp[i-1] && amp[i] > amp[i+1])
            n_peaks++;

    printf("%d peaks found\n", n_peaks);
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
parabolic_interpolation(double amp[FFT_SIZE], int sample, int SAMPLE_RATE)
{
    double al = 20 * log(amp[sample - 1]);
    double ac = 20 * log(amp[sample]);
    double ar = 20 * log(amp[sample + 1]);
    double delta = .5 * (al - ar) / (al - 2 * ac + ar);
    return (sample + delta) * SAMPLE_RATE / FFT_SIZE;
}

/**
 * @brief Retrieves sample index of the 2 local maxima.
 * Note: first and last sample cannot be local maxima.
 * 
 * @param amp 
 * @param peak1_sample
 * @param peak2_sample
 */
static void
retrieve_2_peaks(double amp[FFT_SIZE], int* peak1_sample, int* peak2_sample/*, int nb, int SAMPLE_RATE*/)
{
    *peak1_sample = 0, *peak2_sample = 0;
    for (int i = 1; i < FFT_SIZE/2 - 1; i++)
        if (amp[i] >= amp[i-1] && amp[i] > amp[i+1])
        {
            if (amp[i] > amp[*peak1_sample])
            {
                *peak2_sample = *peak1_sample;
                *peak1_sample = i;
            } 
            else if (amp[i] > amp[*peak2_sample])
                *peak2_sample = i;
        }
}

/**
 * @brief Computes the frequences corresponding to the 2 local maxima.
 * Note: first and last sample cannot be local maxima.
 * 
 * @param amp 
 * @param freq1
 * @param freq2
 * @param SAMPLE_RATE
 */
static void
retrieve_2_freq(double amp[FFT_SIZE], double* freq1, double* freq2, int SAMPLE_RATE)
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

    *freq1 = round(parabolic_interpolation(amp, peak1_sample, SAMPLE_RATE));
    *freq2 = round(parabolic_interpolation(amp, peak2_sample, SAMPLE_RATE));
}

/**
 * @brief Fills correspondance table for number nb.
 * 
 * @param sample1
 * @param sample2
 * @param nb
 * @param SAMPLE_RATE
 */
static void
fill_c_tab(double amp[FFT_SIZE], int sample1, int sample2, int nb, int SAMPLE_RATE)
{
    // c_tab[nb][0] = sample1 * SAMPLE_RATE / FFT_SIZE;
    // c_tab[nb][1] = sample2 * SAMPLE_RATE / FFT_SIZE;
    c_tab[nb][0] = round(parabolic_interpolation(amp, sample1, SAMPLE_RATE));
    c_tab[nb][1] = round(parabolic_interpolation(amp, sample2, SAMPLE_RATE));
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
 * @brief Finds the index of the first occurrence of an element in an list of double.
 * 
 * @param list
 * @param n size of the list
 * @param el element wanted in the list
 * 
 * @return The index found.
 */ 
static int
indexOf(double* list, int n, double el)
{
    for (int i = 0; i < n; i++)
        if (list[i] == el) 
            return i;
    printf("Element not in list\n");
    exit(EXIT_FAILURE);
}

static int
nearest_freq_index(double freq, double prec)
{
    int n;
    double *list;

    if (freq < 1000)
        list = line, n = 4;
    else
        list = col, n = 3;

    for (int i = 0; i < n; i++)
        if (list[i]-prec <= freq && freq <= list[i]+prec)
            return i;

    printf("Error: %lf is an invalid frequence\n", freq);
    exit(EXIT_FAILURE);
}

/**
 * @brief Finds the key corresponding to a signal with frequences freq1 and freq2.
 * 
 * @param freq1
 * @param freq2
 * 
 * @return The char corresponding.
 */
static char
decode(double freq1, double freq2, double prec)
{
    int i1 = nearest_freq_index(freq1, 2);
    int i2 = nearest_freq_index(freq2, 2);
    if (freq1 < 1000)
        return c_tab[i1][i2];
    return c_tab[i2][i1];

    // if (freq1 < 1000)
    //     return c_tab[indexOf(line, 4, freq1)][indexOf(col, 3, freq2)];
    // return c_tab[indexOf(line, 4, freq2)][indexOf(col, 3, freq1)];
}

/**
 * @brief
 */
static double
energy(double buffer[FRAME_SIZE])
{
    double e = 0;
    for (int i = 0; i < FRAME_SIZE; i++)
        e += buffer[i]*buffer[i];
    return e / FRAME_SIZE;
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

    printf("Frame Size: %d.\n", FRAME_SIZE);
    printf("Hop Size: %d.\n", HOP_SIZE);
    printf("FFT Size: %d.\n", FFT_SIZE);

    // Initialize FFT.
    double fft_buffer[FFT_SIZE], amp[FFT_SIZE], phs[FFT_SIZE];
    fftw_complex data_in[FFT_SIZE], data_out[FFT_SIZE];
    fft_init(data_in, data_out);

    // bool is_prev_silence = false;
    // char prev_key = ' ';

    // Loop over each frame.
    while (read_samples(infile, new_buffer, sfinfo.channels) == 1) {
        // printf("\nProcessing frame %d…\n", nb_frames);

        // Fill frame buffer (original signal during the frame).
        fill_buffer(buffer, new_buffer);

        // Fill FFT buffer (frame buffer and zeros if necessary).
        // for (int i = 0; i < FFT_SIZE; i++) {
        //     // fft_buffer[i] = i < FRAME_SIZE ? buffer[i] * hann(i) : 0.; // For using w/ Hann.
        //     fft_buffer[i] = i < FRAME_SIZE ? buffer[i] : 0.;
        // }

        // Hann window.
        // for (int i = 0; i < FFT_SIZE; i++)
        //     buffer[i] *= hann(i);

        // Execute FFT.
        fft(buffer, data_in);
        cartesian_to_polar(data_out, amp, phs);

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
        // double threshold = 0.005;
        if (max_amp == 0 /*|| energy(buffer) < threshold*/) {
            // printf("Null frame, skipping…\n");
            // is_prev_silence = true;
            nb_frames++;
            continue;
        }

        // if (VERBOSE)
        //     printf("Max amplitude: %lf.\n", max_amp);

        // Define FFT frequency precision.
        // double freq_prec = SAMPLE_RATE / (2. * FFT_SIZE);
        // printf("Precision: %lf\n", freq_prec);

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

        // display_nb_peaks(amp);

        // Fill correspondance table (use when param is telbase.wav).
        // int nb, sample1, sample2;
        // if (nb_frames % 3 == 0)
        // {
        //     nb = nb_frames / 3;
        //     retrieve_2_peaks(amp, &sample1, &sample2/*nb, SAMPLE_RATE*/);
        //     fill_c_tab(amp, sample1, sample2, nb, SAMPLE_RATE);
        // }

        // double freq1, freq2;
        // retrieve_2_freq(amp, &freq1, &freq2, SAMPLE_RATE);
        // // printf("%lf, %lf\n", freq1, freq2);
        // char key = decode(freq1, freq2, freq_prec);

        // // Check if we are analysing the same key as previous frame.
        // if (prev_key == key)
        // {
        //     if (is_prev_silence)
        //         printf("%c", key);
                
        // } else
        //     printf("%c", key);

        // Display frame energy.
        // printf("%lf\n", energy(buffer));

        // Display the frame.
        if (PLOT) {
            gnuplot_resetplot(h);
            // gnuplot_plot_x(h, buffer, FRAME_SIZE, "Temporal Frame");
            gnuplot_plot_x(h, amp, FRAME_SIZE / 10, "Spectral Frame");
            sleep(1);
        }

        // Increase frame id for next frame.
        nb_frames++;

        // is_prev_silence = false;
        // prev_key = key;
    }

    // printf("\n");

    // Displays correspondance table values.
    // for (int i = 0; i < 10; i++)
    //     printf("%d = (%d, %d)\n", (i+1)%10, c_tab[i][0], c_tab[i][1]);
    // printf("* = (%d, %d)\n", c_tab[10][0], c_tab[10][1]);
    // printf("# = (%d, %d)\n", c_tab[11][0], c_tab[11][1]);

    // Shut down FFT, close file and exit program.
    fft_exit();
    sf_close(infile);
    return EXIT_SUCCESS;
}