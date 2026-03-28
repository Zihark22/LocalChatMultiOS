#include "config.hpp"

// ===========================================================
// Prototypes
// ===========================================================
void *connection_handler(void *);
void fin(int n);
void popClient(string nom, string ip);
void checkClient(int socket_desc, Client &user);
void afficheClients();
void broadcastMessage(const string &message, const string &excludeName, const string &excludeIp);
void broadcastSystemMessage(const string &message);

// ===========================================================
// Variables globales
// ===========================================================
int compteurClients = 0;
Client *tabClient   = NULL;
int server_fd       = -1;

// Mutex pour protéger les accès concurrents à tabClient et compteurClients
pthread_mutex_t mutex_clients = PTHREAD_MUTEX_INITIALIZER;

// ===========================================================
// Main
// ===========================================================
int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt     = 1;
    struct sigaction action;

    // __ Initialisation ncurses ______________________________
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    scrollok(stdscr, TRUE);
    start_color();

    // Paires de couleurs (on évite la paire 0 réservée)
    for (int i = 1; i < 8; i++)
        init_pair(i, i, COLOR_BLACK);

    // __ Gestion des signaux _________________________________
    memset(&action, 0, sizeof(action));
    action.sa_handler = fin;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    // Uniquement les signaux de terminaison
    sigaction(SIGINT,  &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGHUP,  &action, NULL);
    signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE si un client coupe brutalement

    // __ Allocation tableau clients __________________________
    tabClient = (Client *)malloc((NBR_CO_MAX + 1) * sizeof(Client));
    if (tabClient == NULL) {
        printw("Problème allocation\n");
        refresh();
        exit(EXIT_FAILURE);
    }

    // __ Création socket _____________________________________
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Erreur de création du socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Erreur de configuration du socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Binding du socket a échoué");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, NBR_CO_MAX) < 0) {
        perror("Erreur dans l'attente d'une connexion");
        exit(EXIT_FAILURE);
    }

    // __ Affichage infos serveur _____________________________
    pid_t pid = getpid();
    attron(COLOR_PAIR(JAUNE));
    printw("|==================================|\n");
    printw("|       Serveur de chat lancé      |\n");
    printw("|==================================|\n");
    printw("PID  = %d\n", pid);
    printw("PORT = %d\n", PORT);
    printw("En attente de connexions...\n\n");
    attroff(COLOR_PAIR(JAUNE));
    refresh();

    // __ Boucle d'acceptation des connexions _________________
    while (1) {
        refresh();

        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0) {
            perror("Erreur d'acceptation de la connexion");
            continue; // On continue, pas d'exit brutal
        }

        // Vérifier la limite de connexions
        pthread_mutex_lock(&mutex_clients);
        if (compteurClients >= NBR_CO_MAX) {
            pthread_mutex_unlock(&mutex_clients);
            attron(COLOR_PAIR(ROUGE));
            printw("Connexion refusée : nombre maximum de clients atteint (%d)\n", NBR_CO_MAX);
            attroff(COLOR_PAIR(ROUGE));
            refresh();
            // Informer le client et fermer
            string msg = "SERVEUR : Connexion refusée, serveur plein.";
            send(new_socket, msg.c_str(), msg.length(), 0);
            close(new_socket);
            continue;
        }
        pthread_mutex_unlock(&mutex_clients);

        // Récupérer l'IP du client
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);

        attron(COLOR_PAIR(VERT));
        printw("Nouvelle connexion depuis : %s\n", client_ip);
        attroff(COLOR_PAIR(VERT));
        refresh();

        // Allouer les données du client pour le thread
        // On passe une struct pour avoir socket + ip disponibles dans le thread
        Client *newClient    = new Client();
        newClient->socket    = new_socket;
        newClient->ip        = string(client_ip);
        newClient->color     = 0;
        newClient->name      = "";

        // Création du thread
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, connection_handler, (void *)newClient) < 0) {
            perror("Erreur de création du thread");
            delete newClient;
            close(new_socket);
            continue;
        }
        // Détacher le thread pour libérer ses ressources automatiquement
        pthread_detach(thread_id);

        printw("Thread %ld créé pour le client\n\n", (long)thread_id);
        refresh();
    }

    return 0;
}

