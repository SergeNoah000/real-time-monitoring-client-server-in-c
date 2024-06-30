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


jmp_buf env ;


void handle_sigint() {
    longjmp(env, 1);
}



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
        if (utf8_str[i] >= 0 ) {
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
        sprintf(size_str, " %.ld", files[i].filesize);
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
            file_all[number].filesize =  file_stat.st_size ;
            convert_time(file_all[number].last_modified_date, sizeof(file_all[number].last_modified_date), file_stat.st_mtime);
            //strncpy(file_all[file_number].filename, parent->d_name, sizeof(file_all[file_number].filename));
            number++;
           
        }

    } 
    
    closedir(dir);

    char *text = file_infos_to_string(file_all, file_number);
    //fprintf(stdout, "%s\n\n", text);
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
    //printf("Initial state of 'data' folder sent sucessfully.\n");
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
                    //printf("File operation : Creation,  state of 'data/'  sent sucessfully.\n");
                    free(files);
                    log_action("File operation : Creation", "Info");

                } else   if (event->mask & IN_DELETE ) {

                    // Capture the new tree path of data and subfolders
                    files = capture_data();

                    // Send it to the server in the specified data format
                    send(socket_fd, files, strlen(files), 0);
                    //printf("File operation : Delection,  state of 'data/'  sent sucessfully.\n");
                    free(files);
                    log_action("File operation : Delection", "Info");

                }else   if (event->mask & IN_MOVED_FROM ) {

                    // Capture the new tree path of data and subfolders
                    files = capture_data();
                    // Send it to the server in the specified data format
                    send(socket_fd, files, strlen(files), 0);
                    //printf("File operation : Moving Out,  state of 'data/'  sent sucessfully.\n");
                    free(files);
                    log_action("File operation : Moving Out Action", "Info");

                } else   if (event->mask & IN_MOVED_TO) {
                    // Capture the new tree path of data and subfolders
                    files = capture_data();

                    // Send it to the server in the specified data format
                    send(socket_fd, files, strlen(files), 0);
                    //printf("File operation : Moving In,  state of 'data/'  sent sucessfully.\n");
                    free(files);
                    log_action("File operation : MOving In Action", "Info");
                }else if (event->mask & IN_MODIFY) {
                    // Capture the new tree path of data and subfolders
                    files = capture_data();

                    // Send it to the server in the specified data format
                    send(socket_fd, files, strlen(files), 0);
                    //printf("File operation : Modification,  state of 'data/'  sent sucessfully.\n");
                    free(files);
                    log_action("File operation : Modification", "Info");

                }
                i += EVENT_SIZE + event->len;
            }
        }
    }
    

	



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



void progress_bar_ftp(size_t downloaded_size, size_t total_size) {
        int bar_width = 70;
        float progress = (float)downloaded_size / (float)total_size;
        int pos = bar_width * progress;

        printf("[");
        for (int i = 0; i < bar_width; ++i) {
            if (i < pos) {
                printf("░");  // emoji for the loaded part
            } else {
                printf(" ");
            }
        }
        printf("] %0.2f%%\r", progress * 100);
        fflush(stdout);
    }

