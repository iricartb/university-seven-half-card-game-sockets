#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include "structs.h"
#include <pthread.h>

#define MAXPORT          65535
#define MAXDATASIZE      2000
#define RETURN           10
#define SPACE            32
#define DOT              46
#define RECVOK           "RECVOK"
#define MSGLINE          "_____________________________________________________________________\n"

void * command;
char * token;
char IPGame[16];
unsigned int portGame;
int fd_SimServer;
int exitGame = 0, blk = 0;

int formatIP(char * IP) {

   if (num_tokens_IP(IP) == 4) {
      if (split_IP(IP, 1) == 0) {
         if ((atoi(token) >= 0) && (atoi(token) < 256)) {
            if (split_IP(IP, 2) == 0) {
               if ((atoi(token) >= 0) && (atoi(token) < 256)) {
                  if (split_IP(IP, 3) == 0) {
                     if ((atoi(token) >= 0) && (atoi(token) < 256)) {
                        if (split_IP(IP, 4) == 0) { 
                           if ((atoi(token) >= 0) && (atoi(token) < 256)) { return(0); }
                           else return(-1);   
                        }
                        else return(-1);
                     }
                     else return(-1);   
                  }
                  else return(-1);   
               }
               else return(-1);   
            }
            else return(-1);    
         }
         else return(-1);
      }
      else return(-1);
   }
   return(-1);
}

int split_IP(char *command, int pos) {
  int i=0, j=0, word=0;
  unsigned int lport;
 
  token = malloc(strlen(command));
  strcpy(token, command);

  if ((pos > 0) && (pos <= num_tokens_IP(command))) {
    if ((*(command + i) != DOT) && (*(command + i) != RETURN)  && (*(command + i) != '\x00')) word++;
    while (word != pos) {
      if ((*(command + i) == DOT) &&  (*(command + i + 1) != DOT) && (*(command + i + 1) != '\x00')) word++;
      i++;
    }
    while ((*(command + i) != DOT) && (*(command + i) != RETURN) && (*(command + i) != '\x00'))  {
      *(token + j) = *(command + i);
      i++; j++;
    }
    *(token + j) = '\x00';
    return(0);
  }
  else return(-1);
}

int num_tokens_IP(char *command) {
  int i=0; int word=0;
 
  if ((*(command + i) != DOT) && (*(command + i) != RETURN) && (*(command + i) != '\x00')) { word++; i++; }
  while (*(command + i) != '\x00') {
    if ((*(command + i) == DOT) &&  (*(command + i + 1) != DOT) && (*(command + i + 1) != '\x00')) word++;
    i++;
  }
 
  return(word);
} 

void * SincRecvMsg(void *ptr) {
   char msgRecvSimServer[MAXDATASIZE], numBytesSimServer;
   
   for(;;) {
      numBytesSimServer = recv(fd_SimServer, msgRecvSimServer, MAXDATASIZE, 0);
      if (numBytesSimServer == -1) {
         fprintf(stderr, "\n\t[ ! ] Error en la lectura del socket");
         close(fd_SimServer);
         exit(-5);
      }
      msgRecvSimServer[numBytesSimServer] = '\x00';
      fprintf(stdout, "\n>>> [ SERVIDOR ] "); fprintf(stdout, msgRecvSimServer); fprintf(stdout, "\n");
      if (strcmp(msgRecvSimServer, "VOLS_CARTA") == 0) { fprintf(stdout, "\n>>> [ COMMAND ]: "); blk = 1; }
      send(fd_SimServer, RECVOK, strlen(RECVOK), 0);
   }
}  

