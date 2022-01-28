CFLAGS = -O3 -I/opt/homebrew/include -I. -Wall -lm #-pg -g
LDFLAGS = -lsndfile -lvorbis -lvorbisenc -logg -lFLAC -lm -lfftw3

.PHONY: all
all: spectral spectral_iantsa

spectral: gnuplot_i.o spectral.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

spectral_iantsa: gnuplot_i.o spectral_iantsa.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean: 
	@$(RM) *.o *~
