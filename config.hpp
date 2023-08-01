#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <iostream>
#include <string>

#define PORT 8080
#define IPserveur "192.168.0.37"

#define TAILLE_MSG 1024
#define TAILLE_BUF TAILLE_MSG
#define TAILLE_NOM 50

#define NBR_CO_MAX 2
#define MSG_DECO "!deco" // msg de deconection

using namespace std;

typedef struct {  
	string name;   
	string ip;     
} Client;