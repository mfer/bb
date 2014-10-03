#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "BigIntegerLibrary.hh"
#include <iostream>
#include <sstream>
#include <fstream>

#define BUF_SIZE 2048

BigInteger mdc(BigInteger n1, BigInteger n2){
    BigInteger resto;
    resto = n1%n2;
    while(resto!=0){
        n1    = n2;
        n2    = resto;
        resto = n1%n2;
    }
    return n2;
}

BigInteger mdc3(BigInteger n1, BigInteger n2, BigInteger n3){
    BigInteger temp = mdc(n1, n2);
    return mdc(temp, n3);
}

BigInteger bigPow(BigInteger base, BigInteger expoent){
    BigInteger i;
    BigInteger resp = 1;
    for(i = 0; i < expoent; i++)
        resp *= base;
    return resp;
}

/*
|  Se A^x +B^y = C^z, onde A, B, C, x, y, e z são inteiros positivos, 
|  com x, y, z > 2, então A, B, e C possuem um fator primo comum
|    'possuir fator primo comum é o mesmo que possuir mdc maior que um'
|
|  bmin e bmax representam os valores mínimo e máximo para as bases (A, B, C) 
|  e pmin e pmax representam os valores mínimos e máximos para os expoentes (x, y, e z).
*/
BigInteger x,y,z,a,b,c;
int break_beal(BigInteger bmin, BigInteger bmax, BigInteger pmin, BigInteger pmax){	
    for(x = pmin; x <= pmax; x++){
    for(y = pmin; y <= pmax; y++){
    for(z = pmin; z <= pmax; z++){
    for(a = bmin; a <= bmax; a++){
    for(b = bmin; b <= bmax; b++){
    for(c = bmin; c <= bmax; c++){
        if( (bigPow(a,x) + bigPow(b,y)) == (bigPow(c,z)) ){
            if(mdc3(a,b,c) == 1){ //para quebrar beal valer, mdc3 deveria ser maior que um
                return 1;
            }
        }
    }}}}}}
    return 0;
}

/*
| realiza maisuma tentativa(s)
|  de enviar msg para sfd 
|  se não obtiver sucesso retorna 0
*/
int
send_msg_and_wait_response(int maisuma, int sfd, char msg[BUF_SIZE])
{
	char buf[BUF_SIZE];	
	int len = strlen(msg) + 1;
	if (len + 1 > BUF_SIZE) {
		fprintf(stderr,
		"Ignoring long message in argument\n");
		exit(1);
	}
	
	int response=0;
    int timeout=1, ready;
    struct timeval tt;
    fd_set fds;
    if (maisuma<0) maisuma=1;
    
	
	do{
		//sending
		if (write(sfd, msg, len) != len) {
			fprintf(stderr, "partial/failed write\n");
			exit(EXIT_FAILURE);
		}
		
		FD_ZERO(&fds);
		FD_SET(sfd,&fds);
		tt.tv_sec=timeout;
		tt.tv_usec=0;
		ready=select(sfd+1,&fds,0,0,&tt);
		
		if(ready<0){
			perror("Select");
		}else{
			//waiting
			if(ready && FD_ISSET(sfd,&fds)){
				ssize_t nread = read(sfd, buf, BUF_SIZE);
				if (nread == -1) {
					perror("read");
					exit(EXIT_FAILURE);
				}else{
					response = 1;
					//printf("Received %ld bytes: %s\n", (long) nread, buf);
					sprintf(msg, buf);
				}				
			}else{
				printf("No reponse, try %d more times.\n",maisuma);
				maisuma--;
			}
		}
	}while(!response && maisuma);
	return maisuma;
}

int
main(int argc, char *argv[])
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;
	char buf[BUF_SIZE];

	if (argc < 3) {
		fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Obtain address(es) matching host/port */

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */

	s = getaddrinfo(argv[1], argv[2], &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.
	Try each address until we successfully connect(2).
	If socket(2) (or connect(2)) fails, we (close the socket
	and) try the next address. */

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,
		rp->ai_protocol);
		if (sfd == -1)
		continue;

		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
		break;                  /* Success */

		close(sfd);
	}

	if (rp == NULL) {               /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);           /* No longer needed */


	int maisuma=5;
	char set[BUF_SIZE];
	char sid[BUF_SIZE];
	while(1 && maisuma){ // from here to eternity
		sprintf(buf,"id");
		if (send_msg_and_wait_response(10,sfd,buf)) {
			if(strstr(buf,"set_")){
				puts("set_: received");
				puts(buf);
				memcpy( set, &buf[4], strlen(buf) );
				set[strlen(buf)-4] = '\0';
			}else{
				break;
			}
			//reenvia para que o servidor saiba que 
			//o cliente recebeu o conjunto correto
			if (send_msg_and_wait_response(10,sfd,buf)){
				if(strstr(buf,"bb")){
					puts("bb: let's break the beal!");
				}else{
					break;
				}
				
				//split set[] in bmin,bmax,pmin and pmax
				std::string str(set, set + strlen(set));
				std::istringstream iss(str);
				std::string token;
				getline(iss, token, ',');
				sprintf(sid,token.c_str());
				getline(iss, token, ',');
				BigInteger bmin = stringToBigInteger(token);
				getline(iss, token, ',');
				BigInteger bmax = stringToBigInteger(token);
				getline(iss, token, ',');
				BigInteger pmin = stringToBigInteger(token);
				getline(iss, token, ',');
				BigInteger pmax = stringToBigInteger(token);

				//realizar contas
				puts("breaking...");
				int ans = break_beal(bmin, bmax, pmin, pmax);
				printf("ans:%d\n",ans);
				
				//ao terminar de calcular enviar a resposta
				sprintf(buf,"ans_%s,%d",sid,ans);
				if(ans){
					strcat(buf,",");
					strcat(buf,bigIntegerToString(a).c_str());
					strcat(buf,",");
					strcat(buf,bigIntegerToString(b).c_str());
					strcat(buf,",");
					strcat(buf,bigIntegerToString(c).c_str());
					strcat(buf,",");
					strcat(buf,bigIntegerToString(x).c_str());
					strcat(buf,",");
					strcat(buf,bigIntegerToString(y).c_str());
					strcat(buf,",");
					strcat(buf,bigIntegerToString(z).c_str());
				}
				if (send_msg_and_wait_response(10,sfd,buf)){
					if(strstr(buf,"wd")){
						puts("wd: well done, let's do it again!");
						puts("---------------------------------");
					}else{
						break;
					}					

				}else{
					puts("failed send ans and wait a response");
					maisuma--;
				}				
			}else{
				puts("failed send set_sid and wait a response");
				maisuma--;
			}				
		}else {
			puts("failed send id and wait a response");
			maisuma--;
		}

		/* Send remaining command-line arguments as separate
		datagrams, and read responses from server */
		/*
		inj;
		for (j = 3; j < argc; j++) {
			if (send_msg_and_wait_response(10,sfd,argv[j])) 
				puts("success");
			else 
				puts("failed");
		}
		*/
	}
	exit(EXIT_SUCCESS);
}
