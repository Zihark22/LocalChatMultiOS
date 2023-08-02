#include "config.hpp"

void *reception_handler(void *socket_desc);
string saisie(string message);

int main(int argc, char const *argv[]) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    string name, message;

    // Initialisation de ncurses
    initscr(); // initialise ncurses et prépare affichage
    cbreak();  // désactive attente d'un retour à la ligne
    noecho();   // désactive l'affichage auto des caractères saisies
    keypad(stdscr, TRUE); // récupère les touches additionnelles 
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK); // paire 1 = caractere noire sur fond rouge

    name=saisie("Entrez votre nom : ");

    // Création du socket IPv4(AF_INET) de type flux(SOCK_STREAM) en TCP (0) bidirectionel
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printw("\n Erreur de création du socket \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convertion d'une adresse IP au format texte en une représentation binaire dans une structure struct sockaddr_in pour une utilisation ultérieure avec des sockets
    if(inet_pton(AF_INET, IPserveur, &serv_addr.sin_addr) <= 0) {
        printw("\nAdresse invalide/Adresse non supportée \n");
        return -1;
    }

    // Etablie une connexion réseau depuis le socket sock vers l'adresse IP et le port spécifiés dans la structure serv_addr
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printw("\nConnexion échouée \n");
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
    printw("\nClient opérationnel\n");
    printw("Nom client: %s\n",name.c_str());
    printw("PID=%d\n",pidP);
    cout << "IP=" << client_ip << endl;
    printw("Bienvenu dans le chat !\n\n");


    char send_message[TAILLE_MSG+TAILLE_NOM] = {0};
    sprintf(send_message, "%s : %s", name.c_str(), "co");
    send(sock, send_message, strlen(send_message), 0);

    // Création d'un nouveau thread en parallèle au thread principal pour gérer la réception des messages des autres clients dans un environnement multithread
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, reception_handler, (void *)&sock) < 0) {
        perror("Erreur de création d'un processus pour le client");
        exit(EXIT_FAILURE);
    }
    printw("Processus %ld créé pour la communication du serveur\n", (long) thread_id);
    cout << endl;
    cout << "Moi : ";

    while(1)
    {
        message=saisie("Moi : ");

        string send_message = name + " : " + message;

        // Envoye les données contenues dans la chaîne de caractères send_message à travers le socket sock vers le destinataire connecté
        send(sock, send_message.c_str(), strlen(send_message.c_str()), 0);
        cout << "Moi : ";

        if(message==MSG_DECO) {
            endwin(); // Nettoyage de ncurses
            printf("...\nDéconection!\n");
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
    while(1) { 
        valread = read(sock, buffer, TAILLE_BUF); // Bloquant
        refresh(); // Rafraîchir l'écran
        string newbuf(buffer);
        if(newbuf==MSG_DECO)
            break;
        cout << endl;
        cout << "\x1b[1A";
        cout << "\x1b[1L";
        cout << newbuf << endl;
    
        for (int i = 0; i < TAILLE_BUF; i++)
            buffer[i]='\0';
    }

    close(sock);         // destruction du socket client
    pthread_exit(NULL);  // destruction du thread
}

string saisie(string message) {
    int ch;
    string txtSaisie;

    printw(message.c_str());
    refresh();

    while ((ch = getch()) != '\n') {
        if (ch == KEY_BACKSPACE || ch == 127) {
            if (!txtSaisie.empty()) {
                txtSaisie.pop_back();
                printw("\b \b"); // Efface le caractère précédent sur le terminal
                refresh();
            }
        } else if (isprint(ch)) {
            txtSaisie.push_back(ch);
            printw("%c", ch);
            refresh();
        }
    }
    return txtSaisie;
}