/// @brief Receive a file from the sockfd socket and save it in fp
/// @param fp 
/// @param sockfd 
/// @return size_t size of the reveived file
size_t receive_file_with_ftp(const char *fp, const int sockfd) {
    // Open the file in write binary mode
    FILE *received_file_desc = fopen(fp, "wb");
    if (received_file_desc == NULL) {
        log_action("Failed to open the file in receive_file_with_ftp function", "Error");
        perror("open file");
        return 0;
    }

    char data[BUFFER_SIZE];
    size_t file_size = 0, total_bytes_received = 0;
    recv(sockfd, &file_size, sizeof(file_size), 0);
    ssize_t bytes_received;
    printf("Debut de reception  de %ld ..\n", file_size);
    printf("Debut de reception ..\n");
    bytes_received = recv(sockfd, data, BUFFER_SIZE, 0);
    while (bytes_received  > 0) {
        size_t bytes_written = fwrite(data, 1, bytes_received, received_file_desc);
        if (bytes_written <  (size_t)bytes_received) {
            log_action("Failed to write file data", "Error");
            perror("Error in writing file.");
            fclose(received_file_desc);
            return total_bytes_received;
        }
        total_bytes_received = bytes_received + total_bytes_received ;
        progress_bar_ftp(total_bytes_received, file_size);
        if((float)total_bytes_received == (float)file_size){
            printf("File transfer completed successfully.🎉🎉🎉🥳🥳🥳🎊🎁 \n");
            fclose(received_file_desc);
            close(sockfd);
            return total_bytes_received;
        }
        bytes_received = recv(sockfd, data, BUFFER_SIZE, 0);
    }

    fclose(received_file_desc);
    if (total_bytes_received == file_size) {
        printf("File transfer completed successfully.🎉🎉🎉🥳🥳🥳🎊🎁 \n");
        close(sockfd);
        return total_bytes_received;
    }else{
        printf("Failed to download file with FTP protocol");
        log_action("Failed to download file with FTP protocol", "Error");
        perror("open file");
        close(sockfd);
        return total_bytes_received;
    }
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
    ssize_t bytes_sent;
     printf("Which method to download ? \n\t b : for bitTorrent\n\t f : for FTP\n\t h : for HTTP\n");
    char method;
    scanf(" %c", &method);
    if (method == 'f'){
        char msg = 'A';
        send(sock, &msg, sizeof(msg), 0);
    }else if (method == 'h'){
        char msg = 'C';
        send(sock, &msg, sizeof(msg), 0);
    }else {
        char msg = 'B';
        send(sock, &msg, sizeof(msg), 0);
    }
    

    if ( ( bytes_sent = send(sock, filename, strlen(filename), 0)) == -1) {
        char message[MAX_FILE_SIZE];
        sprintf(message, "Failled to send the filename %s the client %s", filename, ip);
        log_action(message, "Error");
        return -1;
    }

    size_t size = 0;
    char path[MAX_FILE_SIZE] ;
    sprintf(path, "Downloads/%s", filename);
   
    if (method == 'f') {
        size = receive_file_with_ftp(path, sock);
    }else if (method == 'h') {
        size = download_file_with_http(ip, sock, filename );
    }else{
        size = receive_file_with_bittorent(filename, sock);
    }
    char message[MAX_FILE_SIZE];
    sprintf(message, "Received %ld bytes of '%s' from '%s'", size , filename, ip );
    log_action(message, "Info");

    return 0;
}



void human_readable_size(size_t size, char *output) {
    const char *units[] = {"octets", "Ko", "Mo", "Go", "To", "Po"};
    int unit_index = 0;
    double human_size = size;

    while (human_size >= 1024 && unit_index < 5) {
        human_size /= 1024;
        unit_index++;
    }

    snprintf(output, 20, "%.2f %s", human_size, units[unit_index]);
}

bool is_valid_file(FileInfo file) {
    return strlen(file.filename) > 0 && strlen(file.modification_date) > 0 && file.size > 0;
}

bool is_duplicate(FileInfo files[], int count, FileInfo file) {
    for (int i = 0; i < count; i++) {
        if (strcmp(files[i].filename, file.filename) == 0) {
            return true;
        }
    }
    return false;
}


