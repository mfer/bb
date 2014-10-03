/* 
 * Exemplo de um servidor usando o protocolo TCP.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#define ISspace(x) isspace((int)(x))
#define SERVER_STRING "Server: 127.0.0.1\r\n"

void cat(int client, FILE *resource){
	char buf[1024];

	fgets(buf, sizeof(buf), resource);
	while (!feof(resource)){
		send(client, buf, strlen(buf), 0);
		fgets(buf, sizeof(buf), resource);
	}
}

void not_found(int client){
	char buf[1024];

	sprintf(buf, "HTTP/1.1 404 NOT FOUND\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>The server could not fufill\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "your request because the resource specified\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "is unavailable or nonexistent.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}

void headers(int client, const char *filename){
	char buf[1024];
	(void)filename;  /* could use filename to determine file type */

	strcpy(buf, "HTTP/1.1 200 OK\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
}

void serve_file(int client, const char *filename){
	FILE *resource = NULL;
	int numchars = 1;
	char buf[1024];

	buf[0] = 'A'; buf[1] = '\0';
	while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
		numchars = get_line(client, buf, sizeof(buf));

	resource = fopen(filename, "r");
	if (resource == NULL)
		not_found(client);
	else
	{
		headers(client, filename);
		cat(client, resource);
	}
	fclose(resource);
}

void unimplemented(int client)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.1 501 Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><HEAD><TITLE>simple html - not implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</TITLE></HEAD>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}

int get_line(int sock, char *buf, int size){
	int i = 0;
	char c = '\0';
	int n;

	while ((i < size - 1) && (c != '\n')){
		n = recv(sock, &c, 1, 0);
		printf("%c",c);
		if (n > 0){
			if (c == '\r'){
				n = recv(sock, &c, 1, MSG_PEEK);
				printf("%c",c);
				if ((n > 0) && (c == '\n'))
					recv(sock, &c, 1, 0);
				else
					c = '\n';
			}
			buf[i] = c;
			i++;
		}
		else
		c = '\n';
	}
	buf[i] = '\0';

	return(i);
}

void accept_request(int client){
	char buf[1024]={};
	int numchars;
	char method[255];
	char url[255];
	char path[512];
	size_t i, j;
	struct stat st;

	numchars = get_line(client, buf, sizeof(buf));

	if(numchars!=0){ //cliente nao envoi pedidos
		i = 0; j = 0;
		while (!ISspace(buf[j]) && (i < sizeof(method) - 1)){
			method[i] = buf[j];
			i++; j++;
		}
		method[i] = '\0';

		if (strcasecmp(method, "GET")){
			unimplemented(client);
			return;
		}

		i = 0;
		while (ISspace(buf[j]) && (j < sizeof(buf)))
			j++;
			
		while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf))){
			url[i] = buf[j];
			i++; j++;
		}
		url[i] = '\0';


		sprintf(path, "htdocs%s", url);
		if (path[strlen(path) - 1] == '/')
			strcat(path, "index.html");

		if (stat(path, &st) == -1) {
			while ((numchars > 0) && strcmp("\n", buf))
				numchars = get_line(client, buf, sizeof(buf));
			not_found(client);
		}else{
			if ((st.st_mode & S_IFMT) == S_IFDIR)
				strcat(path, "/index.html");
			serve_file(client, path);
		}
		close(client);

	}
}

void timestamp(){
    time_t ltime; /* calendar time */
    ltime=time(NULL); /* get current cal time */
    printf("%s",asctime( localtime(&ltime) ) );
}

main (int argc, char *argv[]){
    int sockfd,      /* socket do servidor */ 
        newsockfd,   /* socket utilizado quando um cliente se conecta */        
        clilen;

    struct sockaddr_in cli_addr,  /* Informacoes de endereco do cliente */ 
                       serv_addr; /* Informacoes de endereco do servidor */

    char byte;       /* Caracter a ser enviado */

    char *serv_tcp_port;  /* Porto em o servidor ficara escutando */

    if (argc < 2) {
        printf("Uso: %s <porto_do_servidor>\n", argv[0]);
        exit(1);
    }

    serv_tcp_port = argv[1];

    /*
     * Cria um socket que utiliza protocolos do tipo
     * AF_INET (protocolos da internet), e o socket e 
     * do tipo SOCK_STREAM (stream socket). Retorna
     * um inteiro que e o descritor do socket criado.
     */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Servidor: erro ao criar socket.\n");
        exit(1);
    }

    /*
     * Preenche a estrutura de dados que 
     * representa o endereco do servidor.
     */

    /* 
     * Tipo de endereco, neste caso, enderecos
     * utilizados na internet
     */
    serv_addr.sin_family = AF_INET;

    /* 
     * Endereco em que o servidor executara. 
     * Neste caso, pode executar em qualquer maquina.
     */
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 

    /* 
     * Porto em que o servidor ficara escutando 
     */ 
    serv_addr.sin_port = htons(atoi(serv_tcp_port));


    /*
     * Associa o socket ao endereco dado 
     * pela estrutura preenchida acima.
     */ 
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("Servidor: erro ao dar o bind no endereco local.\n");
        exit(1);
    }
    

    /* 
     * Fica escutando no porto escolhido.
     * O segundo parametro e numero de clientes
     * que podem ficar esperando enquanto o servidor
     * processa o pedido de algum cliente.
     */
    listen(sockfd, 5);
    printf("Servidor inicializado.\n");

    /* 
     * Entra em um loop infinito e
     * fica esperando um cliente se conectar.
     */
    for (;;) 
    {
        clilen = sizeof(cli_addr);

        timestamp();

        /*
         * Passa a aceitar clientes, e os dados
         * do cliente que se conectar serao armazenados
         * na estrutura cli_addr. O accept retorna um novo
         * socket que sera utilizado pelo servidor para
         * conversar com o cliente.
         */
        printf("Esperando por conexoes...\n\n");
        
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        printf("Servidor: Recebi uma conexao.\n");

        if (newsockfd < 0) {
             printf("Servidor: erro no accept.\n");
             exit(1);
        }
        
        accept_request(newsockfd); 
        printf("Servidor: Caracteres enviados.\n");

        /*
         * Fecha o socket utilizado para conversar
         * com o cliente.
         */
        close(newsockfd);
        printf("Servidor: Conexao finalizada.\n\n");
    }

}
