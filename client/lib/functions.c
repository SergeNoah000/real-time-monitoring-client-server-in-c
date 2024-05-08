/*
        ---- Licence UY1 -----
    This file is one of the HomeWork of the Programming Computing Course for
    Computer Science field in Yaoundé I University. 

    You can use it if and only if you include the participation of Computer Sciences
    of Yaoundé I University Especially Serge Noah, matricule: 20U2669 it creator
*/

#include <stdio.h>
#include "../include/client.h"
#include <stdbool.h>



//convert time to a valid  string
void convert_time(char *str,  size_t size, time_t time){
    struct tm *tmp = localtime(&time);
    strftime(str, size, "%^a %d-%m-%Y %H:%M:%S", tmp);
}


// Function to log an action with its corresponding danger level
void log_action(const char *action, char* danger) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Open log file failled");
        exit(EXIT_FAILURE);
    }

    time_t date_t = time(NULL);
    char *str_date = malloc(sizeof(char) * 30);
    convert_time(str_date, sizeof(date_t ) * 4, date_t);
    fprintf(log_file, "[ %s ] %s: %s \n",str_date, danger, action );
    fclose(log_file);
    free(str_date);
}


// Encode UTF-8 string to ASCII
char *utf8_to_ascii(const char *utf8_str) {
    size_t utf8_len = strlen(utf8_str);
    char *ascii_str = (char *)malloc(3 * utf8_len + 1); // Allocate memory for ASCII string
    if (ascii_str == NULL) {
        log_action("Memory allocation failed in the  'utf8_to_ascii' function", "Error");
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t ascii_index = 0;
    for (size_t i = 0; i < utf8_len; i++) {
        // Encode non-ASCII characters as escape sequences in ASCII string
        if (utf8_str[i] >= 0 && utf8_str[i] <= 127) {
            ascii_str[ascii_index++] = utf8_str[i];
        } else {
            sprintf(&ascii_str[ascii_index], "\\x%02X", (unsigned char)utf8_str[i]); // Encode non-ASCII character
            ascii_index += 4; // Move to next position
        }
    }
    ascii_str[ascii_index] = '\0'; // Null-terminate ASCII string
    return ascii_str;
}


//convert the struct to string. In this format, i can easilly  send it to the server
char *file_infos_to_string(file_infos *files, int num_files) {
    // Calculate the maximum possible size of the resulting string
    int max_string_size = num_files * (sizeof(file_infos) + 1); // Add 1 for '\n' separator for each entity
    char *result_string = (char *)malloc(max_string_size);
    if (result_string == NULL) {
        log_action("Memory allocation failed in the  'file_infos_to_string' function", "Error");
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Iterate through the file_infos array and concatenate each entity to the result string
    int offset = 0;
    result_string[0]='\0'; // Initialize result_string as an empty string
    for (int i = 0; i < num_files; i++) {
        // Append filename
        strcat(result_string + offset, files[i].filename);
        offset += strlen(files[i].filename);

        // Append size
        char size_str[20]; // Assuming int can be represented in 20 characters
        sprintf(size_str, " %.2f", files[i].filesize);
        strcat(result_string + offset, size_str);
        offset += strlen(size_str);

        // Append modification date
        strcat(result_string + offset, " ");
        offset += 1;
        strcat(result_string + offset, files[i].last_modified_date);
        offset += strlen(files[i].last_modified_date);

        // Append newline separator
        strcat(result_string + offset, "\n");
        offset += 1;
    }

    return result_string;
}

//capture all files informations of "data" folder
char *capture_data(){
        
    int  file_number = 0;
    DIR *dir; 
    struct dirent *parent;
    struct stat file_stat;

    dir = opendir("data");
    if (dir == NULL)
    {
        log_action("Failled to open the 'data/' directory:", "Error");
        perror("Failled to open the 'data/' directory");
        exit(EXIT_FAILURE);
    }
    

    //count the number of files and folders in directory
    while((parent = readdir(dir)) != NULL){
        char filename[PATH_MAX]; // PATH_MAX is defined in <limits.h>
        snprintf(filename, sizeof(filename), "data/%s", parent->d_name); 
        stat(filename, &file_stat); 
        if (S_ISDIR(file_stat.st_mode)){
            continue;
        }else{
        file_number ++;
        }
        
    }
    closedir(dir);
    
    //allocate memory for the file information array
    if (file_number == 0)
    {
        log_action("The folder 'data/' has no file(s)", "Warning");
        perror("The folder 'data/' has no file(s)");
        return NULL;
    }
    
    file_infos *file_all = (file_infos *)malloc(sizeof(file_infos) * file_number);

    if (!file_all)
    {
        log_action("Failled to allocate memory for the file information array", "Error");
        perror("Falled to allocate memory for the file information array");
        exit(EXIT_FAILURE);
    }

    // Populate the file_info array data
    int number = 0;
    dir = opendir("data/");
    if (dir == NULL)
    {
        log_action("Failled to open the 'data/' directory:", "Error");
        perror("Failled to open the 'data/' directory");
        exit(EXIT_FAILURE);
    }
    while ((parent = readdir(dir)) != NULL){
       if (strcmp(parent->d_name, ".") == 0 || strcmp(parent->d_name, "..") == 0)
            continue;

        char filename[PATH_MAX]; // PATH_MAX is defined in <limits.h>
        snprintf(filename, sizeof(filename), "data/%s", parent->d_name); // Construct full file path
        stat(filename, &file_stat); // Use full file path
        if (!S_ISDIR(file_stat.st_mode)) {
           snprintf(file_all[number].filename, sizeof(file_all[number].filename), "%s", parent->d_name);
            file_all[number].filesize = (double ) file_stat.st_size / (1024 * 1024);
            convert_time(file_all[number].last_modified_date, sizeof(file_all[number].last_modified_date), file_stat.st_mtime);
            //strncpy(file_all[file_number].filename, parent->d_name, sizeof(file_all[file_number].filename));
            number++;
           
        }

    } 
    
    closedir(dir);

    char *text = file_infos_to_string(file_all, file_number);
    fprintf(stdout, "%s\n\n", text);
    return utf8_to_ascii(text);    
}   


    void share_data_contents(int socket_fd){
    char *files, buffer[BUFFER_SIZE] = {0};
    int dir_fd, watcher_dir;


    //capture the actual tree state of "data" folder
    files = capture_data();


    //Send the initial state to the server  
    send(socket_fd, files, strlen(files), 0);
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
                    send(socket_fd, files, strlen(files), 0);
                    printf("File operation : Creation,  state of 'data/'  sent sucessfully.\n");
                    free(files);
                    log_action("File operation : Creation", "Info");

                } else   if (event->mask & IN_DELETE ) {

                    // Capture the new tree path of data and subfolders
                    files = capture_data();

                    // Send it to the server in the specified data format
                    send(socket_fd, files, strlen(files), 0);
                    printf("File operation : Delection,  state of 'data/'  sent sucessfully.\n");
                    free(files);
                    log_action("File operation : Delection", "Info");

                }else   if (event->mask & IN_MOVED_FROM ) {

                    // Capture the new tree path of data and subfolders
                    files = capture_data();
                    // Send it to the server in the specified data format
                    send(socket_fd, files, strlen(files), 0);
                    printf("File operation : Moving Out,  state of 'data/'  sent sucessfully.\n");
                    free(files);
                    log_action("File operation : Moving Out Action", "Info");

                } else   if (event->mask & IN_MOVED_TO) {
                    // Capture the new tree path of data and subfolders
                    files = capture_data();

                    // Send it to the server in the specified data format
                    send(socket_fd, files, strlen(files), 0);
                    printf("File operation : Moving In,  state of 'data/'  sent sucessfully.\n");
                    free(files);
                    log_action("File operation : MOving In Action", "Info");
                }else if (event->mask & IN_MODIFY) {
                    // Capture the new tree path of data and subfolders
                    files = capture_data();

                    // Send it to the server in the specified data format
                    send(socket_fd, files, strlen(files), 0);
                    printf("File operation : Modification,  state of 'data/'  sent sucessfully.\n");
                    free(files);
                    log_action("File operation : Modification", "Info");

                }
                i += EVENT_SIZE + event->len;
            }
        }
    }
    

        // Réception de données du serveur
        // recv(socket_fd, buffer, BUFFER_SIZE, 0);
        // printf("Réponse %d du serveur: %s\n\n", i, buffer);
	
        // Fermeture du socket
        close(socket_fd);



    }


