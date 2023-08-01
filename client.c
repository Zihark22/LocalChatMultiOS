#include "config.h"

int main(int argc, char const *argv[]) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[TAILLE_BUF] = {0};
    char message[TAILLE_MSG] = {0};
    char name[TAILLE_NOM];

    printf("Veuillez entrer votre nom : ");
    fgets(name, TAILLE_NOM, stdin);
    strtok(name, "\n");

    // Création du socket IPv4(AF_INET) de type flux(SOCK_STREAM) en TCP (0) bidirectionel
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Erreur de création du socket \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convertion d'une adresse IP au format texte en une représentation binaire dans une structure struct sockaddr_in pour une utilisation ultérieure avec des sockets
    if(inet_pton(AF_INET, IPserveur, &serv_addr.sin_addr) <= 0) {
        printf("\nAdresse invalide/Adresse non supportée \n");
        return -1;
    }

    // Etablie une connexion réseau depuis le socket sock vers l'adresse IP et le port spécifiés dans la structure serv_addr
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnexion échouée \n");
        return -1;
    }

    pid_t pidP=getpid();
    printf("\nClient opérationnel\n");
    printf("Nom client: %s\n",name);
    printf("PID=%d\n",pidP);
    printf("Bienvenu dans le chat !\n\n");

    while(1) {
        printf("Message : ");
        fgets(message, TAILLE_MSG, stdin); // Lie une ligne de texte à partir de l'entrée standard (stdin) et de la stocker dans le tampon message
        strtok(message, "\n"); // Découpe la chaîne de caractères message en sous-chaines délimités par "\n"

        
        char send_message[TAILLE_MSG+TAILLE_NOM] = {0};
        sprintf(send_message, "%s : %s", name, message);

        // Envoye les données contenues dans la chaîne de caractères send_message à travers le socket sock vers le destinataire connecté
        send(sock, send_message, strlen(send_message), 0);

        printf("Message envoyé\n\n");
        
        if(strcmp(message,MSG_DECO)==0) {
            // close(sock);
            printf("...\nDéconection!\n");
            break;
        }

     }
    return 0;
}