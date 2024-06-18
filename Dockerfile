# Utilisation d'une image de base avec SSH et les bibliothèques C pré-installées
FROM ubuntu:latest

# Mettre à jour et installer les bibliothèques C nécessaires
RUN apt-get update && apt-get install -y gcc make  

# Créer un répertoire de travail pour votre application
WORKDIR /app

# Copier vos fichiers C dans l'image
COPY . /app

