CC := gcc
CFLAGS := -fPIC -std=c17 -g -O3 -Wall -I.
LDFLAGS := -lfftw3 -lm

PLUGINS := vocal_remover # amplifier # noise_gate

.PHONY: all
all: iantsa bastien

.PHONY: iantsa bastien
iantsa: $(patsubst %, %_iantsa.so, ${PLUGINS})
bastien: $(patsubst %, %_bastien.so, ${PLUGINS})

%.so: %.o gnuplot_i.o
	ld -shared ${LDFLAGS} -o $@ $^

%.o: %.c
	${CC} ${CFLAGS} -o $@ -c $^

.PHONY: clean
clean:
	${RM} *.so *.o
