CC := clang
CFLAGS := -g -O3 -Wall -I.
LDFLAGS := -lm

TARGET := shepard
DEPS := sound_file sinusoid

.PHONY: all
all: iantsa bastien

.PHONY: iantsa bastien
iantsa: ${TARGET}_iantsa
bastien: ${TARGET}_bastien

${TARGET}_iantsa: $(patsubst %, %.o, ${DEPS}) ${TARGET}_iantsa.o
${TARGET}_bastien: $(patsubst %, %.o, ${DEPS}) ${TARGET}_bastien.o

.PHONY: clean
clean:
	${RM} ${TARGET}_iantsa ${TARGET}_bastien *.o