// Function to display the console diagram of the file sharing system
void displayDiagram() {
    // Display the diagram with credits
    printf("File Sharing System Diagram\n");
    printf("----------------------------\n");
    printf("   Client 1 -----> Server <----- Client 2\n");
    printf("        |               |               |\n");
    printf("        V               V               V\n");
    printf("   Client 4         Client 3         Client 5\n");
    printf("\nCredits: Serge Noah (Master I SE & RESEAU)\n");
}



/// @brief Download filename to another client ip given
/// @param filename : Filename to download
/// @param ip : Client IP
/// @return int Returns 0 if successful otherwise returns -1
int downloade_file(char *filename, char * ip){
    //create socket

     struct sockaddr_in serv_addr;
    char *address;
    address = ip;
    long port = 7070;
    int sock;

    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        log_action("Creating socket file failled", "Danger");
        perror("Erreur lors de la création du socket\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert the IP server address to binary format
    if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0) {
        log_action("Invalid IP ADDRESS of other client \n", "Errro");
        perror("Invalid IP ADDRESS of other client  / NOt supported IP !");
        return -1;
    }

    // Connection to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        log_action("Connexion to the other client failled", "Error");
        perror("Connexion to the  client failled");
        return -1;
    }

    if (send(sock, filename, sizeof(filename), 0) == -1) {
        char message[MAX_FILE_SIZE];
        sprintf(message, "Failled to send the filename %s the client %s", filename, ip);
        log_action(message, "Error");
        return -1;
    }

    int n;
    FILE *fp;
    char path[MAX_FILE_SIZE] ;
    sprintf(path, "Downloads/%s", filename);
    char buffer[BUFFER_SIZE];

    fp = fopen(path, "w");
    while (1) {
        n = recv(sock, buffer, sizeof(buffer), 0);
        if (n <= 0){
        break;
        return;
        }
        fprintf(fp, "%s", buffer);
        bzero(buffer, BUFFER_SIZE);
    }
     char message[MAX_FILE_SIZE];
    sprintf(message, "Receiving file '%s' from '%s'", filename, ip);
    log_action(message, "Info");

    close(sock);
    return 0;

}


