#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "BigIntegerLibrary.hh"
#include <set>
#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>

#define BUF_SIZE 2048
#define BMININO 3
#define PMININO 3
#define CLIENTS 30 //para ajudar a decidir se um set deve sair de doing

BigInteger binc=5;
BigInteger pinc=5;

//TODO: ler do arquivo de persistência os valores já calculados 
//	para não haver repetições a cada vez que o servidor reinicia...

BigInteger sid=0; //set_id

BigInteger nti=0;
BigInteger bti=2;
BigInteger pti=0;
BigInteger hti=0;

BigInteger nts=0;
BigInteger bts=0;
BigInteger pts=1;
BigInteger hts=0;

std::set<BigInteger> todo; 
std::set<BigInteger> doing; 
std::set< std::vector<BigInteger> > conjunto;

void
triangulo_inferior(BigInteger *b, BigInteger *p){
	pti++;
		
	nti = hti + pti;
	*b = bti;
	*p = pti;
	
	if( pti == (bti-1) ){
		pti = 0;
		hti = nti;
		bti++;
	}
}

void
triangulo_superior(BigInteger *b, BigInteger *p){
	bts++;
		
	nts = hts + bts;
	*b = bts;
	*p = pts;
	
	if( bts == pts ){
		bts = 0;
		hts = nts;
		pts++;
	}
}

/*
| get_set auxilia no controle do espaço de busca
| a estrategia adotada subdivide o quadrante bXp
| em dois triangulos: superior e inferior
*/
void
get_set(char tmp[BUF_SIZE]){
	BigInteger b, p;

	sid++;
	if((sid%2).toInt()){
		triangulo_superior(&b, &p);
	}else{
		triangulo_inferior(&b, &p);
	}

	std::vector<BigInteger> v0 = std::vector<BigInteger>(5);
	v0[0]=sid;
	v0[1]=(b-1)*binc+BMININO;
	v0[2]=b*binc+BMININO-1;
	v0[3]=(p-1)*pinc+PMININO;
	v0[4]=p*pinc+PMININO-1;
	conjunto.insert(v0);
			
    std::string ssid = bigIntegerToString(v0[0]);
    std::string bmin = bigIntegerToString(v0[1]);
	std::string bmax = bigIntegerToString(v0[2]);
	std::string pmin = bigIntegerToString(v0[3]);
	std::string pmax = bigIntegerToString(v0[4]);
		
	sprintf(tmp,"set_%s,%s,%s,%s,%s",ssid.c_str(),bmin.c_str(),
		bmax.c_str(),pmin.c_str(),pmax.c_str());
}

void
update_storage(){
	// o arquivo possui
	// 1) sid:BigInt
	// 2) todo:BigInt,BigInt,BigInt,...,BigInt,
	// 3) doing:BigInt,BigInt,BigInt,...,BigInt,
	// 4) conjunto:SIZE
	// n) sid:bmin bmax pmin pmax

/*
	std::cout << "-->\nsid:" << sid << '\n';
	std::set<BigInteger>::iterator it;
	std::cout << "todo:";
	for (it=todo.begin(); it!=todo.end(); ++it)
		std::cout  << *it << ',';
	std::cout << '\n';
	std::cout << "doing:";
	for (it=doing.begin(); it!=doing.end(); ++it)
		std::cout  << *it << ',';
	std::cout << '\n';
	
	std::cout << "conjunto:" << conjunto.size() << "\n";
	std::set< std::vector<BigInteger> >::iterator itv;
	for( itv = conjunto.begin(); itv != conjunto.end(); itv++) {
		const std::vector<BigInteger>& i = (*itv); // HERE we get the vector
		std::cout << i[0] << ":" << i[1] << " " << i[2] << " " << i[3] << " " << i[4] << "\n";
	}
*/
	
	std::ofstream myfile;
	myfile.open ("view/htdocs/index.html");
	myfile << "<!DOCTYPE html><html><head><title>Break Beal</title><meta http-equiv=\"refresh\" content=\"1\" ></head>";
	myfile << "<body style=\"text-align: center;\"><h1>Break Beal Results</h1>";
	
	myfile << "<p>" << "sid:" << sid << "</p>";
	
	std::set<BigInteger>::iterator it;
	myfile << "<p>" << "todo:";
	for (it=todo.begin(); it!=todo.end(); ++it)
		myfile  << *it << ',';
	myfile << "</p>";
	
	myfile << "<p>" << "doing:";
	for (it=doing.begin(); it!=doing.end(); ++it){
		myfile  << *it << ',';
//TODO: colcoar um timestamp em cada set e compará-lo com a data-hora atual
		//gerindo de forma heurśtica a existência de sets muito antigos no doing
		if ((sid - *it) > CLIENTS ){
			//adicionar na lista todo
			todo.insert(*it);
			//apagar da lista doing
			doing.erase(*it);
		}
	}
	myfile << "</p>";
	
	myfile << "<p>" << "conjunto:" << conjunto.size() << "</p>";
	std::set< std::vector<BigInteger> >::iterator itv;
	for( itv = conjunto.begin(); itv != conjunto.end(); itv++) {
		const std::vector<BigInteger>& i = (*itv); // HERE we get the vector
		myfile << "<p>" << i[0] << ":" << i[1] << " " << i[2] << " " << i[3] << " " << i[4] << "</p>";
	}
	
	myfile << "<p>Values that break Beal (A B C x y z):</p><iframe src=\"brokeit.txt\"></iframe></body></html>\n";
	myfile.close();
}

