#include "config.h"

void *connection_handler(void *);

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[TAILLE_BUF] = {0};

    // Création du socket IPv4(AF_INET) de type flux(SOCK_STREAM) en TCP (0) bidirectionel
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Erreur de création du socket");
        exit(EXIT_FAILURE);
    }

    // Config du socket server_fd pour réutiliser une adresse locale : l'option SO_REUSEADDR définie sur le socket lui même (SOL_SOCKET) permet à un socket d'écouter une adresse même si un socket précédent lié à cette adresse n'a pas encore été correctement fermé
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Erreur de configuration du socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Association d'une adresse au socket, afin qu'il puisse écouter les connexions entrantes sur cette adresse spécifique.
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Binding du socket a échoué");
        exit(EXIT_FAILURE);
    }

	pid_t pid=getpid();
	printf("Lancement du serveur !\n");
	printf("PID=%d\n", pid);
	printf("En attente de connexion d'un client ...\n\n");

    // Indique au socket qu'il est prêt à accepter les connexions entrantes des clients. Il peut mettre en attente jusqu'à 3 connexions en même temps, les autres sont refusées
    if (listen(server_fd, NBR_CO_MAX) < 0) {
        perror("Erreur dans l'attente d'une connexion");
        exit(EXIT_FAILURE);
    }

    while(1) {
        // Accepte une connexion entrante sur le socket et de créer un nouveau socket pour gérer cette connexion avec le client
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {  // Bloquant
            perror("Erreur accepté de la connexion");
            exit(EXIT_FAILURE);
        }

        pthread_t thread_id;
        printf("Connexion acceptée\n");

        // Création d'un nouveau thread en parallèle au thread principal pour gérer une connexion client dans un environnement multithread
        if (pthread_create(&thread_id, NULL, connection_handler, (void *)&new_socket) < 0) {
            perror("Erreur de création d'un processus pour le client");
            exit(EXIT_FAILURE);
        }

        printf("Processus %d créé pour la communication du client\n\n", (int) thread_id);
    }
    return 0;
}

void *connection_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    int valread;
    char buffer[TAILLE_BUF] = {0};

    // Lecture des données à partir du socket dans le tampon
    while((valread = read(sock, buffer, TAILLE_BUF)) > 0) {
        printf("%s\n", buffer);
        send(sock, buffer, strlen(buffer), 0);  // Envoye des données à travers le socket sock vers le destinataire connecté
    }

    if (valread == 0) {
        printf("Client déconnecté\n");
    } else {
        perror("Erreur dans la lecture de l'entrée");
    }

    close(sock);        // destruction du socket client
    pthread_exit(NULL);  // destruction du thread
}