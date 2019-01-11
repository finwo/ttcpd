CC:=$(shell which gcc tcc | head -1)

tcpd:
	$(CC) -O3 -s src/main.c -o tcpd