// Function to display the list of files
void view_files_list(int sock) {

    signal(SIGINT, handle_sigint);
    if (setjmp(env) == 0) {
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
             printf("\n\nIndex\t Filename\t\t Size \t\tModification Date\n");
            printf("-------------------------------------------------\n");

            if (file_count == 0) {
                printf("No files found\n");
                log_action("No files found", "Info");
                return;
            } else {
                int valid_file_count = 0;
                for (int i = 0; i < file_count; i++) {
                    if (is_valid_file(files[i])) {
                        if (!is_duplicate(files, i, files[i])) {
                            valid_file_count++;
                            char output[20];
                            human_readable_size(files[i].size, output);
                            printf("[%d]\t%s\t\t%s\t\t%s\n", valid_file_count, files[i].filename, output, files[i].modification_date);
                        }
                    }
                }

                if (valid_file_count == 0) {
                    printf("No valid files found\n");
                    log_action("No valid files found", "Info");
                    return;
                }

                // Interaction with the user for downloading or returning to the menu
                printf("\nEnter the index of the file to download (Ctrl+C to return to the menu): ");
                int file_index;
                scanf("%d", &file_index);
                while (file_index > valid_file_count || file_index < 1) {
                    printf("\nChoose a correct number, %d is not in the list below: ", file_index);
                    scanf("%d", &file_index);
                }

                // Find the corresponding file by index
                int current_index = 0;
                for (int i = 0; i < file_count; i++) {
                    if (is_valid_file(files[i]) && !is_duplicate(files, i, files[i])) {
                        current_index++;
                        if (current_index == file_index) {
                            printf("\n\n\nDownloading file %s with index %d...\n", files[i].filename, file_index);
                            if (downloade_file(files[i].filename, files[i].ip) == -1) {
                                char message[1024];
                                sprintf(message, "Error downloading file %s from %s", files[i].filename, files[i].ip);
                                printf("%s\n", message);
                                log_action(message, "Error");
                                perror("Error downloading file");
                                return;
                            }
                            break;
                        }
                    }
                }
            
                char message[1024];
                sprintf(message, "downloading file %s from %s finished", files[file_index-1].filename, files[file_index-1].ip);
                log_action(message, "Success");
                printf("Downloading file %s. finished. Your file is available in the Downloads/ folder. \n", files[file_index-1].filename);
            
            }
            
            return;
        }else{
           return;
        }
}


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
            log_action("Failled to get a file informaton when try to browse the 'data' directory", "Warning");
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



size_t send_file_with_ftp(const char *fp, const int sockfd) {
    FILE *send_file_desc = fopen(fp, "rb");
    if (send_file_desc == NULL) {
        log_action("Failed to open the file in send_file function", "Error");
        perror("open file");
        return 0;
    }

    fseek(send_file_desc, 0, SEEK_END);
    long file_size = ftell(send_file_desc);
    fseek(send_file_desc, 0, SEEK_SET);
    printf("Size of file: %ld", file_size);

    send(sockfd, &file_size, sizeof(file_size), 0);
    char data[BUFFER_SIZE];
    size_t total_bytes_sent = 0;
    size_t bytes_read;

    while ((bytes_read = fread(data, 1, BUFFER_SIZE, send_file_desc)) > 0) {
        ssize_t bytes_sent = send(sockfd, data, bytes_read, 0);
        if (bytes_sent == -1) {
            log_action("Failed to send the file data", "Error");
            perror("Error in sending file.");
            fclose(send_file_desc);
            return total_bytes_sent;
        }
        total_bytes_sent += bytes_sent;
    }

    fclose(send_file_desc);
    if (total_bytes_sent == (size_t)file_size){
        char message[1024];
        sprintf(message, "File %s sent successfully with FTP", fp);
        log_action(message, "Success");
        close(sockfd);
        printf("✔️✔️✔️✔️ Transfert finished 👌👌👌✔️\n\n");
        return total_bytes_sent;
    }else{
        char message[1024];
        sprintf(message, "Error sending file %s", fp);
        log_action(message, "Error");
        perror("Error sending file");
        return total_bytes_sent;
    }
}




