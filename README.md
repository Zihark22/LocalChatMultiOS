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


### V1
Le serveur reçoit un message du client et l'affiche.


### V2
Le serveur reçoit plusieurs clients en même temps


### V3
Les client peuvent se déconnecter en envoyant le message "!deco".

### V4
Enregistrement des clients pour vérifier s'il doit être bloqué


## Explications du Code V4
Ce code est un serveur C++ qui gère les connexions de plusieurs clients et permet un chat entre eux. Voici un aperçu détaillé de son fonctionnement global :

1. Le serveur commence par initialiser des variables et des structures nécessaires pour gérer les connexions clients et stocker les informations associées aux clients.

2. En utilisant la fonction `sigaction`, il configure le traitement des signaux pour gérer proprement la terminaison du serveur.

3. Le serveur crée un socket TCP IPv4 en utilisant `socket()` et configure le socket pour réutiliser une adresse locale avec l'option `SO_REUSEADDR`.

4. Il associe une adresse au socket à l'aide de `bind()` pour qu'il puisse écouter les connexions entrantes sur une adresse spécifique.

5. Le serveur lance l'écoute des connexions entrantes avec `listen()` et attend qu'un client se connecte à l'aide de `accept()`.

6. Lorsqu'un client se connecte, le serveur récupère l'adresse IP du client en utilisant `inet_ntop()` et affiche l'adresse IP du client connecté.

7. Le serveur crée ensuite un nouveau thread pour gérer la communication avec le client connecté, en utilisant `pthread_create()` et la fonction `connection_handler`.

8. La fonction `connection_handler` est responsable de la communication avec le client dans le thread créé. Elle lit les messages envoyés par le client à l'aide de `read()` et les traite en fonction du contenu du message.

9. Dans la fonction `connection_handler`, chaque message reçu du client est analysé pour extraire le nom du client et son message. La fonction `find()` est utilisée pour trouver la position du délimiteur `:` dans le message, et `substr()` est utilisée pour extraire le nom et le message de la chaîne complète.

10. La fonction `checkClient()` vérifie si le client est déjà présent dans la liste des clients connectés. Si ce n'est pas le cas, le client est ajouté à la liste.

11. Si le message envoyé par le client est égal à la constante `MSG_DECO`, le serveur considère que le client souhaite se déconnecter, et il supprime le client de la liste des clients connectés en appelant `popClient()`.

12. Le serveur diffuse ensuite le message du client à tous les autres clients connectés en utilisant une boucle qui parcourt la liste des clients connectés et envoie le message à chaque client connecté avec `send()`.

13. Si le nombre maximum de clients (`NBR_CO_MAX`) est atteint, le serveur refuse de nouvelles connexions.

14. Lorsqu'un client se déconnecte, le socket associé à ce client est fermé en appelant `close()`.

15. Le serveur affiche également la liste des clients connectés à l'aide de la fonction `afficheClients()`.

16. Le serveur continue d'accepter de nouvelles connexions et de gérer les clients existants en bouclant indéfiniment dans la boucle `while(1)`.

17. La fonction `fin()` est appelée lorsque le serveur est terminé. Elle affiche un message et termine proprement le programme.

Cet exemple montre comment mettre en place un chat simple en C++ en utilisant des threads pour gérer les connexions de plusieurs clients simultanément. Vous pouvez l'utiliser comme base pour créer un chat plus sophistiqué avec des fonctionnalités supplémentaires.
