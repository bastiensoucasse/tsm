CC := clang
CFLAGS := -g -O3 -Wall -I.
LDFLAGS := -lm

TARGETS := synthesis

.PHONY: all
all: iantsa bastien

.PHONY: iantsa bastien
iantsa: $(patsubst %, %_iantsa, ${TARGETS})
bastien: $(patsubst %, %_bastien, ${TARGETS})

.PHONY: clean
clean:
	${RM} -r *.dSYM *.o $(patsubst %, %_iantsa, ${TARGETS}) $(patsubst %, %_bastien, ${TARGETS})
