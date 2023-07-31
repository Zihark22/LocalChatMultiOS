#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define IPserveur "192.168.0.37"

#define TAILLE_MSG 1024
#define TAILLE_BUF TAILLE_MSG
#define TAILLE_NOM 50

#define NBR_CO_MAX 3