// ===========================================================
// Handler de connexion client (thread)
// ===========================================================
void *connection_handler(void *arg) {
    // Récupération des données client passées au thread
    Client user    = *(Client *)arg;
    delete (Client *)arg; // Libération de la mémoire allouée dans main

    int sock       = user.socket;
    int valread;
    char buffer[TAILLE_BUF];
    bool registered = false; // true une fois que le client est enregistré

    while (1) {
        memset(buffer, 0, TAILLE_BUF);
        valread = read(sock, buffer, TAILLE_BUF - 1);

        if (valread <= 0) {
            // Connexion perdue brutalement
            pthread_mutex_lock(&mutex_clients);
            if (registered) {
                attron(COLOR_PAIR(ROUGE));
                printw("Client '%s' (%s) perdu (connexion coupée)\n",
                       user.name.c_str(), user.ip.c_str());
                attroff(COLOR_PAIR(ROUGE));
                refresh();

                string sysMsg = "*** " + user.name + " a quitté le chat (connexion perdue) ***\n";
                popClient(user.name, user.ip);
                pthread_mutex_unlock(&mutex_clients);

                broadcastSystemMessage(sysMsg);

                pthread_mutex_lock(&mutex_clients);
                // Fermer le chat si plus personne
                if (compteurClients == 0) {
                    pthread_mutex_unlock(&mutex_clients);
                    attron(COLOR_PAIR(JAUNE));
                    printw("\n--- Plus aucun client connecté ---\n\n");
                    attroff(COLOR_PAIR(JAUNE));
                    refresh();
                } else {
                    pthread_mutex_unlock(&mutex_clients);
                }
            } else {
                pthread_mutex_unlock(&mutex_clients);
            }
            break;
        }

        string newbuf(buffer);

        // Trouver le délimiteur " : "
        size_t delimiterPos = newbuf.find(" : ");
        if (delimiterPos == string::npos) {
            // Message mal formé, on ignore
            continue;
        }

        string nom = newbuf.substr(0, delimiterPos);
        string msg = newbuf.substr(delimiterPos + 3); // +3 pour " : "

        // Suppression du '\n' final éventuel
        if (!msg.empty() && msg.back() == '\n')
            msg.pop_back();

        // __ Premier message : enregistrement du client ______
        if (msg == "co") {
            pthread_mutex_lock(&mutex_clients);
            user.name = nom;

            // Vérifier si déjà enregistré (reconnexion ?)
            bool exists = false;
            for (int i = 0; i < compteurClients; i++) {
                if (tabClient[i].name == user.name && tabClient[i].ip == user.ip) {
                    exists = true;
                    user   = tabClient[i];
                    break;
                }
            }

            if (!exists) {
                user.color = (compteurClients % 6) + 1; // Couleur cyclique (1-6)
                tabClient[compteurClients] = user;
                compteurClients++;
                registered = true;

                attron(COLOR_PAIR(VERT));
                printw("Nouveau client enregistré : %s (%s)\n",
                       user.name.c_str(), user.ip.c_str());
                attroff(COLOR_PAIR(VERT));
                afficheClients();

                if (compteurClients == 2) {
                    attron(COLOR_PAIR(JAUNE));
                    printw("====== Début du chat ! ======\n\n");
                    attroff(COLOR_PAIR(JAUNE));
                }
            } else {
                registered = true;
            }
            pthread_mutex_unlock(&mutex_clients);

            // Notifier tous les autres de la connexion
            string sysMsg = "*** " + nom + " a rejoint le chat ***\n";
            broadcastSystemMessage(sysMsg);

            refresh();
            continue;
        }

        // __ Message de déconnexion volontaire _______________
        if (msg == MSG_DECO) {
            pthread_mutex_lock(&mutex_clients);
            attron(COLOR_PAIR(ROUGE));
            printw("Client '%s' déconnecté proprement\n", nom.c_str());
            attroff(COLOR_PAIR(ROUGE));

            popClient(user.name, user.ip);
            int remaining = compteurClients;
            pthread_mutex_unlock(&mutex_clients);

            // Confirmer la déconnexion au client
            send(sock, MSG_DECO, strlen(MSG_DECO), 0);

            // Notifier tous les autres de la déconnexion
            string sysMsg = "*** " + nom + " a quitté le chat ***\n";
            broadcastSystemMessage(sysMsg);

            pthread_mutex_lock(&mutex_clients);
            afficheClients();
            pthread_mutex_unlock(&mutex_clients);

            // Fermer le chat si plus personne
            if (remaining == 0) {
                attron(COLOR_PAIR(JAUNE));
                printw("\n====== Plus aucun client - Fin du chat ======\n\n");
                attroff(COLOR_PAIR(JAUNE));
                refresh();
                //  On ne ferme PAS le serveur, il reste en écoute
            }

            refresh();
            break;
        }

        // __ Message normal ___________________________________
        pthread_mutex_lock(&mutex_clients);
        // Mettre à jour user depuis tabClient (pour avoir la bonne couleur)
        for (int i = 0; i < compteurClients; i++) {
            if (tabClient[i].name == user.name && tabClient[i].ip == user.ip) {
                user = tabClient[i];
                break;
            }
        }
        pthread_mutex_unlock(&mutex_clients);

        // Affichage serveur
        attron(COLOR_PAIR(user.color));
        printw("%s", nom.c_str());
        attroff(COLOR_PAIR(user.color));
        printw(" : %s\n", msg.c_str());
        refresh();

        // Diffusion aux autres clients
        string fullMsg = nom + " : " + msg + "\n";
        broadcastMessage(fullMsg, user.name, user.ip);
    }

    close(sock);
    pthread_exit(NULL);
}

