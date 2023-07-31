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

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Erreur de création du socket \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, IPserveur, &serv_addr.sin_addr) <= 0) {
        printf("\nAdresse invalide/Adresse non supportée \n");
        return -1;
    }

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
        fgets(message, TAILLE_MSG, stdin);
        strtok(message, "\n");

        char send_message[TAILLE_MSG+TAILLE_NOM] = {0};
        sprintf(send_message, "%s : %s", name, message);

        send(sock, send_message, strlen(send_message), 0);

        printf("Message envoyé\n\n");
     }
    return 0;
}