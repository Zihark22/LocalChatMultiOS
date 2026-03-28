#include "config.hpp"

void *reception_handler(void *socket_desc);
string saisie(string message);
void fin(int n);

int sock;
string name;

int main(int argc, char const *argv[]) {
    struct sockaddr_in serv_addr;
    string message;
    struct sigaction action;

    sock = 0;

    // ===========================================
    // Initialisation de ncurses
    // ===========================================
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    scrollok(stdscr, TRUE);
    start_color();

    // Initialisation des paires de couleurs (on évite la paire 0 car réservée)
    for (int i = 1; i < 8; i++) {
        init_pair(i, i, COLOR_BLACK);
    }

    // ===========================================
    // Gestion des signaux - UNIQUEMENT les signaux utiles
    // ===========================================
    memset(&action, 0, sizeof(action));
    action.sa_handler = fin;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    // ⚠️ On n'installe le handler QUE sur les signaux de terminaison
    // Ne pas toucher à SIGCHLD, SIGWINCH, SIGPIPE etc.
    sigaction(SIGINT,  &action, NULL);  // Ctrl+C
    sigaction(SIGTERM, &action, NULL);  // kill
    sigaction(SIGHUP,  &action, NULL);  // terminal fermé

    // Ignorer SIGPIPE pour éviter crash si le serveur ferme la connexion
    signal(SIGPIPE, SIG_IGN);

    // ===========================================
    // Saisie du nom
    // ===========================================
    name = saisie("Entrez votre nom : ");
    refresh();

    // ===========================================
    // Création du socket
    // ===========================================
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printw("\nErreur de création du socket\n");
        refresh();
        getch(); // Pause pour voir le message
        endwin();
        return -1;
    }

    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_port        = htons(PORT);
    memset(&serv_addr.sin_zero, 0, sizeof(serv_addr.sin_zero));

    if (inet_pton(AF_INET, IPserveur, &serv_addr.sin_addr) <= 0) {
        printw("\nAdresse invalide ou non supportée : %s\n", IPserveur);
        refresh();
        getch();
        endwin();
        return -1;
    }

    // ===========================================
    // Connexion au serveur
    // ===========================================
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        // On sort de ncurses pour afficher l'erreur système
        endwin();
        perror("Connexion échouée");
        return -1;
    }

    // ===========================================
    // Récupération de l'IP locale du client
    // ===========================================
    struct sockaddr_in local_addr;
    socklen_t addr_length = sizeof(local_addr);
    char client_ip[INET_ADDRSTRLEN] = "Inconnue";

    // ⚠️ getsockname() donne l'adresse LOCALE, pas celle du serveur
    if (getsockname(sock, (struct sockaddr *)&local_addr, &addr_length) == 0) {
        inet_ntop(AF_INET, &(local_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    }

    pid_t pidP = getpid();

    // ===========================================
    // Affichage des informations de connexion
    // ===========================================
    clear(); // Efface l'écran pour repartir proprement
    attron(COLOR_PAIR(JAUNE));
    printw("|==============================|\n");
    printw("|     Client opérationnel      |\n");
    printw("|==============================|\n");
    printw("| Nom    : %-20s|\n", name.c_str());
    printw("| PID    : %-20d|\n", pidP);
    printw("| IP     : %-20s|\n", client_ip);
    printw("| Serveur: %-20s|\n", IPserveur);
    printw("|==============================|\n");
    printw("Bienvenu dans le chat !\n\n");
    attroff(COLOR_PAIR(JAUNE));
    refresh(); // ⚠️ Indispensable pour afficher

    // ===========================================
    // Envoi du message de connexion au serveur
    // ===========================================
    char send_buf[TAILLE_MSG + TAILLE_NOM] = {0};
    snprintf(send_buf, sizeof(send_buf), "%s : co", name.c_str()); // ✅ snprintf plus sûr que sprintf
    send(sock, send_buf, strlen(send_buf), 0);

    // ===========================================
    // Création du thread de réception
    // ===========================================
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, reception_handler, (void *)&sock) < 0) {
        endwin();
        perror("Erreur de création du thread de réception");
        exit(EXIT_FAILURE);
    }
    pthread_detach(thread_id); // Le thread se nettoie tout seul à sa fin

    printw("Thread %lu créé pour la réception\n\n", (unsigned long int)thread_id);
    refresh();

    // ===========================================
    // Boucle principale de saisie et envoi
    // ===========================================
    while (1) {
        attron(COLOR_PAIR(ROUGE));
        message = saisie("Moi : ");
        attroff(COLOR_PAIR(ROUGE));

        if (message.empty())    // On ignore les messages vides
            continue;

        if (message == MSG_DECO)
            break;

        // Construction et envoi du message
        string full_message = name + " : " + message;
        send(sock, full_message.c_str(), full_message.length(), 0);
    }

    fin(0);
    return 0;
}

// ===========================================================
// Thread de réception des messages du serveur
// ===========================================================
void *reception_handler(void *socket_desc) {
    int socket = *(int *)socket_desc;
    int valread;
    char buffer[TAILLE_BUF];

    while (1) {
        memset(buffer, 0, TAILLE_BUF); // ✅ Nettoyage avant lecture
        valread = read(socket, buffer, TAILLE_BUF - 1); // -1 pour garder le \0 final

        // Gestion de la déconnexion ou erreur
        if (valread <= 0) {
            break;
        }

        string newbuf(buffer);

        if (newbuf == MSG_DECO)
            break;

        // ─────────────────────────────────────────
        // Affichage du message reçu
        // On efface la ligne "Moi : " en cours avant d'afficher
        // ─────────────────────────────────────────
        int y, x;
        getyx(stdscr, y, x);

        move(y, 0);
        clrtoeol(); // Efface la ligne courante (le "Moi : " en cours)

        attron(COLOR_PAIR(BLEU));
        printw("%s\n", newbuf.c_str());
        attroff(COLOR_PAIR(BLEU));

        // Ré-affiche le prompt de saisie
        attron(COLOR_PAIR(ROUGE));
        printw("Moi : ");
        attroff(COLOR_PAIR(ROUGE));

        refresh();
    }

    close(socket);
    pthread_exit(NULL);
}

// ===========================================================
// Fonction de saisie avec ncurses
// ===========================================================
string saisie(string message) {
    int ch;
    string txtSaisie;

    printw("%s", message.c_str());
    refresh();

    while ((ch = getch()) != '\n') {
        if (ch == ERR)  // getch() a retourné une erreur
            continue;

        if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
            if (!txtSaisie.empty()) {
                txtSaisie.pop_back();
                int y, x;
                getyx(stdscr, y, x);
                move(y, x - 1);
                delch();
                refresh();
            }
        } else if (isprint(ch)) {
            txtSaisie.push_back((char)ch);
            printw("%c", ch);
            refresh();
        }
    }

    printw("\n");
    refresh();

    return txtSaisie;
}

// ===========================================================
// Gestion de la fin propre du programme
// ===========================================================
void fin(int n) {
    // Envoi du message de déconnexion si le socket est ouvert
    if (sock > 0) {
        string msgFin = name + " : " + MSG_DECO;
        send(sock, msgFin.c_str(), msgFin.length(), 0);
        close(sock);
    }

    endwin(); // ✅ Nettoyage ncurses avant printf
    printf("\nDéconnexion...\n");
    exit(EXIT_SUCCESS);
}
