# LocalChatMultiOS
Création d'un chat local via le terminal entre plusieurs ordi avec des OS différents
Le chat se fera via 2 programmes : 1 serveur et 1 client.
Le serveur reçoit les connexions et les messages des clients et les retransmets aux autres.
Le client se connecte au serveur, envoie des messages et reçoit ceux des autres clients.
La communication entre processus se fait via des sockets pour permettre la communication entre plusieurs machines en passant par le réseau.
Elle utilise le protocole TCP qui est plus fiable et sécurisé.

## Versions

- V1 : Connexion d'un utilisateur avec 1 requête
- V2 = Connexion des utilisateurs et retour du message  "ok" à l'envoyeur
- V3 = Déconnexion enregistrée+blocage des utilisateurs en trop
- V4 = envoie du message aux autres clients
- V5 = simulation de chat (envoie des messages aux autres clients)
- V6 = 


### V1
Le serveur reçoit