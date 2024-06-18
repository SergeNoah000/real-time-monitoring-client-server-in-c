
#include "../include/client.h"

int calculate_hash1(const char *data, size_t length, unsigned char *hash) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, length);
    SHA256_Final(hash, &sha256);
    return 0;
}

int calculate_hashes_from_file(const char *filename, unsigned char **hashes, int piece_count) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("File open failed");
        return -1;
    }

    char buffer[PIECE_SIZE];
    printf("Nombre de piece: %d\n", piece_count);
    for (int i = 0; i < piece_count; i++) {
        size_t length = fread(buffer, 1, PIECE_SIZE, file);
        if (length == 0) {
            perror("Read error or end of file");
            fclose(file);
            return -1;
        }
        calculate_hash(buffer, length, hashes[i]);
        // for (int j = 0; j < SHA256_DIGEST_LENGTH; j++) {
        //     printf("%02x", hashes[i][j]);
        // }
        // printf("  Ok \n");
    }
    fclose(file);
    return 0;
}

int calculate_global_hash_from_file(const char *filename, int piece_count, unsigned char *global_hash) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }

    char buffer[PIECE_SIZE];
    for (int i = 0; i < piece_count; i++) {
        size_t length = fread(buffer, 1, PIECE_SIZE, file);
        if (length == 0) {
            perror("Read error or end of file");
            exit(EXIT_FAILURE);
        }
        SHA256_Update(&sha256, buffer, length);
    }
    fclose(file);
    SHA256_Final(global_hash, &sha256);

    // printf("Calcul de hash global..\n");
    // for (int j = 0; j < SHA256_DIGEST_LENGTH; j++) {
    //     printf("%02x", global_hash[j]);
    // }
    // printf(" Ok\n");
    return 0;
}

int send_piece(int sockfd, const char *filename, int piece_index) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("File open failed");
        return -1;
    }

    // Positionnement du curseur pour lire la pièce demandée
    if (fseek(file, piece_index * PIECE_SIZE, SEEK_SET) != 0) {
        perror("fseek failed");
        fclose(file);
        return -1 ;
    }

    // Lecture des données
    char buffer[PIECE_SIZE];
    size_t length = fread(buffer, 1, PIECE_SIZE, file);
    if (length == 0 && !feof(file)) {
        perror("Read error");
        fclose(file);
        return -1 ;
    }
    fclose(file);

    // Calcul du hash
    unsigned char hash[SHA256_DIGEST_LENGTH];
    calculate_hash(buffer, length, hash);

    // Envoi des informations de la pièce
    if (send(sockfd, &piece_index, sizeof(piece_index), 0) != sizeof(piece_index) ||
        (size_t)send(sockfd, &length, sizeof(length), 0) != sizeof(length) ||
        (size_t)send(sockfd, buffer, length, 0) != length ||
        send(sockfd, hash, SHA256_DIGEST_LENGTH, 0) != SHA256_DIGEST_LENGTH) {
        perror("Failed to send piece");
        return -1;
    }
    return 0;
}


int send_hash_list(int sockfd, unsigned char **hashes, int piece_count) {
    send(sockfd, &piece_count, sizeof(piece_count), 0);
    printf("Debut du transfert de la liste des hashs\n\n");
    for (int i = 0; i < piece_count; i++) {
        send(sockfd, hashes[i], SHA256_DIGEST_LENGTH, 0);
    }
    printf("Transfert de la liste des hash termine ..\n\n");
    return 0;
}

int send_global_hash(int sockfd, unsigned char *global_hash) {
    printf("Debut du transfert de la liste des hashs\n\n");
    if (send(sockfd, global_hash, SHA256_DIGEST_LENGTH, 0) != SHA256_DIGEST_LENGTH) {
        perror("send_global_hash");
        return -1;
        log_action(" send global failled", "Error");
    }
    printf("Transfert termine..\n\n");
    return 0;
}



int main_sender(int argc, char const *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    const char *filename = argv[2];

    int sockfd, newsockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    

    int piece_count;
    struct stat st;
    if (stat(filename, &st) == 0) {
        piece_count = (st.st_size + PIECE_SIZE - 1) / PIECE_SIZE;
    } else {
        perror("File stat failed");
        exit(EXIT_FAILURE);
    }

    unsigned char **hashes = malloc(piece_count * sizeof(unsigned char *));
    for (int i = 0; i < piece_count; i++) {
        hashes[i] = malloc(SHA256_DIGEST_LENGTH);
    }
    calculate_hashes_from_file(filename, hashes, piece_count);

    unsigned char global_hash[SHA256_DIGEST_LENGTH];
    if (calculate_global_hash_from_file(filename, piece_count, global_hash) != 0) {       
        perror("Failed to calculate global hash");
        return -1;
    }

    listen(sockfd, 5);

    printf("Server listening on port %d\n", port);

    newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    if (newsockfd < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    printf("Connection accepted from client\n");

    while (1) {
        char request_type;
        if (recv(newsockfd, &request_type, sizeof(request_type), 0) <= 0) {
            break;
        }

        if (request_type == 'L') {
            size_t file_size = st.st_size;
            send(newsockfd, &piece_count, sizeof(piece_count), 0);
            send(newsockfd, &file_size, sizeof(file_size), 0);
            for (int i = 0; i < piece_count; i++) {
                send(newsockfd, hashes[i], SHA256_DIGEST_LENGTH, 0);
            }
        } 
        
        else if (request_type == 'H') {
            send(newsockfd, global_hash, SHA256_DIGEST_LENGTH, 0);
        }
         
        else if (request_type == 'P') {
            int index;
            recv(newsockfd, &index, sizeof(index), 0);
            send_piece(newsockfd, filename, index);
        }

    }

    close(newsockfd);
    close(sockfd);

    for (int i = 0; i < piece_count; i++) {
        free(hashes[i]);
    }
    free(hashes);

    return 0;
}
