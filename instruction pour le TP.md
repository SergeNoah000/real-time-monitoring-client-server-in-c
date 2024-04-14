# TP Distributed Programming

## Instructions à faire pour le client et le serveur
Le client est chargé d'envoyer les requêtes en permanence au serveur pour l'informer de la liste des fichiers contenus dans le dossier `data`. Le serveur à son tour doit stocker la liste de tous les clients avec leur contenu.
*  Ajout d'un systeme de gestion de logs
    
*  Commenter en anglais et non en francais

*  Expliquer le  niveau de design goal auquel on se trouve avec les TPs hebdomadaires
*  Le dossier se nomme "data"

*  Le format de stockage des informations du ficher au niveau du serveur est:
```markdown
        ADRESSE_IP PORT
            FICHIER1 TAILLE
            FICHIER2 TAILLE
            FICHIER3 TAILLE
            FICHIER4 TAILLE
```
*  Gerer l'encodade et le decodage des noms des fichiers

*  Au niveau du client, un fichier est représenté par une paire <nom, taille>

 *  Envoyer des requetes au serveur quand c'est necessaire(Lorsqu'il y a modfication ajout ou suppression)