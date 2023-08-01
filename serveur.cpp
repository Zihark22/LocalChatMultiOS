#include "config.hpp"

void *connection_handler(void *);
void fin(int n);
void popClient();
void checkClient(int socket_desc);
void afficheClients();

int compteurClients;
Client *tabClient;
Client user;

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[TAILLE_BUF] = {0};
    int opt = 1;

    struct sigaction action;
	int sig,i=0;

    tabClient=NULL;
    compteurClients=0;

    // on commence par prevoir la terminaison sur signal du serveur
	action.sa_handler = fin;
	for(i=1; i<NSIG; i++) sigaction(i, &action, NULL);	// installation du handler de fin pour tous les signaux

    // Tableau des clients connectés
	tabClient=(Client*)malloc((NBR_CO_MAX+1)*sizeof(Client));
	if(tabClient==NULL) {
		printf("Problème allocation\n");
		exit(EXIT_FAILURE);
	}

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

    address.sin_family = AF_INET;           // IPv4
    address.sin_addr.s_addr = INADDR_ANY;   // Socket sera lié à toutes les interfaces réseau disponibles sur la machine (c'est-à-dire qu'il sera accessible via toutes les adresses IP de la machine)
    address.sin_port = htons(PORT);         // Convertir l'entier court (le numéro de port) en une représentation réseau. C'est nécessaire pour assurer que l'ordre des octets est correct pour le réseau (car le réseau utilise généralement l'ordre des octets en réseau, également appelé "big-endian")

    // Association d'une adresse au socket, afin qu'il puisse écouter les connexions entrantes sur cette adresse spécifique.
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Binding du socket a échoué");
        exit(EXIT_FAILURE);
    }

    // Récupération du PID
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
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0 && compteurClients>=NBR_CO_MAX) {  // Bloquant
            perror("Erreur accepté de la connexion");
            exit(EXIT_FAILURE);
        }

        // Récupérer l'adresse IP du client
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Client connecté avec l'adresse IP : %s\n", client_ip);
        user.ip=client_ip;

        pthread_t thread_id;
        printf("Connexion acceptée\n");

        // Création d'un nouveau thread en parallèle au thread principal pour gérer une connexion client dans un environnement multithread
        if (pthread_create(&thread_id, NULL, connection_handler, (void *)&new_socket) < 0) {
            perror("Erreur de création d'un processus pour le client");
            exit(EXIT_FAILURE);
        }

        printf("Processus %ld créé pour la communication du client\n\n", (long) thread_id);
    }
    return 0;
}

void fin(int n) {
	fprintf(stderr, "\nTerminaison du serveur.\n");
	exit(EXIT_SUCCESS);
}

void *connection_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    int valread;
    string buffer, nom, msg;

    // Lecture des données à partir du socket dans le tampon
    while((valread = read(sock, &buffer, buffer.size())) > 0) { // Bloquant

        cout << "buffer=" << buffer << endl;
        // Trouver la position du délimiteur (":")
        size_t delimiterPos = buffer.find(':');

        // Extraire le nom et le message à partir de la chaîne d'entrée
        nom = buffer.substr(0, delimiterPos-1);
        msg = buffer.substr(delimiterPos + 2); // +2 pour ignorer l'espace après le délimiteur

        user.name=nom;
        checkClient(sock);

        if(msg==MSG_DECO) {
            cout << "Client " << nom << " déconnecté" << endl;
            popClient();
            if (compteurClients==1)
                cout << "Fin du chat !" << endl;
            else if(compteurClients<1)
                fin(compteurClients);
            break;
        }
        else {
            cout << nom << " : " << msg << endl; 
            // send(sock, buffer, strlen(buffer), 0);  // Envoye des données à travers le socket sock vers le destinataire connecté
        }
    }

    close(sock);         // destruction du socket client
    pthread_exit(NULL);  // destruction du thread
}

void popClient() {
    int indice=compteurClients, i=0;
    for ( i = 0; i < compteurClients; ++i)
    {
        if(tabClient[i].name==user.name && tabClient[i].ip==user.ip)
            indice=i;
    }
    for (i = indice; i < compteurClients; ++i)
        tabClient[i]=tabClient[i+1];
    compteurClients--;
}
void checkClient(int socket_desc) {
    int cmpt=0;
    for(int i = 0; i < compteurClients; ++i)
    {
        if(tabClient[i].name==user.name && tabClient[i].ip==user.ip)
            break;
        else
            cmpt++;
    }
    if(cmpt==compteurClients) 
    {
        compteurClients++;
        if(compteurClients>NBR_CO_MAX) // limite de connexion 
        {
            printf("\nNombre de clients maximum déjà atteint.\n");
            close(socket_desc);
            pthread_exit(NULL);
        }
        else 
        { 
            tabClient[compteurClients-1]=user;
            afficheClients();
            if(compteurClients==2) printf("Début du chat !\n");
        }
    }
}
void afficheClients() {
    printf("\nListe des clients :\n");
    for(int i = 0; i < compteurClients; ++i)
    {
        cout << "\t" << tabClient[i].name << "\t-->\tIP = " << tabClient[i].ip << endl;
    }
    printf("\n");
}