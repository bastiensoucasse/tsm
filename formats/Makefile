CFLAGS  := -I$(HOMEBREW_PATH)/include -I. -O3 -Wall
LDFLAGS := -lsndfile -lvorbis -lvorbisenc -logg -lFLAC -lm -lfftw3

.PHONY: all
all: iantsa bastien

.PHONY: iantsa
iantsa: sound_iantsa spectral_iantsa

.PHONY: bastien
bastien: sound_bastien spectral_bastien

sound_iantsa: sound_iantsa.o gnuplot_i.o
sound_bastien: sound_bastien.o gnuplot_i.o

spectral_iantsa: spectral_iantsa.o gnuplot_i.o
spectral_bastien: spectral_bastien.o gnuplot_i.o

.PHONY: clean
clean:
	$(RM) *.o *~ sound_iantsa sound_bastien spectral_iantsa spectral_bastien tmp-in.raw
