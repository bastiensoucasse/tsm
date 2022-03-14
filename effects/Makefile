CC := gcc
CFLAGS := -fPIC -g -O3 -Wall -I.
LDFLAGS := -lfftw3 -lm

PLUGINS := vocal_remover amplifier noise_gate delay

.PHONY: all
all: iantsa bastien

.PHONY: iantsa bastien
iantsa: $(patsubst %, %_iantsa.so, ${PLUGINS})
bastien: $(patsubst %, %_bastien.so, ${PLUGINS})

%.so: %.o gnuplot_i.o
	ld -shared ${LDFLAGS} -o $@ $^

.PRECIOUS: gnuplot_i.o

%.o: %.c
	${CC} ${CFLAGS} -o $@ -c $^

.PHONY: clean
clean:
	${RM} $(patsubst %, %*.o, ${PLUGINS}) $(patsubst %, %*.so, ${PLUGINS})
