/*
 * Exemplo de um cliente usando o protocolo TCP.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXBUF 1024

main (argc, argv)
int  argc;
char *argv[];
{
    int sockfd;    /* socket utilizado para a conexao */
    struct sockaddr_in serv_addr; /* endereco do servidor */
    char byte;  /* caracter a ser recebido */
    int rc;     /* numero de caracteres recebidos */
    char *serv_tcp_port; /* porta em que o servidor esta escutando */
    char *serv_tcp_addr; /* endereco IP do servidor */
    char buffer[MAXBUF];


    if (argc < 4) {
        printf("Uso: %s <endereco_ip_do_servidor> <porta_do_servidor> <uri>\n", argv[0]);
        exit(1);
    }
    serv_tcp_addr = argv[1];
    serv_tcp_port = argv[2];

    /*
     * Preenche a estrutura de dados que representa
     * o endereco do servidor.
     */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(serv_tcp_addr);
    serv_addr.sin_port = htons(atoi(serv_tcp_port));

    /* 
     * Cria um socket para conectar com o servidor 
     */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Cliente: erro ao abrir o socket.\n");
        exit(1);
    }

    /* 
     * Conecta com o servidor.
     */
    if (connect(sockfd, (struct sockaddr *) &serv_addr,
                                            sizeof(serv_addr)) < 0) {
        printf("Cliente: nao consegui conectar ao servidor.\n");
        exit(1);
    }
    
    
    /* 
     * envia solicitação ao servidor.
     */
    sprintf(buffer, "GET %s HTTP/1.1\n\n", argv[3]);
    send(sockfd, buffer, strlen(buffer), 0);
    

    /* 
     * Enquanto o servidor estiver enviando alguma
     * coisa, le byte a byte e imprime na tela.
     */
    while((rc = read(sockfd, &byte, 1)) == 1) {
        printf("%c", byte);
    }

    /*
     * Fecha o socket.
     */
    close(sockfd);

    exit(0);
}
