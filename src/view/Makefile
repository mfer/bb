default: serv cli

cli: cli.o
	cc -o cli cli.o

cli.o: cli.c

serv: serv.o                                              
	cc -o serv serv.o

serv.o: serv.c

clean:
	rm -f *.o serv cli
