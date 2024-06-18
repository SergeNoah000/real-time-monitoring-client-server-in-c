#include "../include/client.h"
#include <errno.h>
#include <netdb.h>


#define PIECE_SIZE 262144


typedef struct {
    int index;
    size_t length;
    char *data;
} Piece;


// Structure pour stocker les informations du fichier .hash
typedef struct {
    int index;
    char hash[65]; // 64 caractères pour le hash + 1 caractère de fin de chaîne
    int received;
} HashEntry;



// //convert time to a valid  string
// void convert_time(char *str,  size_t size, time_t time){
//     struct tm *tmp = localtime(&time);
//     strftime(str, size, "%^a %d-%m-%Y %H:%M:%S", tmp);
// }



// // Function to log an action with its corresponding danger level
// void log_action(const char *action, char* danger) {
//     FILE *log_file = fopen(LOG_FILE, "a");
//     if (log_file == NULL) {
//         perror("Open log file failled");
//         exit(EXIT_FAILURE);
//     }

//     time_t date_t = time(NULL);
//     char *str_date = malloc(sizeof(char) * 30);
//     convert_time(str_date, sizeof(date_t ) * 4, date_t);
//     fprintf(log_file, "[ %s ] %s: %s \n",str_date, danger, action );
//     fclose(log_file);
//     free(str_date);
// }


// Fonction pour lire le fichier .hash
HashEntry* read_hash_file(const char* filename, int* count) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        return NULL;
    }

    HashEntry* entries = NULL;
    *count = 0;

    while (!feof(file)) {
        HashEntry entry;
        if (fscanf(file, "%d %64s %d", &entry.index, entry.hash, &entry.received) == 3) {
            entries = realloc(entries, (*count + 1) * sizeof(HashEntry));
            if (entries == NULL) {
                perror("realloc");
                fclose(file);
                return NULL;
            }
            entries[*count] = entry;
            (*count)++;
        }
    }

    fclose(file);
    return entries;
}

// Fonction pour écrire dans le fichier .hash
int write_hash_file(const char* filename, HashEntry* entries, int count) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("fopen");
        return -1;
    }

    for (int i = 0; i < count; i++) {
        fprintf(file, "%d %s %d\n", entries[i].index, entries[i].hash, entries[i].received);
    }

    fclose(file);
    return 0;
}

// Fonction pour vérifier si une pièce est déjà reçue
int is_piece_received(HashEntry* entries, int count, int index) {
    for (int i = 0; i < count; i++) {
        if (entries[i].index == index) {
            return entries[i].received;
        }
    }
    return 0;
}

// Fonction pour mettre à jour le statut d'une pièce
int update_piece_status(HashEntry* entries, int count, int index) {
    for (int i = 0; i < count; i++) {
        if (entries[i].index == index) {
            entries[i].received = 1;
            return 0;
        }
    }
    return -1;
}


int calculate_hash(const char *data, size_t length, unsigned char *hash) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, length);
    SHA256_Final(hash, &sha256);
    return 0;
}

void save_piece_to_file(const char *filename, const Piece *piece) {
    char path[1024];
    sprintf(path, "Downloads/_%s/%d.hash", filename, piece->index);

    if (!access(path, F_OK)) {
        char message[256];
        sprintf(message, "Downloads/_%s/ does not exist when the pieces are completely received", filename);
        perror(message);
        exit(-1);
    }

    FILE *file = fopen(path, "wb");
    if (!file) {
        perror("File open failed");
        return;
    }
    fwrite(piece->data, 1, piece->length, file);
    fclose(file);
}

int save_hash_list(const char *filename, unsigned char **hashes, int piece_count, unsigned char *global_hash, size_t file_size) {
    char path[1024];
    sprintf(path, "Downloads/_%s/.hash", filename);

    if (access(path, F_OK) != 0) {
        char dir_path[1024];
        sprintf(dir_path, "Downloads/_%s/", filename);
        mkdir(dir_path, 0777);
    }

    FILE *file = fopen(path, "wb");
    if (!file) {
        perror("File open failed");
        return -1;
    }
    fwrite(&file_size, sizeof(file_size), 1, file);
    fwrite(&piece_count, sizeof(piece_count), 1, file);
    fwrite(global_hash, SHA256_DIGEST_LENGTH, 1, file);
    for (int i = 0; i < piece_count; i++) {
        fwrite(hashes[i], SHA256_DIGEST_LENGTH, 1, file);
    }
    fclose(file);
    return 0;
}

