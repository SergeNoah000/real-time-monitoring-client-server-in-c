
/*
        ---- Licence UY1 -----
    This file is one of the HomeWork of the Programming Computing Course for
    Computer Science field in Yaoundé I University. 

    You can use it if and only if you include the participation of Computer Sciences
    of Yaoundé I University Especially Serge Noah, matricule: 20U2669 it creator
*/



#include "include/server.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
Connection *connections_queue;



int  queue_size = 0;
int queue_capacity = 10;
int active_threads = 0;



/*
    The server role is to listen on the given PORT in argv[1]

    When it receive data, it logs it in the "server_data.log" file
    in the format: 

        CLIENT_IP::
        FILE_1 FILE_1_SIZE FILE_1_LAST_MODIFIED_DATE
        FILE_2 FILE_2_SIZE FILE_2_LAST_MODIFIED_DATE
        FILE_3 FILE_2_SIZE FILE_3_LAST_MODIFIED_DATE
        .
        .
        .
        FILE_N FILE_N_SIZE FILE_N_LAST_MODIFIED_DATE

    It the care to update the list if the client is still present in the log file

*/



// Handle a client connexion in a thread and wait future data for this client
void *handle_connection(void );




int main(int argc, char *argv[] ) {

     // Verification of port parameter's elements
    if (argc < 2) {
        log_action("Too few arguments given (PORT missing)", "Error");
        perror("Usage: server SERVER_PORT\n");
        exit(EXIT_FAILURE);
    }
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        log_action("Creating socket file failled", "Danger");
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        log_action("setsockopt failled", "Error");
        perror("setsockopt error");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[1]));

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        log_action("Binding of port to the sokcket failled ", "Error");
        perror("Binding port failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 300) < 0) {
        log_action("Listen operation error", "Danger");
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", atoi(argv[1]));

    connections_queue = (Connection *)malloc(sizeof(Connection) * queue_capacity);

    pthread_t threads[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_create(&threads[i], NULL, handle_connection, NULL);
    }
    clear_last_lines(1);

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        clear_last_lines(2);

        printf("\033[1;32m"); // Vert gras
        printf("Active threads: [ %d ]\n", active_threads + 1);
        printf("\033[0m"); // Réinitialisation de la couleur

        printf("New connection from %s on port %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        pthread_mutex_lock(&mutex);
        if (queue_size >= queue_capacity) {
            queue_capacity *= 2;
            connections_queue = realloc(connections_queue, sizeof(Connection) * queue_capacity);
        }
        connections_queue[queue_size].client_socket = new_socket;
        connections_queue[queue_size].client_address = address;
        queue_size++;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    return 0;
}




void *handle_connection(void ) {
    while (true) {
        pthread_mutex_lock(&mutex);
        while (queue_size == 0) {
            pthread_cond_wait(&cond, &mutex);
        }
        active_threads++;
        int client_socket = connections_queue[queue_size - 1].client_socket;
        struct sockaddr_in client_address = connections_queue[queue_size - 1].client_address;
        queue_size--;
        pthread_mutex_unlock(&mutex);

        char buffer[BUFFER_SIZE] = {0};


        while(true){
            ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received <= 0) {
                close(client_socket);
                clear_last_lines(2);
                char log_message[512];
                sprintf(log_message, "Connexion with client %s:%d lost", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
                printf("\033[1;32m"); // Vert gras
                printf("Active threads: [ %d ]\n", active_threads + 1);
                printf("\033[0m"); // Réinitialisation de la couleur
                fprintf(stdout,"%s\n",  log_message);
                log_action(log_message, "Warning");

                pthread_mutex_lock(&mutex);
                active_threads--;
                pthread_mutex_unlock(&mutex);
                break;
            }

            if (strstr(buffer, "LIST_FILES") != NULL)
            {
                //  clear_last_lines(1);
                clear_last_lines(2);
                time_t date_t = time(NULL);
                char *str_date = malloc(sizeof(char) * 50);
                convert_time(str_date, sizeof(date_t ) * 4, date_t);
                char log_message[512];
                sprintf(log_message, "[ %s ] Requested list files by %s\n", str_date, inet_ntoa(client_address.sin_addr));
                log_action(log_message, "Info");
                printf("\033[1;32m"); // Vert gras
                printf("Active threads: [ %d ]\n", active_threads + 1);
                printf("\033[0m"); // Réinitialisation de la couleur
                fprintf(stdout,"%s\n",  log_message);
                free(str_date);
                read_and_send_files_infos(FILE_DATA, client_socket);
            }else
            {
            
                clear_last_lines(2);
                time_t date_t = time(NULL);
                char *str_date = malloc(sizeof(char) * 50);
                convert_time(str_date, sizeof(date_t ) * 4, date_t);
                printf("\033[1;32m"); // Vert gras
                printf("Active threads: [ %d ]\n", active_threads + 1);
                printf("\033[0m"); // Réinitialisation de la couleur
                printf("[ %s ] Received data from client %s:%d:\n",str_date, inet_ntoa(client_address.sin_addr),
                    ntohs(client_address.sin_port));
                
                free(str_date);

                char *data = ascii_to_utf8(buffer);

                // Log the received datas        
                pthread_mutex_lock(&mutex);
                replace_log_message(inet_ntoa(client_address.sin_addr), data);
                pthread_mutex_unlock(&mutex);
                char *message;
                message = malloc(sizeof(char) * 70);
                sprintf(message, "Update 'server_data.log' caused by client  ``%s`` ", inet_ntoa(client_address.sin_addr));
                log_action(message, "Info");
                free(message);
            }
        }
        // pthread_mutex_lock(&mutex);
        // active_threads--;
        // pthread_mutex_unlock(&mutex);
    }

    return NULL;
}