int send_file_with_bittorent(const char *filename, int newsockfd, char request_type_l) {
    int piece_count;
    struct stat st;
    if (stat(filename, &st) == 0) {
        piece_count = (st.st_size + PIECE_SIZE - 1) / PIECE_SIZE;
    } else {
        perror("File stat failed");
        return -1;
    }
    unsigned char **hashes = malloc(piece_count * sizeof(unsigned char *));
    if (hashes == NULL) {
        log_action("Memory allocation failed for hashes", "Error");
        fprintf(stderr, "Memory allocation failed for hashes\n");
        return 1;
    }

    for (int i = 0; i < piece_count; i++) {
        hashes[i] = malloc(SHA256_DIGEST_LENGTH);
        if (hashes[i] == NULL) {
            fprintf(stderr, "Memory allocation failed for hashes[%d]\n", i);
            // Libérer les mémoires allouées avant de retourner
            log_action("Memory allocation failed for hashes", "Error");
            for (int j = 0; j < i; j++) {
                free(hashes[j]);
            }
            free(hashes);
            return 1;
        }
    }
    printf("Compute hash of pieces\n");
    if (calculate_hashes_from_file(filename, hashes, piece_count) != 0){
        char message[150];
        sprintf(message, "Failled to calculate hashes of file %s", filename);
        log_action(message, "Error");
        return -1;
    }

    unsigned char global_hash[SHA256_DIGEST_LENGTH];
    if ( calculate_global_hash_from_file(filename, piece_count, global_hash) != 0){
        char message[150];
        sprintf(message, "Failled to calculate global hash of file %s", filename);
        log_action(message, "Error");
        return -1;
    }

    if (request_type_l == 'L') {
        size_t file_size = st.st_size;
        send(newsockfd, &piece_count, sizeof(piece_count), 0);
        send(newsockfd, &file_size, sizeof(file_size), 0);
        for (int i = 0; i < piece_count; i++) {
            send(newsockfd, hashes[i], SHA256_DIGEST_LENGTH, 0);
        }
        printf("Hash list sent firtly\n\n");

    } 

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
            if (send_piece(newsockfd, filename, index) != 0){
                char message[150];
                sprintf(message, "Failled to send piece %d of file %s", index, filename);
                log_action(message, "Error");
                return -1;
            }
        } else if(request_type == 'E') {
            for (int i = 0; i < piece_count; i++) {
                if (hashes[i] != NULL) {
                    free(hashes[i]);
                    hashes[i] = NULL;  //Pour ne ne pas doubler la libération
                }
            }
            if (hashes != NULL) {
                free(hashes);
                hashes = NULL;  
            }

            printf("✔️✔️✔️✔️ Transfert finished 👌👌👌✔️\n\n");
            return 0;
        }
    }


    for (int i = 0; i < piece_count; i++) {
        free(hashes[i]);
    }
    free(hashes);

    return 0;

}



void *upload_file( ){
    int other_client_sockef_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

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
        if ((new_socket = accept(other_client_sockef_fd, (struct sockaddr*) &address, (socklen_t *) &addrlen)) < 0){
            log_action("Failled to accept incomming request", "Error");
            perror("Failled to accept the incomming upload request");
            exit(EXIT_FAILURE);
        }

        char method_type;
        recv(new_socket, &method_type, sizeof(method_type), 0);
        
        char filename[150] = {0};
        ssize_t bytes_received = recv(new_socket, filename, BUFFER_SIZE, 0);
        if( bytes_received == -1 ){
            log_action("Failled receive the incomming request filename", "Error");
            perror("recv the incomming request filename");
            close(new_socket);
            exit(EXIT_FAILURE);
        }
        printf("\n\nNew connexion for the file: %s\n", filename);
        
        pid_t pid = fork();
        if (pid < 0) {
            log_action("Failled to fork", "Error");
            perror("Failled to fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            if (method_type == 'A')
            {
                printf("FTP method choosed\n\n");
                char *filepath = search_file("data", filename);
                send_file_with_ftp(filepath, new_socket);

                sprintf(filepath, "Sucessfully send   %s to %s with FTP method",  filename, inet_ntoa(address.sin_addr));
                log_action(filepath, "Sucess");
                close(new_socket);
                exit(EXIT_SUCCESS);
            }else if (method_type == 'C'){
                printf("HTTP method choosed\n\n");
                char *filepath = search_file("data", filename);
                upload_with_http(new_socket, filepath );

                sprintf(filepath, "Sucessfully send   %s to %s with HTTP/1.1",  filename, inet_ntoa(address.sin_addr));
                log_action(filepath, "Sucess");
                // close(new_socket);
                exit(EXIT_SUCCESS);
            }  else{            
                filename[bytes_received] = '\0';
                bool request_type_l = false;
                if (filename[bytes_received - 1] == 'L') {
                    filename[bytes_received - 1] = '\0';
                    request_type_l  = true;
                }
                char *filepath = search_file("data", filename);
                if (filepath == NULL) {
                    printf("File Not Found: %s\n", filename);
                    log_action("Failled to find the incomming request file", "Error");
                    if ( request_type_l) {
                        if( ( send_file_with_bittorent(filepath, new_socket, 'L')) != 0 ) {
                            log_action("It occurs when trying to send the requested file", "Error");
                            perror("Error sending");
                            close(new_socket);
                            break;
                        }
                    }else{
                        send(new_socket, "NULL", sizeof("NULL"), 0);
                        close(new_socket);
                        exit(EXIT_FAILURE);
                    }
                    

                }else  if ( request_type_l) {            
                        if( ( send_file_with_bittorent(filepath, new_socket, 'L')) != 0 ) {
                            log_action("It occurs when trying to send the requested file", "Error");
                            perror("Error sending");
                            close(new_socket);
                            break;
                        }
                }else if( ( send_file_with_bittorent(filepath, new_socket, 'R')) != 0 ) {
                    log_action("It occurs when trying to send the requested file", "Error");
                    perror("Error sending");
                    close(new_socket);
                    break;
                }
                sprintf(filepath, "Sucessfully send   %s to %s",  filename, inet_ntoa(address.sin_addr));
                log_action(filepath, "Sucess");
                close(new_socket);
                exit(EXIT_SUCCESS);
            }
        }
        printf("Listen to others connexion request .. \n");
    }
    return NULL;
}


























