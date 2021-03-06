#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_net.h>

char switch_axe(int val, int seuil, char high, char low, char def) {
  if(seuil < val)
    return high;
  else if(val < -seuil)
    return low;
  else
    return def;
}

TCPsocket init_net(char *adresse, int port){
  TCPsocket sd;
  IPaddress ip;
  /* Simple parameter checking */
  if (SDLNet_Init() < 0){
      fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
      exit(EXIT_FAILURE);
    }
 
  /* Resolve the host we are connecting to */
  if (SDLNet_ResolveHost(&ip, adresse, port)){
      fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
      exit(EXIT_FAILURE);
    }
 
  /* Open a connection with the IP provided (listen on the host's port) */
  if (!(sd = SDLNet_TCP_Open(&ip))){
      fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
      exit(EXIT_FAILURE);
    } 

  fprintf(stderr,"Connection enabled\n");
  return sd;
}

void priority(char *buffer){
  int i = 3;
  while(buffer[i]=='B' && i < 9){
    fprintf(stderr,"buffer : %c : %d\n", buffer[i],i);
    i++;
  }
  buffer[0] = buffer[i];
}

int capture_event(char *buffer, SDL_Event event, int keepgoing){
  switch(event.type) {   /* Process the appropriate event type */
  case SDL_JOYAXISMOTION:
    /*Valable au moins pour l'Xbox controler*/
    switch(event.jaxis.axis){
    case 0: // droite - gauche pad(s) gauche
      buffer[5] = switch_axe(event.jaxis.value, 10000, 'D', 'Q', 'B');
      break;
    case 1: // haut - bas pad(s) gauche
      buffer[4] = switch_axe(event.jaxis.value, 10000, 'S', 'Z', 'B');
      break;
    default:
      break;
    }
    break;
  case SDL_JOYBUTTONDOWN:
    switch(event.jbutton.button){
    case 0:  /*  A   */
      buffer[6]='A';
      break;
    case 1:  /*  B   */
      buffer[8]='R';
      break;
    case 2:  /*  X   */
      buffer[7]='C';
      break;
    case 3:  /*  Y   */
      buffer[9]='F';
      break;
    default:
      break;
    }
    break;
  case SDL_JOYBUTTONUP:
    switch(event.jbutton.button){
    case 0:  /*  A   */
      buffer[6]='B';
      break;
    case 1:  /*  B   */
      buffer[8]='B';
      break;
    case 2:  /*  X   */
      buffer[7]='B';
      break;
    case 3:  /*  Y   */
      buffer[9]='B';
      break;
    default:
      break;
    }
    break;
  case SDL_QUIT:
    keepgoing = 0;
    break;
  }
  return keepgoing;
}

int main( void ){
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);                      // Demarre SDL
  int keepgoing = 1;                                // Controle de boucle "infini"
  SDL_Event event;                                  // Charge les events
  char buffer[512];
  buffer[3]='B';
  buffer[10]='\0';
  TCPsocket sd = init_net("10.0.0.1",3000);		/* Socket descriptor */

  /*  Charge le joystick  */
  SDL_Joystick * joy;
  for (int i = 0 ; i < SDL_NumJoysticks() ; i++ ){
    joy=SDL_JoystickOpen(i);
    printf("Joystick : detected %d\n", i );
    printf("Joystick : Nom:\t%s\n", SDL_JoystickName(i));
    printf("Joystick : axes :\t%d\n",SDL_JoystickNumAxes(joy));
  }
  printf("Joystick : number : %d\n", SDL_NumJoysticks());

  buffer[2] = '\0';
  while (keepgoing)  {
    /********************** Boucle Infinie ********************************/
    /* Check for events */
    while(SDL_PollEvent(&event)) { /* Loop until there are no events left on the queue */
      keepgoing = capture_event(buffer, event, keepgoing);
      priority(buffer);
      if(buffer[0] != buffer[2]) {
	buffer[2] = buffer[0];
	buffer[1] = '\0';
	SDLNet_TCP_Send(sd, (void *)buffer, 1);
      }
    }
  }
  /*************************************************************************/
  fprintf(stderr,"\n Closing connection\n");
  SDLNet_TCP_Close(sd);
  SDL_Quit();                             //ferme SDL
  return EXIT_SUCCESS;
}
