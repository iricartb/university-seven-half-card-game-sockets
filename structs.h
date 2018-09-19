typedef struct {
   char IPaddress[16];
   unsigned int port;
} Game;

typedef struct {
   char nickname[20];
   Game game;
} Player;

typedef struct { 
   int ID;                                // Identificadores de los clientes conectados
   char nickname[20];
   unsigned int state;                    /* 0 = Activo, 1 = Plantado, 2 = Espera, 3 = Abandonar */
   unsigned int lNaips[10];
   unsigned int count;
   float score;
} PlayerServ;

typedef struct {
   unsigned int lNaips[40];
   unsigned int count;
} Naips;

