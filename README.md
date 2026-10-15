# Chat chiffré RSA en C

Messagerie client-serveur en C où chaque message est chiffré avec **RSA, implémenté de A à Z** : génération de nombres premiers, inverse modulaire, exponentiation modulaire rapide. Aucune bibliothèque de cryptographie n'est utilisée. Projet de deuxième année à l'ISEN Ouest (cycle CIR, cours d'algorithmique avancée).

Il prolonge le [chat client-serveur de première année](https://github.com/Tanguyrnd/chat-c).

> Projet pédagogique : les clés font environ 32 bits et le chiffrement se fait bloc par bloc, sans padding. C'est fait pour comprendre RSA, pas pour protéger de vraies données.

## Compiler et lancer

Linux, `gcc` avec support de C23.

```bash
make
./server          # écoute sur le port 8989, Ctrl+C pour l'arrêter proprement
./client          # dans un autre terminal, autant de fois que voulu
```

## Fonctionnement

1. Au démarrage, le serveur et chaque client génèrent leur propre paire de clés.
2. À la connexion, ils s'échangent leurs clés publiques (en texte, pour éviter les problèmes d'endianness).
3. Le client chiffre ses messages avec la clé publique du serveur. Le serveur les déchiffre, puis les rechiffre avec la clé publique de chaque destinataire avant de les envoyer.

## Organisation du code

```
number_theory.c   pgcd, test de primalité, nombre premier aléatoire, inverse modulaire (Euclide étendu), exponentiation modulaire rapide
rsa.c             génération des clés, chiffrement / déchiffrement d'un bloc, validation d'une clé reçue
rsa_text.c        découpage du texte en blocs de 2 octets, chiffrés puis sérialisés en nombres
secure_channel.c  échange des clés publiques, envoi et réception chiffrés
common.c          envoi complet (boucle sur send) et réception ligne par ligne
client_array.c    tableau dynamique des clients, protégé par un mutex
server.c          un thread détaché par client, diffusion, arrêt propre sur SIGINT
client.c          thread de réception + boucle d'envoi
```

## Points techniques

- **Génération des clés** : `p` et `q` sont tirés entre 2¹⁵ et 2¹⁶, donc `n = p × q` tient sur 32 bits, et un produit de deux valeurs inférieures à `n` tient sur 64 bits. `mod_pow` ne déborde jamais. Si `pgcd(65537, φ(n)) ≠ 1`, on retire une nouvelle paire, sinon l'inverse modulaire n'existerait pas.
- **Exponentiation modulaire rapide** : calcul en O(log e) par élévations au carré successives, avec réduction modulo `n` à chaque étape.
- **Blocs de 2 octets** : deux caractères forment un bloc (valeur ≤ 65 535), toujours strictement inférieur au module.
- **Données venant du réseau** : la taille des messages est bornée, et une clé publique reçue est validée avant d'être utilisée. Un bloc déchiffré trop grand fait rejeter le message.
- **Concurrence** : la liste des clients est protégée par un mutex pendant la diffusion, l'ajout et le retrait. À l'arrêt, `shutdown()` débloque les `recv()` en cours pour que chaque thread se termine.
- **Robustesse TCP** : `send_all` boucle jusqu'à ce que tout soit envoyé, `MSG_NOSIGNAL` évite un `SIGPIPE` si l'autre côté a fermé, et la lecture se fait octet par octet jusqu'au `'\n'`, pour ne jamais lire le début du message suivant.