void
produce_response(char buf[BUF_SIZE], char res[BUF_SIZE]){	

	if(strstr(buf,"id")){		
		memset(res, 0, BUF_SIZE);
		
		update_storage();
		
		if(todo.empty()){ 
			//obtem novo set e o adiciona a conjunto
			get_set(res);
			//adicionar sid a lista todo
			todo.insert(sid);
		}else{
			//obtem o primeiro set da lista todo
			std::cout << ' ' << *todo.begin();
			std::string ssid = bigIntegerToString(*todo.begin());
			
			//busca os valores desse set em conjunto
			std::set< std::vector<BigInteger> >::iterator itv;
			for( itv = conjunto.begin(); itv != conjunto.end(); itv++) {
				const std::vector<BigInteger>& i = (*itv); // HERE we get the vector
				if (*todo.begin()==i[0]){
					std::string ssid = bigIntegerToString(i[0]);
					std::string bmin = bigIntegerToString(i[1]);
					std::string bmax = bigIntegerToString(i[2]);
					std::string pmin = bigIntegerToString(i[3]);
					std::string pmax = bigIntegerToString(i[4]);
					
					sprintf(res,"set_%s,%s,%s,%s,%s",ssid.c_str(),bmin.c_str(),
						bmax.c_str(),pmin.c_str(),pmax.c_str());
					break;
				}
			}
		}		
	}else if(strstr(buf,"set_")){
		memset(res, 0, BUF_SIZE);

		//retirando 'set_' de buf e colocando o resto em subbuf
		char subbuf[BUF_SIZE];
		memcpy( subbuf, &buf[4], strlen(buf) );
		subbuf[strlen(buf)-4] = '\0';
		//puts(subbuf);
		
		//split subbuf[] in bmin,bmax,pmin and pmax
		std::string str(subbuf, subbuf + strlen(subbuf));
		std::istringstream iss(str);
		std::string token;
		
		getline(iss, token, ',');
		puts("<--");
		puts(token.c_str());
		BigInteger sidr = stringToBigInteger(token); //sid received
		getline(iss, token, ',');
		BigInteger bminr = stringToBigInteger(token);
		getline(iss, token, ',');
		BigInteger bmaxr = stringToBigInteger(token);
		getline(iss, token, ',');
		BigInteger pminr = stringToBigInteger(token);
		getline(iss, token, ',');
		BigInteger pmaxr = stringToBigInteger(token);

		//busca os valores desse set, identificado por sidr, em conjunto
		std::set< std::vector<BigInteger> >::iterator itv;
		for( itv = conjunto.begin(); itv != conjunto.end(); itv++) {
			const std::vector<BigInteger>& i = (*itv); // HERE we get the vector
			if (sidr==i[0]){
				if (i[0]==sidr && i[1]==bminr && i[2]==bmaxr && i[3]==pminr && i[4]==pmaxr){
					//enviar autorizacao para break beal (bb)!
					sprintf(res,"bb");
					
					//adicionar sidr na lista doing
					doing.insert(sidr);
					//apagar sidr da lista todo
					todo.erase(sidr);						
				}

				break;
			}
		}

	}else if(strstr(buf,"ans_")){
		memset(res, 0, BUF_SIZE);
		//enviar congratulações de well done (wd)!
		sprintf(res,"wd");
		
		//retirando 'ans_' de buf e colocando o resto em subbuf
		char subbuf[BUF_SIZE];
		memcpy( subbuf, &buf[4], strlen(buf) );
		subbuf[strlen(buf)-4] = '\0';
		//puts(subbuf);
		
		//split subbuf[]
		std::string str(subbuf, subbuf + strlen(subbuf));
		std::istringstream iss(str);
		std::string token;
		
		getline(iss, token, ',');
		puts("<--");
		puts(token.c_str());
		BigInteger sidr = stringToBigInteger(token); //sid received
		getline(iss, token, ',');
		printf("ans:%s\n",token.c_str());
		if(token[0] == '1'){ //1 implica em obter a,b,c,x,y,z
			std::ofstream ofs;
			ofs.open ("view/htdocs/brokeit.txt", std::ofstream::out | std::ofstream::app);
			
			getline(iss, token, ',');
			//BigInteger a = stringToBigInteger(token);
			ofs << token << ' ';
			getline(iss, token, ',');
			//BigInteger b = stringToBigInteger(token);
			ofs << token << ' ';
			getline(iss, token, ',');
			//BigInteger c = stringToBigInteger(token);
			ofs << token << ' ';
			getline(iss, token, ',');
			//BigInteger x = stringToBigInteger(token);
			ofs << token << ' ';
			getline(iss, token, ',');			
			//BigInteger y = stringToBigInteger(token);
			ofs << token << ' ';
			getline(iss, token, ',');
			//BigInteger z = stringToBigInteger(token);
			ofs << token << "</br>";
			
			ofs.close();

		}
		
		//apagar sidr da lista doing
		doing.erase(sidr);
		
		//apagar sidr do conjunto
		//busca o iterador do conjunto a set apagado
		std::set< std::vector<BigInteger> >::iterator itv;
		for( itv = conjunto.begin(); itv != conjunto.end(); itv++) {
			const std::vector<BigInteger>& i = (*itv); // HERE we get the vector
			if (sidr==i[0]){
				conjunto.erase(i);
				break;
			}
		}
		//conjunto.erase(sidr);
	}
}

