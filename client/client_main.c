
/*
        ---- Licence UY1 -----
    This file is one of the HomeWork of the Programming Computing Course for
    Computer Science field in Yaoundé I University. 

    You can use it if and only if you include the participation of Computer Sciences
    of Yaoundé I University Especially Serge Noah, matricule: 20U2669 it creator
*/


#include "include/client.h"

/*
    When the client started, it send his data folder and it subfolder files informations
    after receive the acknowledge, it start watch the change on data folder and his subfolders.
    When a changement occured (rename, delete, add or update file of subfolder), he capture 
    the new tree path of data and it subfolders and send it to the server.

    The data format is:
     [
        {filename, size, last_modification_date}
     ]
*/





int main(int argc, char *args[]) {
    // Verification of parameter's elements
    if (argc < 3) {
        log_action("Too few arguments given", "Error");
        perror("Usage: client SERVER_IP_ADDRESSE SERVER_PORT\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    char *files;
    char *address, buffer[BUFFER_SIZE] = {0};
    address = args[1];
    long port = atoi(args[2]);
    int dir_fd, watcher_dir, sock;



    //capture the actual tree state of "data" folder
    files = capture_data();


    

    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        log_action("Creating socket file failled", "Danger");
        perror("Erreur lors de la création du socket\n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert the IP server address to binary format
    if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0) {
        log_action("Invalid IP ADDRESS of server \n", "Errro");
        perror("Invalid IP ADDRESS of server  / NOt supported IP !");
        exit(EXIT_FAILURE);
    }

    // Connection to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        log_action("Connexion to the server failled", "Error");
        perror("Connexion to the server failled");
        exit(EXIT_FAILURE);
    }


    //Send the initial state to the server  
    send(sock, files, strlen(files), 0);
    log_action("Initial state send to the server", "Info");
    printf("Initial state of 'data' folder sent sucessfully.\n");
    free(files);


    // Initialize inotify to monitor changes in the data folder and subfolders
    dir_fd = inotify_init();
    if (dir_fd < 0) {
        log_action("inotify initialization failled, try later", "Error");
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }

    //start watching the "data" folder with inotify
    watcher_dir = inotify_add_watch(dir_fd, "data", IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
    if (watcher_dir < 0) {
        log_action("inotify watch failled, try later", "Error");
        perror("inotify_add_watch failled: ");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int length = read(dir_fd, buffer, BUF_LEN);

        if (length < 0) {
        log_action("inotify read action, try later", "error");
            perror("read failled: ");
            exit(EXIT_FAILURE);
        }
 
        int i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len) {
                if (event->mask & IN_CREATE ) {
                   
                    // Capture the new tree path of data and subfolders
                    files = capture_data();

                    // Send it to the server in the specified data format
                    send(sock, files, strlen(files), 0);
                    printf("File operation : Creation,  state of 'data/'  sent sucessfully.\n");
                    free(files);
                    log_action("File operation : Creation", "Info");

                } else   if (event->mask & IN_DELETE ) {

                    // Capture the new tree path of data and subfolders
                    files = capture_data();

                    // Send it to the server in the specified data format
                    send(sock, files, strlen(files), 0);
                    printf("File operation : Delection,  state of 'data/'  sent sucessfully.\n");
                    free(files);
                    log_action("File operation : Delection", "Info");

                }else   if (event->mask & IN_MOVED_FROM ) {

                    // Capture the new tree path of data and subfolders
                    files = capture_data();
                    // Send it to the server in the specified data format
                    send(sock, files, strlen(files), 0);
                    printf("File operation : Moving Out,  state of 'data/'  sent sucessfully.\n");
                    free(files);
                    log_action("File operation : Moving Out Action", "Info");

                } else   if (event->mask & IN_MOVED_TO) {
                    // Capture the new tree path of data and subfolders
                    files = capture_data();

                    // Send it to the server in the specified data format
                    send(sock, files, strlen(files), 0);
                    printf("File operation : Moving In,  state of 'data/'  sent sucessfully.\n");
                    free(files);
                    log_action("File operation : MOving In Action", "Info");
                }else if (event->mask & IN_MODIFY) {
                    // Capture the new tree path of data and subfolders
                    files = capture_data();

                    // Send it to the server in the specified data format
                    send(sock, files, strlen(files), 0);
                    printf("File operation : Modification,  state of 'data/'  sent sucessfully.\n");
                    free(files);
                    log_action("File operation : Modification", "Info");

                }
                i += EVENT_SIZE + event->len;
            }
        }
    }
    

        // Réception de données du serveur
        // recv(sock, buffer, BUFFER_SIZE, 0);
        // printf("Réponse %d du serveur: %s\n\n", i, buffer);
	
        // Fermeture du socket
        close(sock);


    return 0;
} 

