/*
        ---- Licence UY1 -----
    This file is one of the HomeWork of the Programming Computing Course for
    Computer Science field in Yaoundé I University. 

    You can use it if and only if you include the participation of Computer Sciences
    of Yaoundé I University Especially Serge Noah, matricule: 20U2669 it creator
*/


#ifndef CLIENT_H


#define CLIENT_H


#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h> //for file_stat struct
#include <time.h> 
#include <sys/inotify.h> // for dir watching
#include <linux/limits.h>


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


#define BUFFER_SIZE 1024
#define MAX_FILE_SIZE 300
#define LOG_FILE "client_actions.log"

//notify's const
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024*(EVENT_SIZE + 16))



// the struct that will containts the file informations.
typedef struct 
{
    char filename[MAX_FILE_SIZE];
    double filesize;
    char last_modified_date[30];

} file_infos;


//convert time to a valid  string
void convert_time(char *str,  size_t size, time_t time);



// Function to log an action with its corresponding danger level
void log_action(const char *action, char* danger);



// Encode UTF-8 string to ASCII
char *utf8_to_ascii(const char *utf8_str) ;


//convert the struct to string. In this format, i can easilly  send it to the server
char *file_infos_to_string(file_infos *files, int num_files) ;



//capture all files informations of "data" folder
char *capture_data();

#endif