int load_hash_list(const char *filename, unsigned char ***hashes, int *piece_count, unsigned char *global_hash, size_t *file_size) {
    // printf("Ok pour la fonction load_hash_list\n\n");
    char path[1024];
    sprintf(path, "Downloads/_%s/.hash", filename);
    struct stat st;
    if (stat(path, &st) != 0) {
        perror("Fichier .hash non trouvé pourtant déclaré présent");
        return -1;
    }

    FILE *file = fopen(path, "rb");
    if (!file) {
        perror("File .hash open failed");
        return -1;
    }

    fread(file_size, sizeof(*file_size), 1, file);
    fread(piece_count, sizeof(*piece_count), 1, file);
    *hashes = malloc(*piece_count * sizeof(unsigned char *));
    fread(global_hash, SHA256_DIGEST_LENGTH, 1, file);
    for (int i = 0; i < *piece_count; i++) {
        (*hashes)[i] = malloc(SHA256_DIGEST_LENGTH);
        fread((*hashes)[i], SHA256_DIGEST_LENGTH, 1, file);
    }
    fclose(file);
    return 0;
}

int reconstruct_file(const char *filename, int piece_size, int piece_count) {
    char path[1024];
    sprintf(path, "Downloads/%s", filename);
    FILE *file = fopen(path, "wb");
    if (!file) {
        perror("File open failed");
        return -1 ;
    }
    for (int i = 0; i < piece_count; i++) {
        sprintf(path, "Downloads/_%s/%d.hash", filename, i);
        FILE *piece_file = fopen(path, "rb");
        if (!piece_file) {
            perror("Piece file open failed");
            return -1;
        }
        char buffer[piece_size];
        size_t length = fread(buffer, 1, piece_size, piece_file);
        fwrite(buffer, 1, length, file);
        fclose(piece_file);
    }
    fclose(file);
    return 0;
}

int receive_hashes(int sockfd, unsigned char ***hashes, int *piece_count, size_t *file_size, unsigned char *global_hash) {
    // printf("OK pour receive_hashes\n\n");
    char request_type = 'L';
    send(sockfd, &request_type, sizeof(request_type), 0);
    // printf("Envoi de la requête L: Ok\n\n");

    recv(sockfd, piece_count, sizeof(*piece_count), 0);
    recv(sockfd, file_size, sizeof(*file_size), 0);
    *hashes = malloc(*piece_count * sizeof(unsigned char *));
    // printf("Début de la réception de la liste des hashes L, leur nombre est %d: Ok\n\n", *piece_count);
    for (int i = 0; i < *piece_count; i++) {
        (*hashes)[i] = malloc(SHA256_DIGEST_LENGTH);
        recv(sockfd, (*hashes)[i], SHA256_DIGEST_LENGTH, 0);
//         printf("Reçu hash[%d]: ", i);
//         for (int j = 0; j < SHA256_DIGEST_LENGTH; j++) {
//             printf("%02x", (*hashes)[i][j]);
//         }
//         printf(" Ok\n\n");
    }
    //  printf("Réponse de la requête de liste L reçue : Ok\n\n");
    request_type = 'H';
    send(sockfd, &request_type, sizeof(request_type), 0);
    // printf("Envoi de la requête H: Ok\n\n");
    recv(sockfd, global_hash, SHA256_DIGEST_LENGTH, 0);
    // printf("Réponse de la requête H reçue : Ok\n\n");
//     for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
//         printf("%02x", global_hash[i]);
//     }
//     printf("\n\n");
    return 0;
}


void request_piece(int sockfd, int index) {
    // printf("Envoie de la requête P puis celle de  index = %d : Ok\n\n", index);
    char request_type = 'P';
    send(sockfd, &request_type, sizeof(request_type), 0);
    send(sockfd, &index, sizeof(index), 0);
    // printf("Envoi fait  index = %d : Ok\n\n", index);
}

