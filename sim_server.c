#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>
#include <time.h>
#include "structs.h"
#include "memoria.h"

#define MAXPORT       65535
#define MAXDATASIZE    2000
#define MAXDATANIPE      40
#define MAXDATAWINNER    60
#define RETURN           10
#define SPACE            32
#define DOT              46
#define MAXCONNECTIONS    8
#define MINUSERS          2
#define MAXNAIPS         40
#define GAMEPOINT        7.5
#define MSGWAITING       "ESPERA_FINAL_MA"
#define MSGSYNC          "SINCRONITZANT PARTIDA AMB ALTRES USUARIS..."
#define MSGWELCOME       "BENVINGUT NAIPS: 40 PUNTUACIO: 0"
#define MSGNAIP          "VOLS_CARTA"
#define MSGWINNER        "HA_GUANYAT"
#define MSGLOSERS        "NO HI HA CAP GUANYADOR"
#define TTL              10
#define MSGLINE          "_____________________________________________________________________\n"

void * sinc_game(void *ptr);
void * command;
char * token;

char * nickname;
char * userCmd;                             // String -> Split(cadena, ':')[0]
char lIP[16];                               // IP Sim_Serv
unsigned int lport;                         // Puerto Sim_Serv        
char simReg_IP[16];                         // IP Sim_Reg
unsigned int simReg_port;                   // Puerto Sim_Reg
unsigned int countPlayers;                  // Contador de Jugadores Conectados a la Partida
PlayerServ listPlayers[MAXCONNECTIONS];     // Lista de jugadores conectados
Naips listNaips;                            // Lista de cartas
PlayerServ tmpPlayer;
unsigned int k, activePlayers;
char listGamesState[MAXPORT];               // Estado Partidas
int idstateGames;
int fd_SimServer;

void iniStateGames(char * point) {
   int i;
   
   for(i=0; i < MAXPORT; i++) *(point + i) = 'N';
}

