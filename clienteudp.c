/*Compilar - gcc clienteudp.c -o clienteudp
  Executar - ./clienteudp 127.0.0.1 mensagem
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> /* memset() */
#include <sys/time.h> /* select() */

#define REMOTE_SERVER_PORT 1500
#define MAX_MSG 100
#define N 16

/* Funcao F que recebe metade da string e chave da rodada
 * Funcao F nao definida no exercicio. Proposta aqui seria fazer um XOR
 entrada: L, K
 saida: result
*/

void F(char *L, int K, char *result, int str_size) {
 int i;
   //printf("\nAplicando F...");
 for(i=0; i<str_size; i++) {
   result[i] = L[i] ^ K; 
   //printf("\n%02x XOR %02x = %02x", L[i], K, result[i]);
 }
   //printf("\nDone");

}

int main(int argc, char *argv[]) {

  int sd, rc, i,k;
  struct sockaddr_in cliAddr, remoteServAddr;
  struct hostent *h;

  /* check command line args */
  if(argc<3) {
    printf("usage : %s <server> <data1> ... <dataN> \n", argv[0]);
    exit(1);
  }

  /* get server IP address (no check if input is IP address or DNS name */
  h = gethostbyname(argv[1]);
  if(h==NULL) {
    printf("%s: unknown host '%s' \n", argv[0], argv[1]);
    exit(1);
  }

  printf("%s: sending data to '%s' (IP : %s) \n", argv[0], h->h_name,
	 inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));

  remoteServAddr.sin_family = h->h_addrtype;
  memcpy((char *) &remoteServAddr.sin_addr.s_addr,
	 h->h_addr_list[0], h->h_length);
  remoteServAddr.sin_port = htons(REMOTE_SERVER_PORT);

  /* socket creation */
  sd = socket(AF_INET,SOCK_DGRAM,0);
  if(sd<0) {
    printf("%s: cannot open socket \n",argv[0]);
    exit(1);
  }

  /* bind any port */
  cliAddr.sin_family = AF_INET;
  cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  cliAddr.sin_port = htons(0);

  rc = bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
  if(rc<0) {
    printf("%s: cannot bind port\n", argv[0]);
    exit(1);
  }

  /* Variaveis criptograficas */
  int half;
  int str_size;
  char L0[1+ MAX_MSG/2];//caracter adicional para fim de string com \0 
  char L1[1+ MAX_MSG/2];
  char temp[1+ MAX_MSG/2];//variavel auxiliar
  char msg[1+ MAX_MSG];//mensagem a ser enviada
  char result[1+ MAX_MSG/2];
  int chave[N] = {0xa,0x1,0x0,0x9,0x1,0x0,0x0,0x0, 
                  0xe,0x5,0x3,0x0,0x0,0x8,0x0,0x0 };

  /* init variables */
  memset(result, 0x0, 1+ MAX_MSG/2);
  memset(msg, 0x0, 1+ MAX_MSG);

  str_size = strlen(argv[2]);//obtem metade do tamanho da string

  printf("\n Tamanho da mensagem a ser cifrada: %d", str_size);

  printf("\nMensagem pura: %s",argv[2]);

 /* Divide a mensagem em duas partes*/
  half = str_size/2;
  memcpy(L0,argv[2],half); L0[half] = '\0';
  memcpy(L1,argv[2]+half,str_size); L1[str_size] = '\0'; 

  printf("\nL0: %s",L0);
  printf("\nL1: %s",L1);
  printf("\nresult: %s\n",result);

  printf("\nCriptografando rodada 1");

 /* Criptografa a mensagem */
for (k=0; k<N; k++) { 
 memcpy(temp, L1, half);
 F(L1, chave[k], result, half); 

 for (i=0; i<half;i++) {
   L1[i] = result[i] ^ L0[i];
   //printf("\n%02x XOR %02x = %02x", result[i], L0[i], L1[i]);
 }

 memcpy(L0, temp, half);
 printf("\n#####Rodada %d #####",k);

 printf("\nL0: ");
 for (i=0; i<half;i++)
  printf("%02x ",L0[i]);
 printf("\n");

 printf("\nL1: ");
 for (i=0; i<half;i++)
  printf("%02x ",L1[i]);
 printf("\n");
} 

  /* Trocando posicoes da mensagem a ser enviada */
  memcpy(msg,L1,half);
  memcpy(msg+half,L0,str_size);

 printf("\nMensagem cifrada: ");

 printf("\nEnviando mensagem: ");
 for (i=0; i<str_size;i++)
  printf("%02x ",msg[i]);
 printf("\n");

  /* send data */
  for(i=2;i<argc;i++) {
    rc = sendto(sd, msg, str_size, 0,
		(struct sockaddr *) &remoteServAddr,
		sizeof(remoteServAddr));

    if(rc<0) {
      printf("%s: cannot send data %d \n",argv[0],i-1);
      close(sd);
      exit(1);
    }

  }

  return 1;

}

