CC?=cc
PREFIX?=/usr/local
CFLAGS?=-O2 -s

BIN= \
	ttcpd \

OBJ=$(patsubst %,src/%.o,$(BIN))

LIBS?=

default: $(BIN)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN): $(OBJ)
	$(CC) -o $@ src/$@.o $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -rf $(BIN)
	rm -rf $(OBJ)

.PHONY: install
install: $(BIN)
	install $(BIN) $(PREFIX)/bin

