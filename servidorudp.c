/*Compilar - gcc servidorudp.c -o servidorudp
  Executar - ./servidorudp
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* close() */
#include <string.h> /* memset() */

#define LOCAL_SERVER_PORT 1500
#define MAX_MSG 200
#define N 12

//Função de Criptografia

void F(char *R, int K, char *resultado, int str_size) {
 int i;
 for(i=0; i<str_size; i++) {
   resultado[i] = R[i] ^ K; 
 }
}


int main(int argc, char *argv[]) {
  
  int i, k;
  int sd, rc, n, cliLen;
  struct sockaddr_in cliAddr, servAddr;
  char msg[MAX_MSG];
  FILE *arquivo_saida, *arquivo_saida2;
	int metade;
	int str_size;
	char L0[MAX_MSG/2];// Esse "+1" corresponde ao Char adicional para identificar fim de String
	char R0[MAX_MSG/2]; // Esse "+1" corresponde ao Char adicional para identificar fim de String
	char temp[MAX_MSG/2];// Variável auxiliar
	char resultado[MAX_MSG/2];
	char msg2[MAX_MSG];
	int chaves[N] = {0xb,0xa,0x9,0x8,0x7,0x6,0x5,0x4,0x3,0x2,0x1,0x0};
					
		



 /* socket creation */
  sd=socket(AF_INET, SOCK_DGRAM, 0);
  if(sd<0) {
    printf("%s: cannot open socket \n",argv[0]);
    exit(1);
  }

  /* bind local server port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(LOCAL_SERVER_PORT);
  rc = bind (sd, (struct sockaddr *) &servAddr,sizeof(servAddr));
  if(rc<0) {
    printf("%s: cannot bind port number %d \n", 
	   argv[0], LOCAL_SERVER_PORT);
    exit(1);
  }

  printf("%s: waiting for data on port UDP %u\n", 
	   argv[0],LOCAL_SERVER_PORT);

  /* server infinite loop */
  while(1) {
    
    /* init buffer */
    memset(msg,0x0,MAX_MSG);


    /* receive message */
    cliLen = sizeof(cliAddr);
    n = recvfrom(sd, msg, MAX_MSG, 0, 
		 (struct sockaddr *) &cliAddr, &cliLen);

    if(n<0) {
      printf("%s: cannot receive data \n",argv[0]);
      continue;
    }
   
 //INICIO DESCRIPTOGRAFIA				
					
 /* init variables */
  memset(resultado, 0x0,MAX_MSG/2);
  memset(msg2, 0x0,MAX_MSG);

  str_size = strlen(msg);

 //divide a string em duas partes
  metade = str_size/2;
  memcpy(L0,msg,metade);
  L0[metade] = '\0'; //Último valor da variável é o símbolo de identificação de término de String 
  memcpy(R0,msg+metade,str_size);
  R0[str_size] = '\0'; //Último valor da variável é o símbolo de identificação de término de String 

 
 /*temp = recebe R normal e resultado recebe R depois da criptografia com a chave*/
for (k=0; k<N; k++) { 
 memcpy(temp, R0, metade);
 F(R0, chaves[k], resultado, metade); 

 /*R recebe resultado XOR L*/
 for (i=0; i<metade;i++) {
   R0[i] = resultado[i] ^ L0[i];
  
 }
 memcpy(L0, temp, metade);
}
  //invertendo as duas metades
  memcpy(msg2,R0,metade);
  memcpy(msg2+metade,L0,str_size);


//gravando arquivo criptografado

arquivo_saida = fopen("descriptografado.txt", "wt");
	if(arquivo_saida == NULL){
		printf("\nErro.");
		exit(1);
}
fprintf(arquivo_saida, "%s", msg2);
fclose(arquivo_saida);


   /* print received message */
    printf("%s: from %s:UDP%u : %s \n", 
	   argv[0],inet_ntoa(cliAddr.sin_addr),
	   ntohs(cliAddr.sin_port),msg2);


  }/* end of server infinite loop */

return 0;

}