Piece receive_piece(int sockfd) {
    // printf("Dans receive_piece : Ok\n");

    Piece piece;
    int index;
    size_t length;
    // printf("Réponse de la requête P : Ok\n");

    // Initialisation du buffer de données
    char *data = malloc(PIECE_SIZE);
    if (data == NULL) {
        fprintf(stderr, "Erreur d'allocation de mémoire\n");
        piece.data = NULL;
        return piece;
    }

    if (recv(sockfd, &index, sizeof(index), 0) != sizeof(index)) {
        perror("Failed to receive piece index");
        free(data);
        piece.data = NULL;
        return piece;
    }
    // printf("Index reçu : %d\n", index);
    piece.index = index;

    if (recv(sockfd, &length, sizeof(length), 0) != sizeof(length)) {
        perror("Failed to receive piece length");
        free(data);
        piece.data = NULL;
        return piece;
    }
    // printf("Taille des données reçue : %ld\n", length);
    piece.length = length;

    // Réception des données de la pièce
    ssize_t total_received = 0;
    while ((size_t)total_received < length) {
        ssize_t len = recv(sockfd, data + total_received, length - total_received, 0);
        if (len <= 0) {
            perror("Failed to receive piece data");
            free(data);
            piece.data = NULL;
            return piece;
        }
        total_received += len;
    }
    // printf("Données reçues : %ld Ok\n", piece.length);

    unsigned char received_hash[SHA256_DIGEST_LENGTH];
    if (recv(sockfd, received_hash, SHA256_DIGEST_LENGTH, 0) != SHA256_DIGEST_LENGTH) {
        perror("Failed to receive piece hash");
        free(data);
        piece.data = NULL;
        return piece;
    }
    // printf("Hash reçu : ");
    // for (int j = 0; j < SHA256_DIGEST_LENGTH; j++) {
    //     printf("%02x", received_hash[j]);
    // }
    // printf(" Ok \n");

    unsigned char calculated_hash[SHA256_DIGEST_LENGTH];
    // printf("Calcul du hash : ");
    calculate_hash(data, piece.length, calculated_hash);
    // for (int j = 0; j < SHA256_DIGEST_LENGTH; j++) {
    //     printf("%02x", calculated_hash[j]);
    // }
    // printf(" Ok\n");

    // printf("Comparaison des hashes\n");
    if (memcmp(received_hash, calculated_hash, SHA256_DIGEST_LENGTH) != 0) {
        fprintf(stderr, "Les hash ne correspondent pas. %d\n", piece.index);
        printf("Hash recu: ");
        for (int j = 0; j < SHA256_DIGEST_LENGTH; j++) {
            printf("%02x", received_hash[j]);
        }
        printf(" \n");

        free(data);
        piece.data = NULL;
    } else {
        // printf("Les deux hashes sont egaux : Ok\n\n");
        piece.data = data;
    }

    return piece;
}


int check_and_request_piece(int sockfd, const char *filename, int index, unsigned char **hashes) {
    char path[1024];
    sprintf(path, "Downloads/_%s/%d.hash", filename, index);

    if (access(path, F_OK) == 0) {
        // Le fichier existe, calculer son hash et vérifier
        FILE *file = fopen(path, "rb");
        if (!file) {
            perror("File open failed");
            return -1;
        }

        char buffer[PIECE_SIZE];
        size_t length = fread(buffer, 1, PIECE_SIZE, file);
        fclose(file);

        unsigned char calculated_hash[SHA256_DIGEST_LENGTH];
        calculate_hash(buffer, length, calculated_hash);

        // Comparer le hash calculé avec le hash en mémoire
        if (memcmp(calculated_hash, hashes[index], SHA256_DIGEST_LENGTH) == 0) {
            // Le hash correspond, la pièce est valide
            // printf("Piece %d is already received and valid.\n", index);
            return 0;
        } else {
            // printf("Piece %d exists but is invalid, will request it.\n", index);
        }
    } else {
        // printf("Piece %d does not exist, will request it.\n", index);
    }

    // Envoyer la requête pour récupérer la pièce
    request_piece(sockfd, index);
    return 1;
}


