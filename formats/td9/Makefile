CC := gcc
CFLAGS := -I$(HOMEBREW_PATH)/include -O3 -Wall -g
LDFLAGS := -I$(HOMEBREW_PATH)/lib -lsndfile -lvorbis -lvorbisenc -logg -lFLAC -lfftw3 -lm

.PHONY: all
all: iantsa bastien

.PHONY: iantsa
iantsa: effects_iantsa

.PHONY: bastien
bastien: effects_bastien

effects_iantsa: effects_iantsa.o
effects_bastien: effects_bastien.o

%.so: %.c %.o gnuplot_i.o
	$(CC) -fPIC -c %.c
	ld -shared %.o gnuplot_i.o -lfftw3 -lm -o plugin.so

.PHONY: clean
clean:
	$(RM) *.o effects_iantsa effects_bastien