// ===========================================================
// Diffuse un message à tous les clients sauf l'expéditeur
// ===========================================================
void broadcastMessage(const string &message, const string &excludeName,
                      const string &excludeIp) {
    pthread_mutex_lock(&mutex_clients);
    for (int i = 0; i < compteurClients; i++) {
        if (tabClient[i].name != excludeName || tabClient[i].ip != excludeIp) {
            send(tabClient[i].socket, message.c_str(), message.length(), 0);
        }
    }
    pthread_mutex_unlock(&mutex_clients);
}

// ===========================================================
// Diffuse un message système à TOUS les clients
// ===========================================================
void broadcastSystemMessage(const string &message) {
    pthread_mutex_lock(&mutex_clients);
    for (int i = 0; i < compteurClients; i++) {
        send(tabClient[i].socket, message.c_str(), message.length(), 0);
    }
    pthread_mutex_unlock(&mutex_clients);
}

// ===========================================================
// Supprime un client du tableau (à appeler avec mutex verrouillé)
// ===========================================================
void popClient(string nom, string ip) {
    int indice = -1;
    for (int i = 0; i < compteurClients; i++) {
        if (tabClient[i].name == nom && tabClient[i].ip == ip) {
            indice = i;
            break;
        }
    }
    if (indice == -1) return; // Client non trouvé

    // Décalage du tableau
    for (int i = indice; i < compteurClients - 1; i++)
        tabClient[i] = tabClient[i + 1];

    compteurClients--;
}

// ===========================================================
// Affiche la liste des clients (à appeler avec mutex verrouillé)
// ===========================================================
void afficheClients() {
    attron(COLOR_PAIR(CYAN));
    printw("|- Clients connectés (%d/%d) -|\n", compteurClients, NBR_CO_MAX);
    for (int i = 0; i < compteurClients; i++) {
        printw("|  %s => IP: %s\n",tabClient[i].name.c_str(), tabClient[i].ip.c_str());
    }
    printw("|___________________________|\n\n");
    attroff(COLOR_PAIR(CYAN));
    refresh();
}

// ===========================================================
// Fin propre du serveur
// ===========================================================
void fin(int n) {
    pthread_mutex_lock(&mutex_clients);

    // Informer tous les clients de l'arrêt
    string msgFin = "*** Serveur arrêté ***";
    for (int i = 0; i < compteurClients; i++) {
        send(tabClient[i].socket, msgFin.c_str(), msgFin.length(), 0);
        close(tabClient[i].socket);
    }

    free(tabClient);
    tabClient = NULL;
    pthread_mutex_unlock(&mutex_clients);

    if (server_fd != -1)
        close(server_fd);

    pthread_mutex_destroy(&mutex_clients);

    endwin();
    printf("\nServeur arrêté proprement.\n");
    exit(EXIT_SUCCESS);
}