// Function to display the list of files
void view_files_list(int sock) {
    char *buffer;

    // Sending request to the server
    char *request = "LIST_FILES";
    send(sock, request, strlen(request), 0);

    // buffer size to receive from the server
    int  file_count = 0;
    
    // Receive number of files
    if (recv(sock, &file_count, sizeof(file_count), 0) < 0) {
        perror("Receive failed");
        exit(EXIT_FAILURE);
    }
    // Array to store file information
     FileInfo files[file_count+1];
    // Receive file info
    for (int i = 0; i < file_count; ++i) {
        if (recv(sock, &files[i], sizeof( FileInfo), 0) < 0) {
            perror("Receive failed");
            exit(EXIT_FAILURE);
        }
    }




        // Displaying the list of files with index, name, and modification date
        printf("\n\nIndex\tFilename\tSize (Mo)\tModification Date\n");
        printf("-------------------------------------------------\n");
        for (int i = 0; i < file_count; i++) {
            printf("[%d]\t%s\t\t%ld\t\t%s\n", i+1 ,
                   files[i].filename,
                   files[i].size,
                   files[i].modification_date);
        }

        // Interaction with the user for downloading or returning to the menu
        printf("\nEnter the index of the file to download (Ctrl+C to return to the menu): ");
        int file_index;
        scanf("%d", &file_index);

        // Code to download the file with the given index
        printf("\n\n\n Downloading file %s with index %d...\n", files[file_index-1].filename,  file_index-1);

        // Code to download the file with the given index file
        if ( downloade_file(files[file_index-1].filename, files[file_index-1].ip) == -1 ){
            //log_action("Failling in the download_file function", "Error");
            char message[1024];
            sprintf(message, "Error downloading file %s from %s", files[file_index-1].filename, files[file_index-1].ip);
            printf(message);
            log_action(message, "Error");
            perror("Error downloading file");
            return;
        }
        char message[1024];
        sprintf(message, "downloading file %s from %s finished", files[file_index-1].filename, files[file_index-1].ip);
        log_action(message, "Success");
        printf("Downloading file %s. finished. Your file is available in the Downloads/ folder. \n", files[file_index-1].filename);
        return;
}