int main(int argc, char ** argv) {
   char simReg_IP[16];                         // IP Sim_Reg
   unsigned int simReg_port;                   // Puerto Sim_Reg
   struct hostent * he, * heSim;
   struct sockaddr_in server_SimReg, server_SimServer;
   int fd_SimReg, numBytes, exitProgram = 0, i, iret1, login;
   char msgSend[MAXDATASIZE], msgRecv[MAXDATASIZE];
   pthread_t SincRecv;

   if (argc == 3) {
      strcpy(simReg_IP, argv[1]);
      simReg_port = atoi(argv[2]);
      if ((simReg_port > 0) && (simReg_port <= MAXPORT)) {
         he = gethostbyname(simReg_IP);
         if (he != NULL) {

            fd_SimReg = socket(AF_INET, SOCK_STREAM, 0); 
            if (fd_SimReg != -1) {
               server_SimReg.sin_family = AF_INET;
               server_SimReg.sin_port = htons(simReg_port);
               server_SimReg.sin_addr = *((struct in_addr *) he->h_addr);
               bzero(&(server_SimReg.sin_zero), 8);
             
               for(;exitProgram == 0;) {
                  command = malloc(MAXDATASIZE);
                  fprintf(stdout, "\n");
                  fprintf(stdout, MSGLINE);
                  fprintf(stdout, "\n>>> Llistat de comandes:\n\n\t[ 1 ] OBTENIR_PARTIDES\n\t");
                  fprintf(stdout, "   [ - ] Llistar totes les partides enregistrades al servidor Sim_Reg\n\n\t");		  
                  fprintf(stdout, "[ 2 ] ON_ES_USUARI RENOM\n\t   [ - ] Buscar un usuari en la llista de partides\n\n\t");
                  fprintf(stdout, "[ 3 ] JUGAR_PARTIDA IP PORT\n\t   [ - ] Conectar a un servidor Sim_Server\n\n\t");
                  fprintf(stdout, "[ 4 ] SORTIR\n\t   [ - ] Tanquem la conexio\n");
                  fprintf(stdout, MSGLINE);
                  fprintf(stdout, "\n>>> [ COMMAND ]: ");
                  scanf("%s", (char *) command);
  
                  if (strcmp(command, "OBTENIR_PARTIDES") == 0) {
                     if (connect(fd_SimReg, (struct sockaddr *) &server_SimReg, sizeof(struct sockaddr)) != -1) {
                        sprintf(msgSend, "OBTENIR_PARTIDES");
                        send(fd_SimReg, msgSend, strlen(msgSend), 0);
                        numBytes = recv(fd_SimReg, msgRecv, MAXDATASIZE, 0);
                        if (numBytes == -1) {
                            fprintf(stderr, "\n\t[ ! ] Error en la lectura del socket");
                            close(fd_SimReg);
                            exit(-7);
                         }
                         msgRecv[numBytes] = '\x00';
                         fprintf(stdout, "\n>>> [ "); fprintf(stdout, msgRecv); fprintf(stdout, " ]\n");
                        
                     }
                     else fprintf(stdout, "\n>>> [ ERROR ]: No s'ha pogut conectar amb el Sim_Reg\n");
                  }
                  else if (strcmp(command, "ON_ES_USUARI") == 0) {
                     scanf("%s", (char *) command);
                     if (connect(fd_SimReg, (struct sockaddr *) &server_SimReg, sizeof(struct sockaddr)) != -1) {
                        sprintf(msgSend, "ON_ES_USUARI:%s", command);
                        send(fd_SimReg, msgSend, strlen(msgSend), 0);
                        numBytes = recv(fd_SimReg, msgRecv, MAXDATASIZE, 0);
                        if (numBytes == -1) {
                            fprintf(stderr, "\n\t[ ! ] Error en la lectura del socket");
                            close(fd_SimReg);
                            exit(-6);
                         }
                         msgRecv[numBytes] = '\x00';
                         fprintf(stdout, "\n>>> [ "); fprintf(stdout, msgRecv); fprintf(stdout, " ]\n");                        
                     }
                     else fprintf(stdout, "\n>>> [ ERROR ]: No s'ha pogut conectar amb el Sim_Reg\n");  
                  }
                  else if (strcmp(command, "JUGAR_PARTIDA") == 0) {
                     scanf("%s %d", IPGame, &portGame);
                     
                     if ((formatIP(IPGame) == 0) && (portGame > 0) && (portGame <= MAXPORT)) {
                        heSim = gethostbyname(IPGame);
                        if (heSim != NULL) {

                           fd_SimServer = socket(AF_INET, SOCK_STREAM, 0); 
                           if (fd_SimServer != -1) {
                              server_SimServer.sin_family = AF_INET;
                              server_SimServer.sin_port = htons(portGame);
                              server_SimServer.sin_addr = *((struct in_addr *) heSim->h_addr);
                              bzero(&(server_SimServer.sin_zero), 8);

                              if (connect(fd_SimServer, (struct sockaddr *) &server_SimServer, sizeof(struct sockaddr)) != -1) {
                                 fprintf(stdout, "\n");
                                 fprintf(stdout, MSGLINE);
                                 fprintf(stdout, "\n>>> Llistat de comandes:\n\n\t[ 1 ] SOC RENOM\n\t");
                                 fprintf(stdout, "   [ - ] Ens validem al servidor Sim_Server com un nou usuari\n\n\t");		  
                                 fprintf(stdout, "[ 2 ] NOVA_CARTA\n\t   [ - ] Demanem una nova carta al Sim_Server\n\n\t");
                                 fprintf(stdout, "[ 3 ] EM_PLANTO\n\t   [ - ] Ens plantem amb la nostra ma\n\n\t");
                                 fprintf(stdout, "[ 4 ] ABANDONAR_PARTIDA\n\t   [ - ] Sortim de la partida actual\n");
                                 fprintf(stdout, MSGLINE);
                                 
                                 login = 0;
                                 exitGame = 0; blk = 0;
                                 iret1 = pthread_create(&SincRecv, NULL, SincRecvMsg, NULL);
                                 for(;exitGame == 0;) {
                                    if (login == 0) { fprintf(stdout, "\n>>> [ COMMAND ]: "); scanf("%s", (char *) command); }
                                    else {
                                       for(;blk == 0;);
                                       scanf("%s", (char *) command);
                                       blk = 0;
                                    }                                
                                    if ((strcmp(command, "SOC") == 0) && (login == 0)) {
                                       login = 1;
                                       scanf("%s", (char *) command);
                                       sprintf(msgSend, "SOC:%s", command);
                                       send(fd_SimServer, msgSend, strlen(msgSend), 0);
                                    }
                                    else if ((strcmp(command, "NOVA_CARTA") == 0) && (login == 1)) {
                                       sprintf(msgSend, "NOVA_CARTA");
                                       send(fd_SimServer, msgSend, strlen(msgSend), 0);        
                                    }
                                    else if ((strcmp(command, "EM_PLANTO") == 0) && (login == 1)) {
                                       sprintf(msgSend, "EM_PLANTO");
                                       send(fd_SimServer, msgSend, strlen(msgSend), 0); blk = 1;     
                                    }
                                    else if ((strcmp(command, "ABANDONAR_PARTIDA") == 0) && (login == 1)) {
                                       sprintf(msgSend, "ABANDONAR_PARTIDA");
                                       send(fd_SimServer, msgSend, strlen(msgSend), 0);
                                       pthread_cancel(SincRecv);
                                       exitGame = 1; blk = 1;   
                                    }
                                    else if (login == 0) fprintf(stdout, "\n>>> [ ERROR ]: La primera comanda ha de ser SOC RENOM\n");
                                    else { fprintf(stdout, "\n>>> [ ERROR ]: Comanda incorrecte (NOVA_CARTA, EM_PLANTO, ABANDONAR_PARTIDA)\n"); blk = 1; }
                                 }
                              }
                              else fprintf(stdout, "\n>>> [ ERROR ]: No s'ha pogut conectar amb el Sim_Server -> %s:%d\n", IPGame, portGame);
                           }
                           else fprintf(stdout, "\n>>> [ ERROR ]: En la creacio del socket local\n");
                        }
                        else fprintf(stdout, "\n>>> [ ERROR ]: IP del joc incorrecte -> %s\n", IPGame);                             
                     }
                     else fprintf(stdout, "\n>>> [ ERROR ]: IP o port amb format incorrecte\n");
                  }
                  else if (strcmp(command, "SORTIR") == 0) {
                     exitProgram = 1;
                  }
                  else fprintf(stdout, "\n>>> [ ERROR ]: Comanda no reconeguda\n");
                  fd_SimReg = socket(AF_INET, SOCK_STREAM, 0);
               }
            }
            else {
               fprintf(stderr, "\n------------------------------------------------------------------");
               fprintf(stderr, "\n\t[ ERROR ] Error en el socket");
               fprintf(stderr, "\n------------------------------------------------------------------\n");      
               exit(-4);              
            }
         }
         else { 
            fprintf(stderr, "\n------------------------------------------------------------------");
            fprintf(stderr, "\n\t[ ERROR ] La ip introduida no te el format correcte");
            fprintf(stderr, "\n------------------------------------------------------------------\n");      
            exit(-3); 
         } 
      }
      else {
         fprintf(stderr, "\n------------------------------------------------------------------");
         fprintf(stderr, "\n\t[ ERROR ] El port no pertany al rang [0, 65535]");
         fprintf(stderr, "\n------------------------------------------------------------------\n");      
         exit(-2); 
      }
   }
   else {
      fprintf(stderr, "\n------------------------------------------------------------------");
      fprintf(stderr, "\n\t[ ERROR ] El nombre de parametres es incorrecte");
      fprintf(stderr, "\n\t[ PARAM ] %s IP_sim_reg sim_reg_listening_port", argv[0]);
      fprintf(stderr, "\n\t[ EXAMP ] %s 192.168.1.1 7878", argv[0]);
      fprintf(stderr, "\n------------------------------------------------------------------\n");      
      exit(-1);
   }
}