void eraseExitPlayers(unsigned int numPlayers) {
   int k, l, erasePlayersCount = 0;

   for(k=0; (k < (numPlayers - erasePlayersCount)); k++) {
      if ((listPlayers[k].state == 3) && (k != (numPlayers - erasePlayersCount - 1))) {
         for (l=k+1; (l < (numPlayers - erasePlayersCount)); l++) {
            listPlayers[l-1].ID = listPlayers[l].ID;
            listPlayers[l-1].state = listPlayers[l].state;
            strcpy(listPlayers[l-1].nickname, listPlayers[l].nickname);                               
         }
         erasePlayersCount++; k--;
      }   
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

int split(char *command, int pos) {
  int i=0, j=0, word=0;
  unsigned int lport;
 
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

int num_tokens(char *command) {
  int i=0; int word=0;
 
  if ((*(command + i) != SPACE) && (*(command + i) != RETURN) && (*(command + i) != '\x00')) { word++; i++; }
  while (*(command + i) != '\x00') {
    if ((*(command + i) == SPACE) &&  (*(command + i + 1) != SPACE) && (*(command + i + 1) != '\x00')) word++;
    i++;
  }
 
  return(word);
}

void iniNaips(void) {
   int i, module;

   for(i=0; i < MAXNAIPS; i++) {

     module = ((i+1) % 10);
     if ((module == 8) || (module == 9) || (module == 0)) {
        if (module == 8) listNaips.lNaips[i] = 10;
        else if (module == 9) listNaips.lNaips[i] = 11; 
        else listNaips.lNaips[i] = 12; 
     }
     else listNaips.lNaips[i] = module;
   }
}

void eraseNaip(unsigned int value) {
   if (value == MAXNAIPS - listNaips.count) listNaips.count++;
   else { listNaips.lNaips[value] = listNaips.lNaips[MAXNAIPS - listNaips.count - 1]; listNaips.count++; }
}
 
int get_userCmd(char *command) {
   int i=0, j=0;
   
   userCmd = malloc(strlen(command));

   for(;((*(command + i) != ':') && (*(command + i) != RETURN) && (*(command + i) != '\x00')); i++) { *(userCmd + j) = *(command + i); j++; }
   *(userCmd + j) = '\x00';
   
   if ((*(userCmd + j - 1) == RETURN) || (*(userCmd + j - 1) == '\x00')) return(-1);
   else return(0);
}

int get_nick(char *command) {
   int i=0, j=0;
   
   nickname = malloc(strlen(command));
   for(;((*(command + i) != ':') && (*(command + i) != RETURN) && (*(command + i) != '\x00')); i++);

   if ((*(command + i) == RETURN) || (*(command + i) == '\x00')) return(-1);
   else {
      i++;
      for(;*(command + i) != '\x00'; j++, i++) *(nickname + j) = *(command + i);
      *(nickname + j) = '\x00';
      return(0); 
   } 
}

void * sinc_game(void *ptr) {
   char msgSend[MAXDATASIZE], msgRecv[MAXDATASIZE];
   int  fd_SimReg, numBytes, sin_size, i, gamePlaying, IDTemp, closeClient = 1;
   struct sockaddr_in server_SimReg, server_SimServer, client_SimClient;
   struct hostent * he;
   time_t ftime, ltime;
  
   he = gethostbyname(simReg_IP);
   if (he != NULL) {
      server_SimReg.sin_family = AF_INET;
      server_SimReg.sin_port = htons(simReg_port);
      server_SimReg.sin_addr = *((struct in_addr *) he->h_addr);
      bzero(&(server_SimReg.sin_zero), 8);

      if ((fd_SimServer = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
         fprintf(stderr, "\n\t[ ! ] Error en el socket");
         exit(-7);
      }
      
      /* Configuramos parametros del servidor */
      server_SimServer.sin_family = AF_INET;
      server_SimServer.sin_port = htons(lport);
      server_SimServer.sin_addr.s_addr = INADDR_ANY;
      bzero(&(server_SimServer.sin_zero), 8);

      /* Creamos el canal */
      if (bind(fd_SimServer, (struct sockaddr *) &server_SimServer, sizeof(struct sockaddr)) == -1) {
         fprintf(stderr, "\n\t[ ! ] Error en el bind");
         exit(-8);
      } 
 
      if (listen(fd_SimServer, MAXCONNECTIONS) == -1) {
         fprintf(stderr, "\n\t[ ! ] Error en listen");
         exit(-9);
      }
  
      for(;;) {   
         if (countPlayers < MAXCONNECTIONS) {
            sin_size = sizeof(struct sockaddr_in);

            /* countPlayers se modifica => es necesario una variable temporal que almacene el ID */ 
            IDTemp = accept(fd_SimServer, (struct sockaddr *) &client_SimClient, &sin_size);
            listPlayers[countPlayers].ID = IDTemp;
            
            // Cuenta atras para introducir el comando correcto
            time(&ftime); time(&ltime);
            if (listPlayers[countPlayers].ID == -1) {   
               fprintf(stderr, "\n\t[ ! ] Error en listen");
               close(listPlayers[countPlayers].ID);
               exit(-10);
            }        

            /* RECIBIMOS EL MENSAJE SOC:RENOM DEL SIM_CLIENT */
            while ((ltime - ftime) < TTL) {
               numBytes = recv(listPlayers[countPlayers].ID, msgRecv, MAXDATASIZE, 0);  
               if (numBytes == -1) {
                  fprintf(stderr, "\n\t[ ! ] Error en la lectura del socket");
                  close(listPlayers[countPlayers].ID);
                  exit(-11);
               } 
               msgRecv[numBytes] = '\x00';

               if (get_userCmd(msgRecv) == 0) {
                  if (strcmp(userCmd, "SOC") == 0) {
                     if (get_nick(msgRecv) == 0) { 
                        fd_SimReg = socket(AF_INET, SOCK_STREAM, 0);
                        if (connect(fd_SimReg, (struct sockaddr *) &server_SimReg, sizeof(struct sockaddr)) != -1) {
                           sprintf(msgSend, "ALTA_USUARI %s %d", nickname, lport);
                           send(fd_SimReg, msgSend, strlen(msgSend), 0);
                           numBytes = recv(fd_SimReg, msgRecv, MAXDATASIZE, 0);
                           if (numBytes == -1) {
                              fprintf(stderr, "\n\t[ ! ] Error en la lectura del socket");
                              close(fd_SimReg);
                              exit(-12);
                           } 
                           msgRecv[numBytes] = '\x00';
                           // --> ENVIAR AL CLIENTE fprintf(stdout, "\n%s: %s\n", msgRecv, nickname);
                           strcpy(listPlayers[countPlayers].nickname, nickname);
                           listPlayers[countPlayers].state = 2;
                           listPlayers[countPlayers].score = 0;

                           gamePlaying = 0;
                           for(i=0; ((i < countPlayers) && (gamePlaying == 0)); i++) {
                              if (listPlayers[i].state == 0) {
                                 gamePlaying = 1;
                              }
                           }
                           if (gamePlaying == 1) send(listPlayers[countPlayers].ID, MSGWAITING, strlen(MSGWAITING), 0);
                           else send(listPlayers[countPlayers].ID, MSGSYNC, strlen(MSGSYNC), 0);
                           
                           // Sincronització
                           recv(listPlayers[countPlayers].ID, msgRecv, MAXDATASIZE, 0);
                           countPlayers++;
                           ltime += TTL;
                           closeClient = 0;
                        }
                        else { close(listPlayers[countPlayers].ID); ltime += TTL; }
                     }
                     else { time(&ltime); }
                  }
                  else { time(&ltime); }          
               }
               else { time(&ltime); }
            }
            if (closeClient == 1) { close(listPlayers[countPlayers].ID); }
         }
      }
   }
}

int main(int argc, char ** argv) {
   char msgSend[MAXDATASIZE], msgRecv[MAXDATASIZE], msgSendNaip[MAXDATANIPE], msgSendWinner[MAXDATAWINNER], winners[MAXDATASIZE];
   int fd_SimReg, numBytes, iret1, j, i;
   unsigned int sumInactivePlayers;
   float gameValue = GAMEPOINT, dec;
   unsigned int maxIDPlayer, val, fiGame;
   struct sockaddr_in server_SimReg;
   struct hostent * he;
   pthread_t threadSincGame;
   char * pStateGames;
   char * pState;

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
      
               // memoria compartida
               idstateGames = ini_mem(MAXPORT);
               pStateGames = (char *) map_mem(idstateGames);
               iniStateGames(pStateGames);
             
               for(;;) {
                  command = malloc(MAXDATASIZE);
                  fprintf(stdout, "\n");
                  fprintf(stdout, MSGLINE);
                  fprintf(stdout, "\n>>> Llistat de comandes:\n\n\t[ 1 ] OBRIR_PARTIDA IP PORT\n\t");
                  fprintf(stdout, "   [ - ] Creacio d'una partida 7 i mig al servidor i port especificat\n\n\t");		  
                  fprintf(stdout, "[ 2 ] TANCAR_PARTIDA PORT\n\t   [ - ] Tancament d'una partida local al port especificat\n");
                  fprintf(stdout, MSGLINE);
                  fprintf(stdout, "\n>>> [ COMMAND ]: ");
                  scanf("%s", (char *) command);
  
                  if (strcmp(command, "OBRIR_PARTIDA") == 0) {

                     scanf("%s %d", lIP, &lport); 
        
                     if ((formatIP(lIP) == 0) && (lport > 0) && (lport <= MAXPORT)) {
                     
                        if (connect(fd_SimReg, (struct sockaddr *) &server_SimReg, sizeof(struct sockaddr)) != -1) {
                           sprintf(msgSend, "OBRIR_PARTIDA %s:%d", lIP, lport);
                           *(pStateGames + (lport - 1)) = 'N';

                           send(fd_SimReg, msgSend, strlen(msgSend), 0);
                           numBytes = recv(fd_SimReg, msgRecv, MAXDATASIZE, 0);
                           if (numBytes == -1) {
                              fprintf(stderr, "\n\t[ ! ] Error en la lectura del socket\n");
                              fprintf(stderr, MSGLINE);
                              close(fd_SimReg);
                              exit(-6);
                           }
                           msgRecv[numBytes] = '\x00';
                           close(fd_SimReg);
                           fprintf(stdout, "\n>>> [ "); fprintf(stdout, msgRecv); fprintf(stdout, " ]\n");

                           countPlayers = 0;

                           if (fork() == 0) {
                              pState = ((char *) map_mem(idstateGames) + (lport-1));
                              iret1 = pthread_create(&threadSincGame, NULL, sinc_game, NULL);
      
                              for(;*(pState) == 'N';) {
                                 /* COMENÇA PARTIDA */
                                 if (countPlayers >= MINUSERS) {
                                    listNaips.count = 0;
                                    iniNaips();

                                    srand((unsigned) time (NULL));
                                    maxIDPlayer = countPlayers;
                                    activePlayers = maxIDPlayer;

                                    // Estado jugador -> Activado, Enviamos mensaje de bienvenida
                                    for(i=0; i < maxIDPlayer; i++) {
                                       listPlayers[i].state = 0;
                                       listPlayers[i].count = 0;
                                       listPlayers[i].score = 0;
                                       send(listPlayers[i].ID, MSGWELCOME, strlen(MSGWELCOME), 0);
                          
                                       // Sincronització
                                       recv(listPlayers[i].ID, msgRecv, MAXDATASIZE, 0);

                                       // Enviamos la 1 carta
                                       val = (rand() % (MAXNAIPS - listNaips.count));
                                       listPlayers[i].lNaips[listPlayers[i].count] = listNaips.lNaips[val];
                                    
                                       if (listNaips.lNaips[val] > 7) listPlayers[i].score += 0.5;
                                       else listPlayers[i].score += (float) listNaips.lNaips[val];
                                       listPlayers[i].count++;
                                       sprintf(msgSendNaip, "CARTA: %d %.1f", listNaips.lNaips[val], listPlayers[i].score);
                                       send(listPlayers[i].ID, msgSendNaip, strlen(msgSendNaip), 0);

                                       // Sincronització
                                       recv(listPlayers[i].ID, msgRecv, MAXDATASIZE, 0);

                                       // Eliminar carta
                                       eraseNaip(val);
                                    }
                                    while ((listNaips.count < MAXNAIPS) && (activePlayers != 0)) {
                                       for(i=0; ((i < maxIDPlayer) && (listNaips.count < MAXNAIPS)); i++) {
                                          if (listPlayers[i].state == 0) {
                                             send(listPlayers[i].ID, MSGNAIP, strlen(MSGNAIP), 0);
                                       
                                             // Sincronització
                                             recv(listPlayers[i].ID, msgRecv, MAXDATASIZE, 0);

                                             // Mensajes cliente: EM_PLANTO, NOVA_CARTA, ABANDONAR_PARTIDA
                                             numBytes = recv(listPlayers[i].ID, msgRecv, MAXDATASIZE, 0);
                                             if (numBytes == -1) {
                                                fprintf(stderr, "\n\t[ ! ] Error en la lectura del socket\n");
                                                close(listPlayers[i].ID);
                                                exit(-13);
                                             }
                                             msgRecv[numBytes] = '\x00';
                                             if (split(msgRecv, 1) == 0) {
                                                if (strcmp(token, "NOVA_CARTA") == 0) {
                                                   val = (rand() % (MAXNAIPS - listNaips.count));
                                                   listPlayers[i].lNaips[listPlayers[i].count] = listNaips.lNaips[val];
                                                   if (listNaips.lNaips[val] > 7) listPlayers[i].score += 0.5;
                                                   else listPlayers[i].score += (float) listNaips.lNaips[val];
                                                   listPlayers[i].count++;
                                                   sprintf(msgSendNaip, "TIRADA %s %d %.1f", listPlayers[i].nickname, listNaips.lNaips[val], listPlayers[i].score);

                                                   for(j=0; (j < maxIDPlayer); j++) {
                                                      send(listPlayers[j].ID, msgSendNaip, strlen(msgSendNaip), 0);
                                                      // Sincronització
                                                      recv(listPlayers[j].ID, msgRecv, MAXDATASIZE, 0);
                                                   }
                                                   // Borrar la carta
                                                   eraseNaip(val);
                                                }
                                                else if (strcmp(token, "EM_PLANTO") == 0) {
                                                   listPlayers[i].state = 1;
                                                   activePlayers--;
                                                }
                                                else if (strcmp(token, "ABANDONAR_PARTIDA") == 0) {
                                                   listPlayers[i].state = 3; 
                                                   activePlayers--;

                                                   // Dar de baja el usuario del Sim_Reg       
                                                   fd_SimReg = socket(AF_INET, SOCK_STREAM, 0);            
                                                   if (connect(fd_SimReg, (struct sockaddr *) &server_SimReg, sizeof(struct sockaddr)) != -1) {
                                                      sprintf(msgSend, "BAIXA_USUARI %s %d", listPlayers[i].nickname, lport);
                                                      send(fd_SimReg, msgSend, strlen(msgSend), 0);
                                                      numBytes = recv(fd_SimReg, msgRecv, MAXDATASIZE, 0);
                                                      if (numBytes == -1) {
                                                         fprintf(stderr, "\n\t[ ! ] Error en la lectura del socket");
                                                         close(fd_SimReg);
                                                         exit(-6);
                                                      }
                                                      msgRecv[numBytes] = '\x00';
                                                   }
                                                   close(listPlayers[i].ID);  
                                                }
                                                else i--;
                                             }
                                          }
                                       }
                                    }   
                                    sumInactivePlayers = 0; dec = 0; fiGame = 0;
                                    // Suma de Puntos
                                    while ((fiGame == 0) && (dec < GAMEPOINT)) {
                                       for(j=0; j < maxIDPlayer; j++) {
                                          if (listPlayers[j].state != 3) {
                                             if (listPlayers[j].score == (GAMEPOINT - dec)) {
                                                strcat(winners, listPlayers[j].nickname); strcat(winners, " ");
                                                fiGame = 1;
                                             }
                                          }
                                       }
                                       dec += 0.5;
                                    }      

                                    // Enviamos mensaje de ganador a todos los clientes activos
                                    for(i=0; i < maxIDPlayer; i++) {
                                       if (listPlayers[i].state == 3) sumInactivePlayers++; 
                                       else { 
                                          // Todos los jugadores se han pasado del 7.5
                                          if (dec == GAMEPOINT) send(listPlayers[i].ID, MSGLOSERS, strlen(MSGLOSERS), 0);
                                          else {
                                             sprintf(msgSendWinner, "%s %s", MSGWINNER, winners);       
                                             send(listPlayers[i].ID, msgSendWinner, strlen(msgSendWinner), 0);
                                          }
                                          // Sincronització
                                          recv(listPlayers[i].ID, msgRecv, MAXDATASIZE, 0);
                                       } 
                                    }
                                    eraseExitPlayers(countPlayers);
                                    countPlayers -= sumInactivePlayers;    
                                    winners[0] = '\x00';     
                                 }
                                 else usleep(200);
                              } // --> Close for(;;)
                              close(fd_SimServer);
                              elim_mem(idstateGames);
                              exit(0);
                           }          
                           fd_SimReg = socket(AF_INET, SOCK_STREAM, 0);  
                        }
                        else fprintf(stdout, "\n>>> [ ERROR ]: No s'ha pogut conectar amb el Sim_Reg\n");
                     }
                     else {
                        fprintf(stdout, "\n>>> [ ERROR ]: IP o port amb format incorrecte\n");   
                     } 
                  }
                  else if (strcmp(command, "TANCAR_PARTIDA") == 0) {
                     scanf("%d", &lport);       
                     
                     if ((lport > 0) && (lport <= MAXPORT)) {             
                        if (connect(fd_SimReg, (struct sockaddr *) &server_SimReg, sizeof(struct sockaddr)) != -1) {
                           sprintf(msgSend, "TANCAR_PARTIDA %d", lport);
                           send(fd_SimReg, msgSend, strlen(msgSend), 0);
                           numBytes = recv(fd_SimReg, msgRecv, MAXDATASIZE, 0);
                           if (numBytes == -1) {
                              fprintf(stderr, "\n\t[ ! ] Error en la lectura del socket");
                              close(fd_SimReg);
                              exit(-5);
                            }
                            msgRecv[numBytes] = '\x00';
                            fprintf(stdout, "\n>>> [ "); fprintf(stdout, msgRecv); fprintf(stdout, " ]\n");
                        }
                        *(pStateGames + (lport - 1)) = 'S';
                     }
                     else fprintf(stdout, "\n>>> [ ERROR ]: Port amb format incorrecte range(1 - 65535)\n");             
                  }
                  else fprintf(stdout, "\n>>> [ ERROR ]: Comanda no reconeguda\n");
                  fd_SimReg = socket(AF_INET, SOCK_STREAM, 0);
                  free(command);
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


