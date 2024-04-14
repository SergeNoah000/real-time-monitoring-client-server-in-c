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
    FILE *file = fopen(FILE_DATA, "r+");
    if (file == NULL) {
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

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, ip_address) != NULL) {
            ip_found = true;
            fprintf(temp, "%s", line); // write the finded line in tmp file

            // ignore the lines after the ip's line
            char line1;
            while (fgets(line, sizeof(line), file) ){
                line1 = line[0];
                if (line1 == '\n' && line[0] != '\n')
                {
                    break;
                }
                 
            };

            //write the new message in the tmp file
            fprintf(temp, "%s", new_message); 
        } else {
            fprintf(temp, "%s", line); // write the others lines on tmp file
        }
    }

    fprintf(temp, "\0"); 
    fprintf(file, "\0"); 

    //printf("Bonjour, on est dans la fonction. Value of ip_bound: %s", ip_found ? "true" : "false" );

    //Add the new ip and message if its don't exist in the server_data.log
    if (!ip_found) {
        fprintf(file, "\n\n%s:\n%s", ip_address, new_message);
    }

    fclose(file);
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