/*

Pour implémenter un système de transfert de fichiers utilisant des principes similaires à BitTorrent, nous devons écrire du code pour deux côtés : l'envoyeur et le receveur. Le côté envoyeur découpera le fichier, calculera les hashs pour chaque pièce et pour le fichier complet, et répondra aux différentes requêtes du receveur. Le côté receveur demandera et recevra les hashs et les pièces, les vérifiera, et reconstruira le fichier complet.

### Code Côté Envoyeur

#### Étape 1: Découpage du fichier et calcul des hashs

Voici comment découper un fichier et calculer les hashs pour chaque pièce ainsi que pour le fichier complet :

c

*/









/*
#### Étape 2: Écouter et traiter les requêtes

Le serveur doit être capable d'écouter et de traiter différentes requêtes du client (demande de hash global, demande de liste des hashs, et demande de morceaux spécifiques).

c
*/



/*
### Code Côté Receveur

#### Étape 1: Recevoir les hashs et le hash global

Le client receveur demandera d'abord les hashs et le hash global, puis demandera les morceaux manquants.

c

*/


/*

#### Étape 2: Vérifier les hashs et reconstruire le fichier

Le client vérifie les hashs reçus et reconstruit le fichier.

c
*/

/*
### Conclusion

Ce code offre une implémentation de base pour envoyer et recevoir de grandes quantités de données en morceaux, avec des vérifications d'intégrité utilisant des hashs. L'envoyeur découpe le fichier, calcule les hashs pour chaque morceau, et répond aux requêtes des clients pour des listes de hashs, le hash global, ou des morceaux spécifiques. Le receveur demande et vérifie les hashs, puis demande et stocke chaque morceau, reconstruisant le fichier complet et vérifiant son intégrité.

*/











void progress_bar_http(long long current, long long total) {
    int barWidth = 70;
    float progress = (float)current / total;

    printf("[ ");
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) printf("▓");
        else if (i == pos) printf("_");
        else printf(" ");
    }
    printf(" ] %0.2f%%\r", progress * 100.0);
    fflush(stdout);
}






