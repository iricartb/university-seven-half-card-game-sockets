#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include "structs.h"

#define RETURN            10
#define SPACE             32
#define MAXCONNECTIONS    5
#define DOT               46
#define MAXPLAYERS        512
#define MAXGAMES          512
#define MAXDATASIZE       2000
#define MAXPORT           65535
#define MSGGAMEUPOK       "CORRECTE_ALTA_PARTIDA"
#define MSGGAMEUPFAIL     "FORMAT_INCORRECTE_ALTA_PARTIDA"
#define MSGGAMEDOWNOK     "CORRECTE_BAIXA_PARTIDA"
#define MSGGAMEDOWNERR    "NO_HI_HA_PARTIDA"
#define MSGGAMEDOWNFAIL   "FORMAT_INCORRECTE_BAIXA_PARTIDA"
#define MSGPLAYERUPOK     "CORRECTE_ALTA_USUARI"
#define MSGPLAYERUPFAIL   "FORMAT_INCORRECTE_ALTA_USUARI"
#define MSGPLAYERDOWNOK   "CORRECTE_BAIXA_USUARI"
#define MSGPLAYERDOWNERR  "NO_HI_HA_USUARI"
#define MSGPLAYERDOWNFAIL "FORMAT_INCORRECTE_BAIXA_USUARI"
#define MSGEMPTYGAMES     "NO_HI_HA_PARTIDES"
#define MSGEMPTYPLAYERS   "NO_HI_HA_JUGADORS"
#define MSGPLAYERSFAIL    "FORMAT_INCORRECTE_ON_HI_HA_USUARI"
#define MSGNOEXPLAYER     "NO_EXISTEIX_EL_JUGADOR"

void * command;
char * token, * IP, * port, * userCmd, * nickname;
unsigned int counterGames = 0, counterPlayers = 0;

Game tGame[MAXGAMES];
Player tPlayer[MAXPLAYERS]; 

/* ############################################## [ STRING FUNCTIONS ] ############################################## */

int split(char *command, int pos) {
  int i=0, j=0, word=0;
  
  token = malloc(strlen(command));
  strcpy(token, command);

  if ((pos > 0) && (pos <= num_tokens(command))) {
    if ((*(command + i) != SPACE) && (*(command + i) != RETURN)  && (*(command + i) != '\x00')) word++;
    while (word != pos) {
      if ((*(command + i) == SPACE) &&  (*(command + i + 1) != SPACE) && (*(command + i + 1) != '\x00')) word++;
      i++;
    }
    while ((*(command + i) != SPACE) && (*(command + i) != RETURN) && (*(command + i) != '\x00'))  {
      *(token + j) = *(command + i);
      i++; j++;
    }
    *(token + j) = '\x00';
    return(0);
  }
  else return(-1);
}

int get_IP(char *command) {
   int i=0, j=0;
   
   IP = malloc(strlen(command));

   for(;((*(command + i) != ':') && (*(command + i) != RETURN) && (*(command + i) != '\x00')); i++) { *(IP + j) = *(command + i); j++; }
   *(IP + j) = '\x00';
   
   if ((*(IP + j - 1) == RETURN) || (*(IP + j - 1) == '\x00')) return(-1);
   else return(0);
}

int get_userCmd(char *command) {
   int i=0, j=0;
   
   userCmd = malloc(strlen(command));

   for(;((*(command + i) != ':') && (*(command + i) != RETURN) && (*(command + i) != '\x00')); i++) { *(userCmd + j) = *(command + i); j++; }
   *(userCmd + j) = '\x00';
   
   if ((*(userCmd + j - 1) == RETURN) || (*(userCmd + j - 1) == '\x00')) return(-1);
   else return(0);
}

int get_port(char *command) {
   int i=0, j=0;
   
   port = malloc(strlen(command));
   for(;((*(command + i) != ':') && (*(command + i) != RETURN) && (*(command + i) != '\x00')); i++);

   if ((*(command + i) == RETURN) || (*(command + i) == '\x00')) return(-1);
   else {
      i++;
      for(;*(command + i) != '\x00'; j++, i++) *(port + j) = *(command + i);
      *(port + j) = '\x00';
      return(0); 
   } 
}

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