// Return relative path from  the caller dir and filename
/// @brief 
/// @param dir_path 
/// @param filename 
/// @return char *
char *search_file(const char *dir_path, const char *filename) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char *filepath = NULL;

    dir = opendir(dir_path);
    if (dir == NULL) {
        perror("Erreur lors de l'ouverture du répertoire");
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        if (strcmp(entry->d_name, filename) == 0) {
            filepath = strdup(path);
            break;
        }

        if (stat(path, &statbuf) == -1) {
            perror("Erreur lors de la récupération des informations sur le fichier");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            char *sub_filepath = search_file(path, filename);
            if (sub_filepath != NULL) {
                filepath = sub_filepath;
                break;
            }
        }
    }

    closedir(dir);
    return filepath;
}



int send_file(char *fp, int sockfd){
    char data[BUFFER_SIZE] = {0};
    FILE *send_file_desc = fopen(fp, 'r');
    if(send_file_desc == NULL){
        log_action("Failled to open the file in send_file function", "Error");
        perror("open file");
    }

  while(fgets(data, BUFFER_SIZE, send_file_desc) != NULL) {
    if (send(sockfd, data, sizeof(data), 0) == -1) {
        log_action("Failled to send the data file", "Error");
        perror("Error in sending file.");
        return 0;
    }
    bzero(data, BUFFER_SIZE);
  }
  return 1;
}



void *upload_file( ){
    int other_client_sockef_fd, new_socket;
    struct sockaddr_in address, address_other;
    int opt = 1;

    if ((other_client_sockef_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        log_action("Creating socket file failled", "Danger");
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(other_client_sockef_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        log_action("setsockopt failled", "Error");
        perror("setsockopt error");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(7070);

    if (bind(other_client_sockef_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        log_action("Binding of port to the sokcket failled ", "Error");
        perror("Binding port failed");
        exit(EXIT_FAILURE);
    }

    if (listen(other_client_sockef_fd, 300) < 0) {
        log_action("Listen operation error", "Danger");
        perror("listen");
        exit(EXIT_FAILURE);
    }

    log_action("Start listening the incomming upload request", "Info");

    while(true){
        if ((new_socket = accept(other_client_sockef_fd, (struct sockaddr*)&address_other, (socklen_t *)&address_other)) < 0){
                log_action("Failled to accept incomming request", "Error");
                perror("accepting the incomming upload request");
                exit(EXIT_FAILURE);
            }
            char filename[150] = {0};
            ssize_t bytes_received = recv(new_socket, filename, BUFFER_SIZE, 0);
            if( bytes_received == -1 ){
                log_action("Failled receive the incomming request filename", "Error");
                perror("recv the incomming request filename");
                exit(EXIT_FAILURE);
            }

            char *filepath = search_file("data", filename);
            if (filepath == NULL) {
                log_action("Failled to find the incomming request file", "Error");
                send(new_socket, NULL, sizeof(NULL), 0);
                exit(EXIT_FAILURE);
            }

            if( send_file(filepath, new_socket) == 0 ) {
                log_action("It occurs when trying to send the requested file", "Error");
                perror("Error sending");
                break;

            }
            sprintf(filepath, "Sucessfully send  %s to %s", filename, inet_ntoa(address.sin_addr));
            log_action(filepath, "Sucess");

            close(new_socket);

    }
    

}

