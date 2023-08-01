#include "config.hpp"

void *reception_handler(void *socket_desc);

int main(int argc, char const *argv[]) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
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

    // Obtenir l'adresse IP du serveur
    struct sockaddr_in server_addr;
    socklen_t addr_length = sizeof(server_addr);
    if (getsockname(sock, (struct sockaddr *)&server_addr, &addr_length) == -1) {
        perror("Erreur lors de l'obtention de l'adresse IP du serveur");
        close(sock);
        return 1;
    }
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(server_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

    // Obtenir PID
    pid_t pidP=getpid();

    // Affichage
    printf("\nClient opérationnel\n");
    printf("Nom client: %s\n",name);
    printf("PID=%d\n",pidP);
    cout << "IP=" << client_ip << endl;
    printf("Bienvenu dans le chat !\n\n");


    char send_message[TAILLE_MSG+TAILLE_NOM] = {0};
    sprintf(send_message, "%s : %s", name, "co");
    send(sock, send_message, strlen(send_message), 0);

    // Création d'un nouveau thread en parallèle au thread principal pour gérer la réception des messages des autres clients dans un environnement multithread
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, reception_handler, (void *)&sock) < 0) {
        perror("Erreur de création d'un processus pour le client");
        exit(EXIT_FAILURE);
    }
    printf("Processus %ld créé pour la communication du serveur\n", (long) thread_id);
    cout << endl;

    while(1)
    {
        fgets(message, TAILLE_MSG, stdin); // Lie une ligne de texte à partir de l'entrée standard (stdin) et de la stocker dans le tampon message
        strtok(message, "\n"); // Découpe la chaîne de caractères message en sous-chaines délimités par "\n"

        char send_message[TAILLE_MSG+TAILLE_NOM] = {0};
        sprintf(send_message, "%s : %s", name, message);

        // Envoye les données contenues dans la chaîne de caractères send_message à travers le socket sock vers le destinataire connecté
        send(sock, send_message, strlen(send_message), 0);
        

        if(strcmp(message,MSG_DECO)==0) {
            printf("\n...\nDéconection!\n");
            cout << TXT_RED_B << "\n...\nDéconection!" << DEFAULT << endl;
            break;
        }
    }
    return 0;
}

void *reception_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    int valread;
    char buffer[TAILLE_BUF];
    string nom, msg;

    // Lecture des données à partir du socket dans le tampon
    while(1) { // Bloquant
        valread = read(sock, buffer, TAILLE_BUF);
        string newbuf(buffer);
        if(newbuf==MSG_DECO)
            break;
            
        cout << TXT_CYAN_U << newbuf << DEFAULT << endl;
    
        for (int i = 0; i < TAILLE_BUF; i++)
            buffer[i]='\0';
    }

    close(sock);         // destruction du socket client
    pthread_exit(NULL);  // destruction du thread
}