int num_tokens(char *command) {
  int i=0; int word=0;
 
  if ((*(command + i) != SPACE) && (*(command + i) != RETURN) && (*(command + i) != '\x00')) { word++; i++; }
  while (*(command + i) != '\x00') {
    if ((*(command + i) == SPACE) &&  (*(command + i + 1) != SPACE) && (*(command + i + 1) != '\x00')) word++;
    i++;
  }
 
  return(word);
}


int clean_cmd(char *command) {
  int i=0, j=0;

  while (*(command + i) == SPACE) i++;
  while (*(command + i) != '\x00') {
    *(command + j) = *(command + i);
    i++; j++;
  }
  *(command + j) = '\x00';
  return(0);
}

int searchGame(char * IP, unsigned int port) {
   int i;
   for(i = 0; i < counterGames; i++) {
      if ((strcmp(tGame[i].IPaddress, IP) == 0) && (tGame[i].port == port)) return(i); 
   }
   return(-1);   
}

void eraseGame(int pos) {
  
   if (pos != counterGames - 1) {
      strcpy(tGame[pos].IPaddress, tGame[counterGames - 1].IPaddress);
      tGame[pos].port = tGame[counterGames - 1].port;
   }
   counterGames--;
}

int searchPlayer(char * IP, unsigned int port) {
   int i;
   for(i = 0; i < counterPlayers; i++) {
      if ((strcmp(tPlayer[i].game.IPaddress, IP) == 0) && (tPlayer[i].game.port == port)) return(i); 
   }
   return(-1);
}

int searchPlayer2(char * IP, unsigned int port, char * nickname) {
   int i;
   for(i = 0; i < counterPlayers; i++) {
      if ((strcmp(tPlayer[i].game.IPaddress, IP) == 0) && (tPlayer[i].game.port == port) && (strcmp(tPlayer[i].nickname, nickname) == 0)) return(i); 
   }
   return(-1);
}

int searchPlayer3(char * nick) {
   int i;
   for(i = 0; i < counterPlayers; i++) {
      if (strcmp(tPlayer[i].nickname, nick) == 0) return(i); 
   }
   return(-1);
}

void erasePlayer(int pos) {
  
   if (pos != counterPlayers - 1) {
      strcpy(tPlayer[pos].game.IPaddress, tPlayer[counterPlayers - 1].game.IPaddress);
      tPlayer[pos].game.port = tPlayer[counterPlayers - 1].game.port;
   }
   counterPlayers--;
}

unsigned int findPort(char * IP) {
   int i;
   for(i=0; i < counterGames; i++) {
      if (strcmp(tGame[i].IPaddress, IP) == 0) return(tGame[i].port); 
   }
   return(-1);
}

/* ################################################################################################################## */

