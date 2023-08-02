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
    int opt = 1,sig,i=0;
    struct sigaction action;
	
     // Initialisation de ncurses
    initscr(); // initialise ncurses et prépare affichage
    cbreak();  // désactive attente d'un retour à la ligne
    noecho();   // désactive l'affichage auto des caractères saisies
    keypad(stdscr, TRUE); // récupère les touches additionnelles 
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK); // paire 1 = caractere noire sur fond rouge

    tabClient=NULL;
    compteurClients=0;

    // on commence par prevoir la terminaison sur signal du serveur
	action.sa_handler = fin;
	for(i=1; i<NSIG; i++) sigaction(i, &action, NULL);	// installation du handler de fin pour tous les signaux

    // Tableau des clients connectés
	tabClient=(Client*)malloc((NBR_CO_MAX+1)*sizeof(Client));
	if(tabClient==NULL) {
		printw("Problème allocation\n");
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

	printw("Lancement du serveur !\n");
	printw("PID=%d\n", pid);
	printw("En attente de connexion d'un client ...\n\n");

    // Indique au socket qu'il est prêt à accepter les connexions entrantes des clients. Il peut mettre en attente jusqu'à 3 connexions en même temps, les autres sont refusées
    if (listen(server_fd, NBR_CO_MAX) < 0) {
        perror("Erreur dans l'attente d'une connexion");
        exit(EXIT_FAILURE);
    }

    while(1) {
        // Rafraîchir l'écran
        refresh();
        // Accepte une connexion entrante sur le socket et de créer un nouveau socket pour gérer cette connexion avec le client
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0 && compteurClients>=NBR_CO_MAX) {  // Bloquant
            perror("Erreur accepté de la connexion");
            exit(EXIT_FAILURE);
        }

        // Récupérer l'adresse IP du client
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
        printw("Client connecté avec l'adresse IP : %s\n", client_ip);
        user.ip=client_ip;
        user.socket=new_socket;

        // Création d'un nouveau thread en parallèle au thread principal pour gérer une connexion client dans un environnement multithread
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, connection_handler, (void *)&new_socket) < 0) {
            perror("Erreur de création d'un processus pour le client");
            exit(EXIT_FAILURE);
        }
        printw("Processus %ld créé pour la communication du client\n\n", (long) thread_id);
    }
    return 0;
}

void fin(int n) {
    for(int i = 0; i < compteurClients; ++i)
    {
        close(tabClient[i].socket);
    }
    endwin(); // Nettoyage de ncurses
	printf("Terminaison du serveur\n");
	exit(EXIT_SUCCESS);
}

void *connection_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    int valread;
    char buffer[TAILLE_BUF];
    string nom, msg;

    // Lecture des données à partir du socket dans le tampon
    while((valread = read(sock, buffer, TAILLE_BUF)) > 0) { // Bloquant
        // Rafraîchir l'écran
        refresh();
        string newbuf(buffer);
        
        // Trouver la position du délimiteur (":")
        size_t delimiterPos = newbuf.find(':');

        // Extraire le nom et le message à partir de la chaîne d'entrée
        nom = newbuf.substr(0, delimiterPos-1);
        msg = newbuf.substr(delimiterPos + 2); // +2 pour ignorer l'espace après le délimiteur

        user.name=nom;
        checkClient(sock);

        if(msg==MSG_DECO) {
            printw("Client %s déconnecté\n", nom.c_str());
            popClient();
            send(sock, MSG_DECO, strlen(buffer), 0);
            if (compteurClients==1)
                printw("\n------- Fin du chat ! -------\n\n");
            else if(compteurClients<1)
                fin(compteurClients);
            refresh();
            break;
        }
        else if(msg=="co") { // premier message auto pour enregistrer client
            
        }
        else {
            // Afficher le message
            string newbuffer = nom + " : " + msg;
            printw("%s\n",newbuffer.c_str());
            refresh();

            // Diffuser le message à tous les autres clients connectés
            for(int i = 0; i < compteurClients; ++i)
            {
                if(tabClient[i].name!=user.name || tabClient[i].ip!=user.ip)
                    send(tabClient[i].socket, newbuffer.c_str(), strlen(newbuffer.c_str()), 0);
            }
        }
        for (int i = 0; i < TAILLE_BUF; i++)
            buffer[i]='\0';
        refresh();
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
        if(tabClient[i].name==user.name && tabClient[i].ip==user.ip) {
            user=tabClient[i];
            break;
        }
        else
            cmpt++;
    }
    if(cmpt==compteurClients) 
    {
        compteurClients++;
        user.color="\x1b["+to_string(31+compteurClients)+"m";
        if(compteurClients>NBR_CO_MAX) // limite de connexion 
        {
            printw("\nNombre de clients maximum déjà atteint.\n");
            close(socket_desc);
            pthread_exit(NULL);
        }
        else 
        { 
            tabClient[compteurClients-1]=user;
            user=tabClient[compteurClients-1];
            afficheClients();
            if(compteurClients==2) printw("------- Début du chat ! -------\n\n");
        }
    }
    refresh();
}

void afficheClients() {
    printw("Liste des clients :\n");
    for(int i = 0; i < compteurClients; ++i)
    {
        printw("\t%s\t\t-->\tIP = %s\n", tabClient[i].name.c_str(), tabClient[i].ip.c_str());
    }
    printw("\n");
    refresh();
}