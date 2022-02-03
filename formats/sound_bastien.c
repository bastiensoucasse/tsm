#include <complex.h>
#include <fftw3.h>
#include <math.h>

#include "gnuplot_i.h"

#define SAMPLING_RATE 44100
#define NUM_BITS 16
#define NUM_CHANNELS 1
#define ENCODING "signed-integer"
#define N 1024

static char* RAW_FILE = "tmp-in.raw";

static gnuplot_ctrl* h;
static fftw_plan plan;

static FILE* sound_file_open_read(char* sound_file_name)
{
    char cmd[256];
    snprintf(cmd, 256, "sox %s -c %d -r %d -b %d -e %s %s", sound_file_name, NUM_CHANNELS, SAMPLING_RATE, NUM_BITS, ENCODING, RAW_FILE);
    system(cmd);
    return fopen(RAW_FILE, "rb");
}

static int sound_file_read(FILE* fp, double* s)
{
    short tmp[N];
    size_t num_read = fread(tmp, NUM_BITS / 8, N, fp);

    for (int i = 0; i < N; i++)
        s[i] = (double)tmp[i] / pow(2, NUM_BITS - 1);

    return num_read;
}

static void sound_file_close_read(FILE* fp)
{
    fclose(fp);
}

static void fft_init(fftw_complex data_in[N], fftw_complex data_out[N])
{
    plan = fftw_plan_dft_1d(N, data_in, data_out, FFTW_FORWARD, FFTW_ESTIMATE);
}

static void fft(const double s[N], fftw_complex data_in[N])
{
    for (unsigned int i = 0; i < N; i++)
        data_in[i] = s[i];

    fftw_execute(plan);
}

static void fft_exit()
{
    fftw_destroy_plan(plan);
}

static void cartesian_to_polar(double complex S[N], double amp[N])
{
    for (int n = 0; n < N; n++)
        amp[n] = cabs(S[n]) / N * 2;
}

int main(int argc, char** argv)
{
    FILE* input;
    double s[N];
    double x_axis[N];

    for (int i = 0; i < N; i++)
        x_axis[i] = i * (double)SAMPLING_RATE / N;

    h = gnuplot_init();
    // gnuplot_cmd(h, "set yr [-1:1]");
    gnuplot_setstyle(h, "lines");

    input = sound_file_open_read(argv[1]);

    fftw_complex data_in[N], data_out[N];
    fft_init(data_in, data_out);

    double amp[N]/*, phs[N]*/;

    while (sound_file_read(input, s)) {
        fft(s, data_in);
        cartesian_to_polar(data_out, amp);

        gnuplot_resetplot(h);

        // gnuplot_plot_xy(h, x_axis, s, N, "temporal frame");
        // usleep(N / (double)SAMPLING_RATE * 1000000);

        gnuplot_plot_xy(h, x_axis, amp, N / 2, "temporal frame");
        usleep(N / (double)SAMPLING_RATE * 1000000);
    }

    fft_exit();

    sound_file_close_read(input);
    return EXIT_SUCCESS;
}