int main(int argc, char ** argv) {
   unsigned int lport, erasePort, fport;   
   int fd, fd2, numBytes, i;
   struct sockaddr_in server;
   struct sockaddr_in client;
   int sin_size, posGame, posPlayer;  
   char * eraseNick, * pmsgSend;
   char msgSend[MAXDATASIZE];
   char msgPort[6];

   if (argc == 2) {
      lport = atoi(argv[1]);
      if ((lport > 1024) && (lport <= 65535)) {
         if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            fprintf(stderr, "\n\t[ ! ] Error en el socket");
            exit(-3);
         }
           
         /* Configuramos parametros del servidor */
         server.sin_family = AF_INET;
         server.sin_port = htons(lport);
         server.sin_addr.s_addr = INADDR_ANY;
         bzero(&(server.sin_zero), 8); 

         /* Creamos el canal */
         if (bind(fd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
            fprintf(stderr, "\n\t[ ! ] Error en el bind");
            exit(-4);
         }

         if (listen(fd, MAXCONNECTIONS) == -1) {
            fprintf(stderr, "\n\t[ ! ] Error en listen");
            exit(-5);
         }
                 
         while(1) {
            sin_size = sizeof(struct sockaddr_in);

            if ((fd2 = accept(fd, (struct sockaddr *) &client, &sin_size)) == -1) {
               fprintf(stderr, "\n\t[ ! ] Error en listen");
               close(fd2);
               exit(-6);
            }

            command = malloc(MAXDATASIZE);
            numBytes = recv(fd2, command, MAXDATASIZE, 0);
            if (numBytes == -1) {
               fprintf(stderr, "\n\t[ ! ] Error al llegir - socket");
               close(fd2);
               exit(-7);
            }
            *(((char *) command) + numBytes) = '\x00';

            if (split(command, 1) == 0) {
               if (strcmp(token, "OBRIR_PARTIDA") == 0) {         
                  if (split(command, 2) == 0) {
                     if ((get_IP(token) == 0) && (get_port(token) == 0)) {
                        if ((formatIP(IP) == 0) && (atoi(port) > 0) && (atoi(port) <= MAXPORT)) {
                           strcpy(tGame[counterGames].IPaddress, IP);
                           tGame[counterGames].port = atoi(port);
                           counterGames++;
                           send(fd2, MSGGAMEUPOK, strlen(MSGGAMEUPOK), 0);   
                        }
                        else send(fd2, MSGGAMEUPFAIL, strlen(MSGGAMEUPFAIL), 0);        
                     }
                     else send(fd2, MSGGAMEUPFAIL, strlen(MSGGAMEUPFAIL), 0);
                  }
                  else send(fd2, MSGGAMEUPFAIL, strlen(MSGGAMEUPFAIL), 0);
               }
               else if (strcmp(token, "TANCAR_PARTIDA") == 0) {
                  if (split(command, 2) == 0) {
                     erasePort = atoi(token);
                     posGame = searchGame((char *) inet_ntoa(client.sin_addr), erasePort);
                     if (posGame >= 0) { 
                        eraseGame(posGame);
                        do {
                           posPlayer = searchPlayer((char *) inet_ntoa(client.sin_addr), erasePort);
                           if (posPlayer >= 0) erasePlayer(posPlayer);
                        } while (posPlayer >= 0);
                     
                        send(fd2, MSGGAMEDOWNOK, strlen(MSGGAMEDOWNOK), 0);
                     }
                     else send(fd2, MSGGAMEDOWNERR, strlen(MSGGAMEDOWNERR), 0);
                  }
                  else send(fd2, MSGGAMEDOWNFAIL, strlen(MSGGAMEDOWNFAIL), 0);
               }
               else if (strcmp(token, "ALTA_USUARI") == 0) {
                  if (split(command, 2) == 0) {
                     strcpy(tPlayer[counterPlayers].nickname, token);
                     strcpy(tPlayer[counterPlayers].game.IPaddress, (char *) inet_ntoa(client.sin_addr));

                     if (split(command, 3) == 0) { 
                        tPlayer[counterPlayers].game.port = atoi(token);
                        counterPlayers++;
                        send(fd2, MSGPLAYERUPOK, strlen(MSGPLAYERUPOK), 0);
                     }
                     else {
                        fport = findPort((char *) inet_ntoa(client.sin_addr));
                        if (fport >= 0) { 
                           tPlayer[counterPlayers].game.port = fport;
                           counterPlayers++;
                           send(fd2, MSGPLAYERUPOK, strlen(MSGPLAYERUPOK), 0);
                        }
                        else send(fd2, MSGPLAYERUPFAIL, strlen(MSGPLAYERUPFAIL), 0);
                     }
                  }
                  else send(fd2, MSGPLAYERUPFAIL, strlen(MSGPLAYERUPFAIL), 0);
               }
               else if (strcmp(token, "BAIXA_USUARI") == 0) {
                  if (split(command, 2) == 0) {
                     eraseNick = malloc(strlen(token));
                     strcpy(eraseNick, token);
                     if (split(command, 3) == 0) {
                        posPlayer = searchPlayer2((char *) inet_ntoa(client.sin_addr), atoi(token), eraseNick);
                        if (posPlayer >= 0) {
                           erasePlayer(posPlayer);
                           counterPlayers--;
                           send(fd2, MSGPLAYERDOWNOK, strlen(MSGPLAYERDOWNOK), 0);
                        }
                        else send(fd2, MSGPLAYERDOWNERR, strlen(MSGPLAYERDOWNERR), 0);
                     }
                     else {
                        fport = findPort((char *) inet_ntoa(client.sin_addr));
                        if (fport >= 0) {
                           posPlayer = searchPlayer2((char *) inet_ntoa(client.sin_addr), fport, eraseNick);
                           if (posPlayer >= 0) {
                              erasePlayer(posPlayer);
                              counterPlayers--;
                              send(fd2, MSGPLAYERDOWNOK, strlen(MSGPLAYERDOWNOK), 0);  
                           }
                           else send(fd2, MSGPLAYERDOWNERR, strlen(MSGPLAYERDOWNERR), 0);                      
                        }
                        else send(fd2, MSGPLAYERDOWNERR, strlen(MSGPLAYERDOWNERR), 0);     
                     }                
                  }
                  else send(fd2, MSGPLAYERDOWNFAIL, strlen(MSGPLAYERDOWNFAIL), 0);
               }
               else if (strcmp(token, "OBTENIR_PARTIDES") == 0) {
                  if (counterGames > 0) {
                     msgSend[0] = '\x00';
                     pmsgSend = &msgSend[0];
                     for(i=0; i < counterGames; i++) {
                        pmsgSend += sprintf(pmsgSend, "[ %d ] %s:%d ", i + 1, tGame[i].IPaddress, tGame[i].port);
                     }
                     send(fd2, msgSend, strlen(msgSend), 0);
                  }
                  else {
                     send(fd2, MSGEMPTYGAMES, strlen(MSGEMPTYGAMES), 0);   
                  } 
               }
               else {   
                  if (get_userCmd(token) == 0) {
                    if (strcmp(userCmd, "ON_ES_USUARI") == 0) {
             
                          nickname = strrchr(token, ':');
                          if (nickname != NULL) {
                             nickname++;
                             posPlayer = searchPlayer3(nickname);
                             if (posPlayer >= 0) {
                                sprintf(msgSend, "%s@%s:%d", tPlayer[posPlayer].nickname, tPlayer[posPlayer].game.IPaddress, tPlayer[posPlayer].game.port);
                                send(fd2, msgSend, strlen(msgSend), 0);
                             }
                             else {
                                if (counterPlayers > 0) {
                                   msgSend[0] = '\x00';
                                   pmsgSend = &msgSend[0];
                                   for(i=0; i < counterPlayers; i++) {
                                      pmsgSend += sprintf(pmsgSend, "[ %d ] %s@%s:%d", i + 1, tPlayer[i].nickname, tPlayer[i].game.IPaddress, tPlayer[i].game.port);
                                   }
                                   send(fd2, msgSend, strlen(msgSend), 0);                                  
                                }
                                else send(fd2, MSGNOEXPLAYER, strlen(MSGNOEXPLAYER), 0);  
                             }  
                          }
                          else send(fd2, MSGPLAYERSFAIL, strlen(MSGPLAYERSFAIL), 0);

                    }
                 }
              }
           }            
           close(fd2);
         }
      }
      else {
         fprintf(stderr, "\n------------------------------------------------------------------");
         fprintf(stderr, "\n\t[ ERROR ] El port no pertany al rang [1025, 65535]");
         fprintf(stderr, "\n------------------------------------------------------------------\n");      
         exit(-2);      
      }
   }
   else {
      fprintf(stderr, "\n------------------------------------------------------------------");
      fprintf(stderr, "\n\t[ ERROR ] El nombre de parametres es incorrecte");
      fprintf(stderr, "\n\t[ PARAM ] %s sim_reg_listening_port", argv[0]);
      fprintf(stderr, "\n\t[ EXAMP ] %s 7878", argv[0]);
      fprintf(stderr, "\n------------------------------------------------------------------\n");      
      exit(-1);
   }
}