int remove_directory(const char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("opendir");
        return -1;
    }

    int result = 0;
    while ((entry = readdir(dir)) != NULL) {
        char full_path[1024];
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) == -1) {
            perror("stat");
            result = -1;
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            // If the entry is a directory, recursively remove it
            if (remove_directory(full_path) == -1) {
                result = -1;
            }
        } else {
            // If the entry is a file, remove it
            if (unlink(full_path) == -1) {
                perror("unlink");
                result = -1;
            }
        }
    }

    closedir(dir);

    // Finally, remove the directory itself
    if (rmdir(path) == -1) {
        perror("rmdir");
        result = -1;
    }

    return result;
}

void calculate_global_hash(const char *filename, int piece_size, int piece_count, unsigned char *global_hash) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    char path[256];
    for (int i = 0; i < piece_count; i++) {
        sprintf(path, "Downloads/_%s/%d.hash", filename, i);
        FILE *file = fopen(path, "rb");
        if (!file) {
            perror("Piece file open failed");
            exit(EXIT_FAILURE);
        }
        char buffer[piece_size];
        size_t length = fread(buffer, 1, piece_size, file);
        SHA256_Update(&sha256, buffer, length);
        fclose(file);
    }
    SHA256_Final(global_hash, &sha256);
}


void print_progress(size_t received_bytes, size_t file_size) {
    // Calculer le pourcentage de progression
    float progress = ((float)received_bytes / (float)file_size) * 100;
    printf("\033[%dA\033[K", 1);

    // Afficher la barre de progression
    int bar_width = 50;
    int pos = (bar_width * progress) / 100;
        printf("\r[ ");
        for (int i = 0; i < bar_width; ++i) {
            if (i < pos) printf("▒"); // ▒ , ▓ , ░
            else if (i == pos) printf("_");
            else printf(" ");
        }
        printf(" ] %0.2f%% \n", progress);
    // fflush(stdout);
}


size_t receive_file_with_bittorent(const char *filename, const int sockfd) {
    int piece_count;
    unsigned char **hashes;
    unsigned char global_hash[SHA256_DIGEST_LENGTH];
    size_t file_size, received_bytes = 0;
    char chemin[256] ;
    sprintf(chemin, "Downloads/_%s/", filename);
    if (access(chemin, F_OK) == 0) {
        if (load_hash_list(filename, &hashes, &piece_count, global_hash, &file_size) != 0) {
            log_action("Failed to load the hash list from file", "Error");
            perror("Failed to load the hash list from file");
            return -1;
        }
        // printf("Hash list loaded\n\n");
    } else {
        printf("Wait for computing the hash list on sender side.\n\n");
        if (receive_hashes(sockfd, &hashes, &piece_count, &file_size, global_hash) !=0){
            log_action("Failed to receive_hashes from sender", "Error");
            perror("Failed to receive hashes from sender");
            return -1;
        }
        if(save_hash_list(filename, hashes, piece_count, global_hash, file_size) !=0){
            log_action("Failed to save the hash_list hash in file", "Error");
            perror("Failed to save the hash_list hash in file");
            return -1;
        }
        // printf("Hash list received\n\n");
    }

    printf("Debut \n");
    for (int i = 0; i < piece_count; i++) {
        if (check_and_request_piece(sockfd, filename, i, hashes) == 1) {
            Piece piece = receive_piece(sockfd);
            if (piece.data != NULL && piece.length > 0) {
                save_piece_to_file(filename, &piece);
                received_bytes += piece.length;
                print_progress(received_bytes, file_size);
            } else {
                fprintf(stderr, "Failed to receive piece %d\n", i);
                break;
            }
        }else{
            received_bytes += PIECE_SIZE;
        }
    }

    if (reconstruct_file(filename, PIECE_SIZE, piece_count) != 0) {
            log_action("Failed toreconstruct file ", "Error");
            perror("Failed to reconstruct file");
            return -1;
    }

    unsigned char calculated_global_hash[SHA256_DIGEST_LENGTH];
    calculate_global_hash(filename, PIECE_SIZE, piece_count, calculated_global_hash);
    if (memcmp(calculated_global_hash, global_hash, SHA256_DIGEST_LENGTH) == 0) {
        printf("File transfert completed successfully.🎉🎉🎉🥳🥳🎊🎁 ");
        char new_name[1024];
        
        snprintf(new_name, sizeof(new_name), "Downloads/_%s", filename);
        if (remove_directory(new_name) == 0) {
            printf("Directory '%s' removed successfully.\n", new_name);
        } else {
            perror("remove_directory");
            printf("Failed to remove directory '%s'.\n", new_name);
        }
        char request_type = 'E';
        send(sockfd, &request_type, sizeof(request_type), 0);
        
    } else {
        printf("File transfer failed due to hash mismatch.\n");
    }

    close(sockfd);
    return received_bytes + sizeof(hashes) + sizeof(global_hash);
}

