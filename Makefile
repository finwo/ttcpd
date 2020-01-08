CC:=$(shell which gcc tcc | head -1)
PREFIX?=/usr/local

BIN= \
	ttcpd \


SRC=$(patsubst %,src/%.c,$(BIN))

$(BIN): $(SRC)
	$(CC) -O3 -s src/$@.c -o $@

.PHONY: clean
clean:
	rm -rf $(BIN)

.PHONY: install
install: $(BIN)
	install $(BIN) $(PREFIX)/bin
