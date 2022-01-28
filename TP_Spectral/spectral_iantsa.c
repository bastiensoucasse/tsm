#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <complex.h>
#include <fftw3.h>
#include <time.h>

#include <sndfile.h>

#include <math.h>

#include "gnuplot_i.h"

/* taille de la fenetre */
#define	FRAME_SIZE 1024
/* avancement */
#define HOP_SIZE 1024

static gnuplot_ctrl *h;
static fftw_plan plan;

static void
print_usage (char *progname)
{	printf ("\nUsage : %s <input file> \n", progname) ;
	puts ("\n"
		) ;

} 
static void
fill_buffer(double *buffer, double *new_buffer)
{
  int i;
  double tmp[FRAME_SIZE-HOP_SIZE];
  
  /* save */
  for (i=0;i<FRAME_SIZE-HOP_SIZE;i++)
    tmp[i] = buffer[i+HOP_SIZE];
  
  /* save offset */
  for (i=0;i<(FRAME_SIZE-HOP_SIZE);i++)
    {
      buffer[i] = tmp[i];
    }
  
  for (i=0;i<HOP_SIZE;i++)
    {
      buffer[FRAME_SIZE-HOP_SIZE+i] = new_buffer[i];
    }
}

static int
read_n_samples (SNDFILE * infile, double * buffer, int channels, int n)
{

  if (channels == 1)
    {
      /* MONO */
      int readcount ;

      readcount = sf_readf_double (infile, buffer, n);

      return readcount==n;
    }
  else if (channels == 2)
    {
      /* STEREO */
      double buf [2 * n] ;
      int readcount, k ;
      readcount = sf_readf_double (infile, buf, n);
      for (k = 0 ; k < readcount ; k++)
	buffer[k] = (buf [k * 2]+buf [k * 2+1])/2.0 ;

      return readcount==n;
    }
  else
    {
      /* FORMAT ERROR */
      printf ("Channel format error.\n");
    }
  
  return 0;
} 

static int
read_samples (SNDFILE * infile, double * buffer, int channels)
{
  return read_n_samples (infile, buffer, channels, HOP_SIZE);
}

static void
dft (double s[FRAME_SIZE], double complex S[FRAME_SIZE])
{
	for (int m = 0; m < FRAME_SIZE; m++)
	{
		S[m] = 0;
		for (int n = 0; n < FRAME_SIZE; n++)
			S[m] += s[n] * cexp(-I * 2 * M_PI * n * m / FRAME_SIZE);
	}	
}

static void
cartesian_to_polar (double complex S[FRAME_SIZE], double amp[FRAME_SIZE], double phs[FRAME_SIZE])
{
	for (int n = 0; n < FRAME_SIZE; n++)
	{
		amp[n] = cabs(S[n]);
		phs[n] = carg(S[n]);
	}
}

static void
fft_init(fftw_complex data_in[FRAME_SIZE], fftw_complex data_out[FRAME_SIZE])
{
	plan = fftw_plan_dft_1d(FRAME_SIZE, data_in, data_out, FFTW_FORWARD, FFTW_ESTIMATE);
}

static void
fft(double s[FRAME_SIZE], fftw_complex data_in[FRAME_SIZE])
{
	for (int i = 0; i < FRAME_SIZE; i++)
		data_in[i] = s[i];

	fftw_execute(plan);
}

static void
fft_exit()
{
	fftw_destroy_plan(plan);
}

int
main (int argc, char * argv [])
{	char 		*progname, *infilename;
	SNDFILE	 	*infile = NULL ;
	SF_INFO	 	sfinfo ;

	progname = strrchr (argv [0], '/') ;
	progname = progname ? progname + 1 : argv [0] ;

	if (argc != 2)
	{	print_usage (progname) ;
		return 1 ;
		} ;

	infilename = argv [1] ;

	if ((infile = sf_open (infilename, SFM_READ, &sfinfo)) == NULL)
	{	printf ("Not able to open input file %s.\n", infilename) ;
		puts (sf_strerror (NULL)) ;
		return 1 ;
		} ;

	/* Read WAV */
	int nb_frames = 0;
	double new_buffer[HOP_SIZE];
	double buffer[FRAME_SIZE];

	/* Plot Init */
	h=gnuplot_init();
	gnuplot_setstyle(h, "lines");
	
	int i;
	for (i=0;i<(FRAME_SIZE/HOP_SIZE-1);i++)
	  {
	    if (read_samples (infile, new_buffer, sfinfo.channels)==1)
	      fill_buffer(buffer, new_buffer);
	    else
	      {
		printf("not enough samples !!\n");
		return 1;
	      }
	  }

	/* Info file */
	printf("sample rate %d\n", sfinfo.samplerate);
	printf("channels %d\n", sfinfo.channels);
	printf("size %d\n", (int)sfinfo.frames);

	// DFT
	double complex S[FRAME_SIZE];

	// FFT
	fftw_complex data_in[FRAME_SIZE], data_out[FRAME_SIZE];
	fft_init(data_in, data_out);

    // DFT vs FFT
    clock_t t1, t2;
    double delta_t;
    double delta_t_sum = 0;
	  	
    double amp[FRAME_SIZE], phs[FRAME_SIZE];
	while (read_samples (infile, new_buffer, sfinfo.channels)==1)
	  {
	    /* Process Samples */
	    printf("Processing frame %d\n",nb_frames);

	    /* hop size */
	    fill_buffer(buffer, new_buffer);


	    // DFT
        // t1 = clock();
		// dft (buffer, S);
        // t2 = clock();
        // delta_t = t2 - t1;
        // delta_t_sum += delta_t;
        // printf ("DFT: %f secondes\n", delta_t / CLOCKS_PER_SEC);
		// cartesian_to_polar(S, amp, phs);

		//FFT
        t1 = clock();
		fft(buffer, data_in);
        t2 = clock();
        delta_t = t2 - t1;
        delta_t_sum += delta_t;
        // printf ("FFT: %f secondes\n", delta_t / CLOCKS_PER_SEC);
		//cartesian_to_polar(data_out, amp, phs);

	    /* PLOT */
	    // gnuplot_resetplot(h);
	    // gnuplot_plot_x(h,amp,FRAME_SIZE/2,"temporal frame");
	    // sleep(1);
    
	    nb_frames++;
	  }

      printf ("FFT average: %f secondes\n", delta_t_sum / CLOCKS_PER_SEC);

	sf_close (infile) ;
	fft_exit();

	return 0 ;
} /* main */

