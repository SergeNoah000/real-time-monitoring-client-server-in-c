
/*
        ---- Licence UY1 -----
    This file is one of the HomeWork of the Programming Computing Course for
    Computer Science field in Yaoundé I University. 

    You can use it if and only if you include the participation of Computer Sciences
    of Yaoundé I University Especially Serge Noah, matricule: 20U2669 it creator
*/


#include "include/client.h"
#include <sys/signal.h>
#include "pthread.h"
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
    char *address;
    address = args[1];
    long port = atoi(args[2]);
    int sock;
    char option;
    pid_t pid = -1;





    

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

     // Display the file sharing system diagram
    displayDiagram();

    // Main program loop
    while (1) {
        printf("\n\nOptions:\n");
        printf("1. Share file list with the server\n");
        printf("2. View the list of files available\n");
        printf("q. Quit\n");
        printf("p. Start sharing files with the server\n");
        printf("s. Stop sharing files with the server\n");
        printf("Enter your option: ");
        scanf(" %c", &option);

        switch (option) {
            case '1':
                 // Share file list with the server in a new process
                pid = fork();
                if (pid == 0) {
                    // Child process
                    // close(STDOUT_FILENO);  // Close standard output

                    // start listening incomming requests in background
                    pthread_t thread;
                    pthread_create(&thread, NULL, upload_file, &sock);
                    share_data_contents(sock);
                    break;
                } else if (pid > 0) {
                    // Parent process
                    printf("Sharing file list with the server in a new process (PID: %d)\n", pid);
                    break;
                } else {
                    // Fork failed
                    perror("Fork failed");
                    break;
                }

            case '2':
                // View the list of files available
                view_files_list(sock);
                break;
            case 'q':
                // Quit the program
                if (pid > 0)
                {
                    kill(pid, SIGKILL);
                }
                close(sock);
                exit(EXIT_SUCCESS);
            // case 'p':
            //     // Start sharing files with the server
            //     // Add code to initiate file sharing with the server
            //     break;
            case 's':
                // Stop sharing files with the server
                if (pid > 0) {
                    // Send SIGTERM to the child process
                    kill(pid, SIGTERM);
                    printf("Stopped sharing files with the server (PID: %d)\n", pid);
                    pid = -1;  // Reset pid to an invalid value
                } else {
                    printf("No sharing process is currently running\n");
                }
                break;
            default:
                printf("Invalid option. Please try again.\n");
                break;  
        }
    }

    return 0;
    
} 