size_t download_file_with_http(const char *ip, const int sockfd, const char *filename) {
    char request[BUFFER_SIZE], response[BUFFER_SIZE];

    int header_end = 0;
    size_t bytes_received = 0, total_size = 0, downloaded_size = 0;

    // // Send HTTP GET request
    // snprintf(request, sizeof(request), "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", filename, ip);
    // send(sockfd, request, strlen(request), 0);

    char path[MAX_FILE_SIZE];
    snprintf(path, sizeof(path), "Downloads/%s", filename);
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        perror("File opening failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("HTTP method: HTTP Request Sent\n\n");
    // Receive response
    while ((bytes_received = recv(sockfd, response, BUFFER_SIZE, 0)) > 0) {
        if (!header_end) {
            // Find end of header
            char *header_end_ptr = strstr(response, "\r\n\r\n");
            if (header_end_ptr) {
                header_end = 1;
                size_t header_size = header_end_ptr - response + 4;
                fwrite(header_end_ptr + 4, 1, bytes_received - header_size, fp);
                downloaded_size += bytes_received - header_size;

                // Extract Content-Length from header
                char *content_length_str = strstr(response, "Content-Length: ");
                if (content_length_str) {
                    content_length_str += 16; // Length of "Content-Length: "
                    char *content_length_end = strstr(content_length_str, "\r\n");
                    if (content_length_end) {
                        char content_length_val[32];
                        strncpy(content_length_val, content_length_str, content_length_end - content_length_str);
                        content_length_val[content_length_end - content_length_str] = '\0';
                        total_size = strtoul(content_length_val, NULL, 10);
                    }
                }
            }
        } else {
            fwrite(response, 1, bytes_received, fp);
            downloaded_size += bytes_received;
        }

        // Display progress bar
        if (total_size > 0) {
            progress_bar_http(downloaded_size, total_size);
        }
        if (total_size == downloaded_size && total_size != 0)
        {            
            fclose(fp);
            close(sockfd);
            printf("\nDownload complete!\n");
            return downloaded_size;
        }
        
    }
    printf("\nDownload complete!\n");

    fclose(fp);
    close(sockfd);

    return downloaded_size;

}













size_t upload_with_http(int client_socket, const char *filename) {
    printf("HTTP method: begin\n\n");
    size_t bytes_read = 0, bytes_written = 0;
    char buffer[BUFFER_SIZE];
    // while ((bytes_written = (size_t)recv(client_socket, &buffer[bytes_read], MAX_FILE_SIZE, 0)) > 0) {
    //     bytes_read = bytes_read + bytes_written;
    // }
    // if (bytes_read <= 0) {
    //     perror("Reading from socket failed");
    //     return 0;
    // }
    // buffer[bytes_read] = '\0';

    printf("HTTP method: HTTP request received\n\n");
    size_t total_bytes = 0;

    // Parse HTTP request
    // if (strncmp(buffer, "GET ", 4) == 0) {
        FILE *fp = fopen(filename, "rb");
        if (!fp) {
            const char *not_found_response = "HTTP/1.1 404 Not Found\r\n\r\nFile not found!";
            send(client_socket, not_found_response, strlen(not_found_response), 0);
            perror("HTTP/1.1 404 Not Found");
            return 0;
        }

        // Get file size
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        printf("HTTP method: Begin to send headers\n\n");
        // Send HTTP response headers
        char header[BUFFER_SIZE];
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Length: %ld\r\n"
                 "Content-Type: application/octet-stream\r\n"
                 "Connection: close\r\n\r\n", file_size);
        send(client_socket, header, strlen(header), 0);

        // Send file content
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            send(client_socket, buffer, bytes_read, 0);
            total_bytes += bytes_read;
        }

        fclose(fp);
        char message[1024];
        sprintf(message, "File %s sent successfully with HTTP", filename);
        log_action(message, "Success");
        printf("✔️✔️✔️✔️ Transfer finished 👌👌👌✔️\n\n");
        printf("File %s uploaded successfully!\n", filename);
    // } else {
    //     const char *bad_request_response = "HTTP/1.1 400 Bad Request\r\n\r\nUnsupported request!";
    //     send(client_socket, bad_request_response, strlen(bad_request_response), 0);
    //     char message[1024];
    //     sprintf(message, "File %s sent failed with HTTP", filename);
    //     log_action(message, "Error");
    //     printf("✔️✔️✔️✔️ Transfer Failed 😓 😓.\n\n");
    //     printf("An error occurred when trying to transfer the file: %s!\n", filename);
    // }

    return total_bytes;
}