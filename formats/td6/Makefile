CC := clang
CFLAGS := -I$(HOMEBREW_PATH)/include -O3 -Wall -g
LDFLAGS := -I$(HOMEBREW_PATH)/lib -lsndfile -lvorbis -lvorbisenc -logg -lFLAC -lm -lfftw3

.PHONY: all
all: iantsa bastien

.PHONY: iantsa
iantsa: parameters_iantsa

.PHONY: bastien
bastien: parameters_bastien

parameters_iantsa: parameters_iantsa.o gnuplot_i.o
parameters_bastien: parameters_bastien.o gnuplot_i.o

.PHONY: clean
clean:
	$(RM) *.o parameters_iantsa parameters_bastien