int
main(int argc, char *argv[])
{
    fd_set readSet;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;
	ssize_t nread;
	char buf[BUF_SIZE], res[BUF_SIZE];
	if (argc != 2) {
		fprintf(stderr, "Usage: %s port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	s = getaddrinfo(NULL, argv[1], &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.
	Try each address until we successfully bind(2).
	If socket(2) (or bind(2)) fails, we (close the socket
	and) try the next address. */

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;                  /* Success */

		close(sfd);
	}

	if (rp == NULL) {               /* No address succeeded */
		fprintf(stderr, "Could not bind\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);           /* No longer needed */
	
	FD_ZERO(&readSet);
	/* Read datagrams and give a correctly response to sender */
	for (;;) { //until the end...
		FD_SET(sfd, &readSet);
		if(select(sfd + 1, &readSet, NULL, NULL, NULL) < 0) {
			perror("Select failed");
			exit(1);
		}
		if(FD_ISSET(sfd, &readSet)) {
			peer_addr_len = sizeof(struct sockaddr_storage);
			nread = recvfrom(sfd, buf, BUF_SIZE, 0,
				(struct sockaddr *) &peer_addr, &peer_addr_len);
			if (nread == -1)
				continue;               /* Ignore failed request */

			char host[NI_MAXHOST], service[NI_MAXSERV];

			s = getnameinfo((struct sockaddr *) &peer_addr,	peer_addr_len, 
				host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
				
			if (s == 0){
				//printf("Received %ld bytes from %s:%s\n", (long) nread, 
				//	host, service);
				
				produce_response(buf,res);			

				int res_len = strlen(res)+1;
				if (sendto(sfd, res, res_len, 0,(struct sockaddr *) &peer_addr,
					peer_addr_len) != res_len)
					fprintf(stderr, "Error sending response\n");

			}else{
				fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
			}
		}
	}
}
