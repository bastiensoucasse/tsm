#include "gnuplot_i.h"
#include "math.h"
#include <complex.h>
#include <fftw3.h>

#define SAMPLING_RATE 44100.0
#define CHANNELS_NUMBER 1
#define N 1024

char *RAW_FILE = "tmp-in.raw";
static fftw_plan plan;

FILE *
sound_file_open_read (char *sound_file_name)
{
  char cmd[256];

  snprintf (cmd, 256,
	    "sox %s -b 16 -c %d -r %d -e signed-integer %s",
	    sound_file_name,
	    CHANNELS_NUMBER, (int)SAMPLING_RATE, RAW_FILE);

  system (cmd);

  return fopen (RAW_FILE, "rb");
}

void
sound_file_close_read (FILE *fp)
{
  fclose (fp);
}

int
sound_file_read (FILE *fp, double *s)
{
  short tmp[N];
  int nb_read = fread (tmp, 2, N, fp);
  for (int i = 0; i < N; i++)
    s[i] = tmp[i] * 1.0 / (pow(2, 15));

  return nb_read == N;
}

static void
cartesian_to_polar (double complex S[N], double amp[N])
{
	for (int n = 0; n < N; n++)
		amp[n] = cabs(S[n]) / N * 2;
}

static void
fft_init(fftw_complex data_in[N], fftw_complex data_out[N])
{
	plan = fftw_plan_dft_1d(N, data_in, data_out, FFTW_FORWARD, FFTW_ESTIMATE);
}

static void
fft(double s[N], fftw_complex data_in[N])
{
	for (int i = 0; i < N; i++)
		data_in[i] = s[i];

	fftw_execute(plan);
}

static void
fft_exit()
{
	fftw_destroy_plan(plan);
}

int
main (int argc, char *argv[])
{
  FILE *input;
  double s[N];
  double x_axis[N];

  for (int i = 0; i < N; i++)
    // Temporal
    // x_axis[i] = i / SAMPLING_RATE;

    // Frequential
    x_axis[i] = i * (SAMPLING_RATE / N);

  gnuplot_ctrl *h = gnuplot_init();
  // Temporal
  // gnuplot_cmd(h, "set yr [-1:1]");
  gnuplot_setstyle(h, "lines");
  
  input = sound_file_open_read(argv[1]);

  fftw_complex data_in[N], data_out[N];
  fft_init(data_in, data_out);
  
  double amp[N];

  while(sound_file_read (input, s))
    {
    
    // Temporal
    //   for (int i = 0; i < N; i++)
    //     x_axis[i] += N/SAMPLING_RATE;

      fft(s, data_in);
      cartesian_to_polar(data_out, amp);

      // affichage
      // gnuplot_cmd(h, "set yr [-1:1]");
      gnuplot_resetplot(h);

      /* affichage : A COMPLETER */
      // gnuplot_plot_xy(h, x_axis, s, N, "temporal frame");
      // usleep(N/SAMPLING_RATE * 1000000);

      gnuplot_plot_xy(h, x_axis, amp, N/2, "temporal frame");
      usleep(N / (double)SAMPLING_RATE * 1000000);
    }

  fft_exit();
  
  sound_file_close_read (input);
  exit (EXIT_SUCCESS);
}
