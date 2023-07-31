#include "config.h"

int main(int argc, char const *argv[]) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char message[1024] = {0};
    char name[50];

    printf("Veuillez entrer votre nom : ");
    fgets(name, 50, stdin);
    strtok(name, "\n");

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Erreur de création du socket \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "192.168.0.37", &serv_addr.sin_addr) <= 0) {
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
        fgets(message, 1024, stdin);
        strtok(message, "\n");

        char send_message[1074] = {0};
        sprintf(send_message, "%s : %s", name, message);

        send(sock, send_message, strlen(send_message), 0);

        printf("Message envoyé\n\n");
     }
    return 0;
}