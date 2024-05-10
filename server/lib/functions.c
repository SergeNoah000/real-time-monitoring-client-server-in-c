/*
        ---- Licence UY1 -----
    This file is one of the HomeWork of the Programming Computing Course for
    Computer Science field in Yaoundé I University. 

    You can use it if and only if you include the participation of Computer Sciences
    of Yaoundé I University Especially Serge Noah, matricule: 20U2669 it creator
*/


#include "../include/server.h"





void clear_last_lines(int num_lines) {
    printf("\033[%dA\033[K", num_lines);
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




// Decode ASCII string to UTF-8
char *ascii_to_utf8(const char *ascii_str) {
    size_t ascii_len = strlen(ascii_str);
    char *utf8_str = (char *)malloc(ascii_len + 1); // Allocate memory for UTF-8 string
    if (utf8_str == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t utf8_index = 0;
    for (size_t i = 0; i < ascii_len; i++) {
        if (ascii_str[i] == '\\' && ascii_str[i + 1] == 'x') {
            char hex_str[3];
            hex_str[0] = ascii_str[i + 2];
            hex_str[1] = ascii_str[i + 3];
            hex_str[2] = '\0';
            int code = strtol(hex_str, NULL, 16);
            utf8_str[utf8_index++] = code;
            i += 3; // Skip the escape sequence
        } else {
            utf8_str[utf8_index++] = ascii_str[i];
        }
    }
    // utf8_str[utf8_index] = '\0'; // Null-terminate UTF-8 string
    return utf8_str;
}



void replace_log_message(const char *ip_address, const char *new_message) {
    FILE *file_data = fopen(FILE_DATA, "r+");
    if (file_data == NULL) {
        log_action("Failled to open the 'server_data.log' file", "Error");
        perror("Failled to open 'server_data.log' file");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    char temp_file[] = "temp.log";
    FILE *temp = fopen(temp_file, "w");
    if (temp == NULL) {
        log_action("Failled to open the 'tmp.log' file", "Error");
        perror("Failled to open 'tmp.log'");
        exit(EXIT_FAILURE);
    }


    bool ip_found = false;

    // Copy lines to temp file, replacing the message if the IP is found
    bool writing_old_message = false;
    while (fgets(line, sizeof(line), file_data)) {
        if (strstr(line, ip_address) != NULL) {
            ip_found = true;
            writing_old_message = true;
        } else if (writing_old_message && strstr(line, "::") != NULL) {
            writing_old_message = false;
            fprintf(temp, "%s::\n%s\n\n", ip_address, new_message);
            fprintf(temp, "%s", line); 
        } else if (!writing_old_message) {
            fprintf(temp, "%s", line); // Write other lines to temp file
        }
    }

    fprintf(temp, "\0"); 
    fprintf(file_data, "\0"); 

    //Add the new ip and message if its don't exist in the server_data.log
    fprintf(file_data, "\n\n%s::\n%s", ip_address, new_message);


    fclose(file_data);
    fclose(temp);

    if (ip_found)
    {
        if (remove(FILE_DATA) != 0) {
            log_action("Failled to remove 'server_data.log' file", "Error");
            perror(" Failled to remove 'server_data.log'");
            exit(EXIT_FAILURE);
        }

        if (rename(temp_file, FILE_DATA) != 0) {
            log_action("Failled to rename 'tmp.log' file", "Error");
            perror(" Failled to rename 'tmp.log'");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        if (remove(temp_file) != 0) {
            log_action("Failled to remove 'tmp.log' file", "Error");
            perror(" Failled to remove 'tmp.log'");
            exit(EXIT_FAILURE);
        }
    }
    

}





// Function to read and format the content of the file
void read_and_send_files_infos(const char *filename, int socket_fd) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[512]; 
    int file_count = 0;
    long file_size = 0;

    // Counting the number of lines in the file
    while (fgets(line, sizeof(line), file)) {
        // Ignore empty lines
        if (strlen(line) > 1)
            file_count++;
            file_size += strlen(line);

    }



    // Reset the file pointer to the beginning of the file
    fseek(file, 0, SEEK_SET);

    // Allocate memory dynamically for the array of FileInfo structures
     FileInfo *files = malloc(file_count * sizeof(FileInfo));
    if (files == NULL) {
        log_action("Failled to allocate the memory of FileInfoToSend", "Error");
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    // Read and process each line of the file
    int i, current_file_index = 0;
    char current_ip[25] = {0};
    while (fgets(line, sizeof(line), file)) {
        // Ignore empty lines
        if (strlen(line) <= 1){
            continue;
        }
        // Remove the newline character at the end
        line[strcspn(line, "\n")] = '\0';

        // If the line starts with "::", it's a new IP section
        if (strstr(line, "::") != NULL) {
            // Increment the file index and copy the IP address
            strcpy(current_ip, strtok(line, "::"));
        } else {
            // Otherwise, it's a file information line
            sscanf(line, "%s %ld %[A-Za-z0-9 -:]",
                   files[current_file_index].filename,
                   &files[current_file_index].size,
                   files[current_file_index].modification_date);
            
            strcpy(files[current_file_index].ip, current_ip);
            current_file_index++;
        }
    }

    fclose(file);

      // Send number of files
    if (send(socket_fd, &current_file_index, sizeof(current_file_index), 0) != sizeof(current_file_index)) {
        perror("Send failed");
        exit(EXIT_FAILURE);
    }
    
    // Send each file info
    for (int i = 0; i < current_file_index; ++i) {
        if (send(socket_fd, &files[i], sizeof(FileInfo ), 0) != sizeof( FileInfo)) {
            log_action("Successfully send list of files", "Success");
            perror("Send failed");
            exit(EXIT_FAILURE);
        }
    }
    free(files);

    return;
}


