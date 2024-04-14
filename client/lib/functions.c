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



