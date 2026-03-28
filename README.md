# LocalChatMultiOS
Création d'un chat local via le terminal entre plusieurs ordi avec des OS différents
Le chat se fera via 2 programmes : 1 serveur et 1 client.
Le serveur reçoit les connexions et les messages des clients et les retransmets aux autres.
Le client se connecte au serveur, envoie des messages et reçoit ceux des autres clients.
La communication entre processus se fait via des sockets pour permettre la communication entre plusieurs machines en passant par le réseau.
Elle utilise le protocole TCP qui est plus fiable et sécurisé.

## Versions

- V1 : Connexion d'un client via socket entre client et serveur
- V2 : Connexion de plusieurs clients avec reception du chat sur le serveur
- V3 : Gestion de déconnexion des clients
- V4 : Blocage des clients en trop
- V5 : Envoie du message reçu d'un client aux autres clients (Chat)
- V6 : Login des utilisateurs via un fichier 
- V7 : Passage du réseau local à internet (sécuriser et adapter)

idées : ajout de couleur pour chaque client , passage sous ncurses, voir les autres clients connectés

### V1
Le serveur reçoit un message du client et l'affiche.


### V2
Le serveur reçoit plusieurs clients en même temps


### V3
Les client peuvent se déconnecter en envoyant le message "!deco".

### V4
Enregistrement des clients pour vérifier s'il doit être bloqué

### V5




### V6_C++_ncurses
Chat avec max 4 clients. Chaque client est informé de la connexion des autres clients et reçoit leurs messages.