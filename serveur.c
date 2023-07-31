#include "config.h"

void *connection_handler(void *);

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[TAILLE_BUF] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Erreur de création du socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Erreur de configuration du socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Binding du socket a échoué");
        exit(EXIT_FAILURE);
    }

	pid_t pid=getpid();
	printf("Lancement du serveur !\n");
	printf("PID=%d\n", pid);
	printf("En attente de connexion d'un client ...\n\n");

    if (listen(server_fd, 3) < 0) {
        perror("Erreur dans l'attente d'une connexion");
        exit(EXIT_FAILURE);
    }

    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Erreur accepté de la connexion");
            exit(EXIT_FAILURE);
        }

        pthread_t thread_id;
        printf("Connexion acceptée\n");
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

    while((valread = read(sock, buffer, TAILLE_BUF)) > 0) {
        printf("%s\n", buffer);
        send(sock, buffer, strlen(buffer), 0);
    }

    if (valread == 0) {
        printf("Client déconnecté\n");
    } else {
        perror("Erreur dans la lecture de l'entrée");
    }

    close(sock);
    pthread_exit(NULL);
}