int main_receiver(int argc, char const *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <hostname> <port> <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *hostname = argv[1];
    int port = atoi(argv[2]);
    const char *filename = argv[3];

    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket créée avec succès\n\n");

    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "No such host\n");
        exit(EXIT_FAILURE);
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy((char *)&server_addr.sin_addr.s_addr, (char *)server->h_addr_list[0], server->h_length);
    server_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    printf("Connexion à l'envoyeur réussie !\n\n");

    int piece_count;
    unsigned char **hashes;
    unsigned char global_hash[SHA256_DIGEST_LENGTH];
    char chemin[256] ;
    size_t file_size;
    sprintf(chemin, "Downloads/_%s/", filename);
    if (access(chemin, F_OK) == 0) {
        if (load_hash_list(filename, &hashes, &piece_count, global_hash, &file_size) != 0) {
            log_action("Failed to load the hash list from file", "Error");
            perror("Failed to load the hash list from file");
            return -1;
        }
        printf("Hash list loaded\n\n");
    } else {
        printf("Hash list not found,  need to request it.\n\n");
        if (receive_hashes(sockfd, &hashes, &piece_count, &file_size,  global_hash) !=0){
            log_action("Failed to receive_hashes frpm sender", "Error");
            perror("Failed to receive hashes from sender");
            return -1;
        }
        if(save_hash_list(filename, hashes, piece_count, global_hash, file_size) !=0){
            log_action("Failed to save the hash_list hash in file", "Error");
            perror("Failed to save the hash_list hash in file");
            return -1;
        }
    }
    size_t received_bytes = 0;
    printf("Debut \n");
    for (int i = 0; i < piece_count; i++) {
        if (check_and_request_piece(sockfd, filename, i, hashes) == 1) {
            Piece piece = receive_piece(sockfd);
            if (piece.data != NULL && piece.length > 0) {
                save_piece_to_file(filename, &piece);
                received_bytes += piece.length;
                print_progress(received_bytes, file_size);
            } else {
                fprintf(stderr, "Failed to receive piece %d\n", i);
                break;
            }
        }else{
            received_bytes += PIECE_SIZE;
        }
    }

    if (reconstruct_file(filename, PIECE_SIZE, piece_count) != 0) {
            log_action("Failed toreconstruct file ", "Error");
            perror("Failed to reconstruct file");
            return -1;
    }

    unsigned char calculated_global_hash[SHA256_DIGEST_LENGTH];
    calculate_global_hash(filename, PIECE_SIZE, piece_count, calculated_global_hash);
    if (memcmp(calculated_global_hash, global_hash, SHA256_DIGEST_LENGTH) == 0) {
        printf("File transfer completed successfully.🎉🎉🎉🥳🥳🥳🎊🎁 \n");
        char new_name[1024], old_name[1024];
        snprintf(old_name, sizeof(old_name), "Downloads/_%s.reconstructed", filename);
        snprintf(new_name, sizeof(new_name), "Downloads/%s", filename);

        

        // Renommage du fichier
        int retour = rename(old_name, new_name);
        if (retour == 0) {
            printf("Your file is now available on the path: %s\n", new_name);
        } else {
            
            perror("rename");
            fprintf(stderr, "Failed to rename '%s' to '%s': %s\n", old_name, new_name, strerror(errno));
        }
        snprintf(new_name, sizeof(new_name), "Downloads/_%s", filename);
        if (remove_directory(new_name) == 0) {
            printf("Directory '%s' removed successfully.\n", new_name);
        } else {
            perror("remove_directory");
            printf("Failed to remove directory '%s'.\n", new_name);
        }

        
    } else {
        printf("File transfer failed due to hash mismatch.\n");
    }

    close(sockfd);
    return 0;
}
