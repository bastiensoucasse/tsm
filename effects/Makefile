CC := gcc
CFLAGS := -fPIC -std=c17 -g -O3 -Wall -I.
LDFLAGS := -lfftw3 -lm

PLUGINS := vocal_remover # amplifier # noise_gate

.PHONY: all
all: iantsa bastien

.PHONY: iantsa bastien
iantsa: ${PLUGINS}_iantsa
bastien: ${PLUGINS}_bastien

.PHONY: ${PLUGINS}_iantsa ${PLUGINS}_bastien
${PLUGINS}_iantsa: %: %.so
${PLUGINS}_bastien: %: %.so

%.so: %.o gnuplot_i.o
	ld -shared ${LDFLAGS} -o $@ $^

gnuplot_i.o: gnuplot_i.c
	${CC} -fPIC -std=c17 -g -O3 -I. -o $@ -c $^

%.o: %.c
	${CC} ${CFLAGS} -o $@ -c $^

.PHONY: clean
clean:
	${RM} *.so *.o
