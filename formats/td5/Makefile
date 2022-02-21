CC := clang
CFLAGS := -I$(HOMEBREW_PATH)/include -O3 -Wall -g
LDFLAGS := -I$(HOMEBREW_PATH)/lib -lsndfile -lvorbis -lvorbisenc -logg -lFLAC -lm -lfftw3

.PHONY: all
all: watermarking

watermarking: watermarking.o gnuplot_i.o

.PHONY: clean
clean:
	$(RM) *.o watermarking
