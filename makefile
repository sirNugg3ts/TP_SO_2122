all: balcao cliente medico

balcao: balcao.c balcao.h
	gcc -o balcao balcao.c

cliente: cliente.c cliente.h
	gcc -o cliente cliente.c

medico: medico.c medico.h
	gcc -o medico medico.c

clean:
	rm -f balcao