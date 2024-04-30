/*
        ---- Licence UY1 -----
    This file is one of the HomeWork of the Programming Computing Course for
    Computer Science field in Yaoundé I University. 

    You can use it if and only if you include the participation of Computer Sciences
    of Yaoundé I University Especially Serge Noah, matricule: 20U2669 it creator
*/

#include <stdio.h>
#include "../include/client.h"



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






// Function to display the list of files
void view_files_list(int sock) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Sending request to the server
    char *request = "LIST_FILES";
    send(sock, request, strlen(request), 0);

    // Array to store file information
    struct FileInfo files[MAX_FILES];
    int file_count = 0;

    // Receiving the list of files from the server
    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        // Parsing and storing each line of the response
        char *token = strtok(buffer, "\n");
        while (token != NULL && file_count < MAX_FILES) {
            sscanf(token, "%s %d %s %s",
                   files[file_count].ip,
                   &files[file_count].size,
                   files[file_count].filename,
                   files[file_count].modification_date);
            token = strtok(NULL, "\n");
            file_count++;
        }

        // Displaying the list of files with index, name, and modification date
        printf("Index\tFilename\tSize (Mo)\tModification Date\n");
        printf("-------------------------------------------------\n");
        for (int i = 0; i < file_count; i++) {
            printf("%d\t%s\t\t%d\t\t%s\n", i + 1,
                   files[i].filename,
                   files[i].size,
                   files[i].modification_date);
        }

        // Interaction with the user for downloading or returning to the menu
        printf("\nEnter the index of the file to download (Ctrl+C to return to the menu): ");
        int file_index;
        scanf("%d", &file_index);

        // Code to download the file with the given index
        // To be implemented according to your specific application logic
        printf("Downloading file with index %d...\n", file_index);

        // Sending the request again to get a new list
        printf("\n");
        send(sock, request, strlen(request), 0);
    }

    // Handling receive errors
    if (bytes_received == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }
}







/*
void view_files_list(int socket_fd) {
    char buffer[BUFFER_SIZE] = {0};

    // Send request to the server to get the list of files
    send(socket_fd, "get_file_list", strlen("get_file_list"), 0);
    printf("Request sent to the server to get the list of files\n");

    // Receive the list of files from the server
    recv(socket_fd, buffer, BUFFER_SIZE, 0);
    printf("Received file list from the server\n");

    // Parse the received JSON data
    cJSON *root = cJSON_Parse(buffer);
    if (root == NULL) {
        log_action("Failed to parse JSON data received from the server", "Error");
        perror("Failed to parse JSON data received from the server");
        return;
    }

    // Display the list of files with their index, name, and last modification date
    cJSON *client_files = cJSON_GetObjectItem(root, "ip_client");
    int index = 0;
    cJSON *client_file;
    cJSON_ArrayForEach(client_file, client_files) {
        cJSON *file_name = cJSON_GetObjectItem(client_file, "nom_fichier");
        cJSON *file_size = cJSON_GetObjectItem(client_file, "taille");
        cJSON *file_last_modified = cJSON_GetObjectItem(client_file, "dernier_date_modification");
        printf("%d. Name: %s, Size: %s, Last Modified: %s\n", index, file_name->valuestring, file_size->valuestring, file_last_modified->valuestring);
        index++;
    }

    // Prompt the user to enter the index of the file to download
    int selected_index;
    printf("Enter the index of the file to download (Ctrl+C to return to the previous menu): ");
    scanf("%d", &selected_index);

    // Get the selected file from the JSON data
    cJSON *selected_file = cJSON_GetArrayItem(client_files, selected_index);
    if (selected_file == NULL) {
        printf("Invalid index\n");
        return;
    }

    cJSON *file_name = cJSON_GetObjectItem(selected_file, "nom_fichier");
    printf("Selected file to download: %s\n", file_name->valuestring);

    // Download the selected file from the server (implementation not provided)
    // Add code to download the selected file from the server

    // Free the cJSON root object
    cJSON_Delete(root);
}


*/

