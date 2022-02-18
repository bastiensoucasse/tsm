CC := clang
CFLAGS := -I$(HOMEBREW_PATH)/include -O3 -Wall -g
LDFLAGS := -I$(HOMEBREW_PATH)/lib -lsndfile -lvorbis -lvorbisenc -logg -lFLAC -lm -lfftw3

.PHONY: all
all: iantsa bastien

.PHONY: iantsa
iantsa: phone_iantsa

.PHONY: bastien
bastien: phone_bastien

phone_iantsa: phone_iantsa.o gnuplot_i.o
phone_bastien: phone_bastien.o gnuplot_i.o

sound_iantsa: sound_iantsa.o gnuplot_i.o
sound_bastien: sound_bastien.o gnuplot_i.o

spectral_iantsa: spectral_iantsa.o gnuplot_i.o
spectral_bastien: spectral_bastien.o gnuplot_i.o

.PHONY: clean
clean:
	$(RM) *.o *~ phone_iantsa phone_bastien sound_iantsa sound_bastien spectral_iantsa spectral_bastien tmp-in.raw
