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

#define NBR_CO_MAX 4
#define MSG_DECO "!deco" // msg de deconection

using namespace std;

typedef struct {  
	string name;   
	string ip;
	int socket;
} Client;

// Séquences d'échappement ANSI 
#define DEFAULT "\x1b[0m" 		// retour à la normale
#define TXT_CYAN_U "\x1b[36;4m" // texte en cyan sous-ligné
#define TXT_RED_B "\x1b[31;1m" // texte en rouge et gras

/*
Les séquences d'échappement ANSI sont des séquences de caractères commençant par le caractère d'échappement `\x1b` (également représenté par `ESC` ou `\033` en octal). Elles sont utilisées pour contrôler le formatage et le positionnement du texte affiché dans le terminal. Voici quelques-unes des séquences d'échappement ANSI les plus courantes :

1. Effacement de l'écran :
   - `\x1b[2J` : Efface tout l'écran.
   - `\x1b[K` : Efface le texte de la position du curseur jusqu'à la fin de la ligne.

2. Contrôle du curseur :
   - `\x1b[H` : Replace le curseur en haut à gauche de l'écran (équivalent à `\x1b[1;1H`).
   - `\x1b[<ligne>;<colonne>H` : Place le curseur à la ligne et la colonne spécifiées.
   - `\x1b[A` : Déplace le curseur vers le haut d'une ligne.
   - `\x1b[B` : Déplace le curseur vers le bas d'une ligne.
   - `\x1b[C` : Déplace le curseur vers la droite d'une colonne.
   - `\x1b[D` : Déplace le curseur vers la gauche d'une colonne.

3. Couleurs :
   - `\x1b[<n>m` : Change la couleur du texte en fonction de la valeur de `<n>`. Les valeurs possibles pour `<n>` sont :
      - 0 : Remise à zéro des attributs (couleur par défaut).
      - 30 à 37 : Couleurs du texte (noir, rouge, vert, jaune, bleu, magenta, cyan, blanc).
      - 40 à 47 : Couleurs de l'arrière-plan (noir, rouge, vert, jaune, bleu, magenta, cyan, blanc).
      - 90 à 97 : Couleurs du texte en mode haute intensité (noir, rouge, vert, jaune, bleu, magenta, cyan, blanc).

4. Style du texte :
   - `\x1b[<n>;1m` : Active le texte en gras.
   - `\x1b[<n>;4m` : Souligne le texte.
   - `\x1b[<n>;7m` : Inverse les couleurs du texte et de l'arrière-plan.

5. Autres :
   - `\x1b[?25l` : Cache le curseur.
   - `\x1b[?25h` : Affiche le curseur.

Ceci n'est qu'une petite sélection de séquences d'échappement ANSI. Il en existe bien d'autres, permettant de contrôler divers aspects de l'affichage dans le terminal. Notez que la prise en charge de certaines séquences d'échappement peut varier en fonction du terminal et du système d'exploitation utilisé.

L'utilisation de bibliothèques telles que "ncurses" en C/C++ ou "colorama" en Python peut faciliter la manipulation des séquences d'échappement ANSI pour des applications plus complexes.

*/


