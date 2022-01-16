all: clean balcao cliente medico

balcao: balcao.c balcao.h
	gcc -o balcao balcao.c -pthread

cliente: cliente.c cliente.h
	gcc -o cliente cliente.c

medico: medico.c medico.h
	gcc -o medico medico.c -pthread

clean:
	rm -f balcao cliente medico
	rm -f /tmp/balc_fifo*
	rm -f /tmp/client_fifo_*
	rm -f /tmp/medico_fifo_*
	rm -f /tmp/balcao_commands