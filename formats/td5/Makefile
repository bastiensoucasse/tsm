CC := clang
CFLAGS := -I$(HOMEBREW_PATH)/include -O3 -Wall -g
LDFLAGS := -I$(HOMEBREW_PATH)/lib -lsndfile -lvorbis -lvorbisenc -logg -lFLAC -lm -lfftw3

.PHONY: all
all: iantsa bastien

.PHONY: iantsa
iantsa: watermarking_iantsa

.PHONY: bastien
bastien: watermarking_bastien

watermarking_iantsa: watermarking_iantsa.o gnuplot_i.o
watermarking_bastien: watermarking_bastien.o gnuplot_i.o

.PHONY: clean
clean:
	$(RM) *.o watermarking_iantsa watermarking_bastien
