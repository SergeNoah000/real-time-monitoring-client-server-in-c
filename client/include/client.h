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
#include <signal.h>
#include <setjmp.h>


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
#define MAX_FILE_SIZE 1024
#define LOG_FILE "client_actions.log"

//notify's const
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024*(EVENT_SIZE + 16))

#define MAX_FILES 100

// Structure for filereceived information
typedef struct  {
    char ip[INET_ADDRSTRLEN];
    size_t size;
    char filename[256];
    char modification_date[42];
}FileInfo;

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

// share the file information to the server neither stopping nor occupied de console output
void share_data_contents(int socket_fd);


// Display the console diagram of the file sharing system
void displayDiagram();



// Function to request and display the list of files available from the server
void view_files_list(int socket_fd);

// Return relative path from  the caller dir and filename
/// @brief 
/// @param char * dir_path 
/// @param char * filename 
/// @return char *
char *search_file(const char *dir_path, const char *filename) ;


/// @brief Send the specified file to the specified socket
/// @param fp : relative path to the file
/// @param sockfd : socket to send the file
/// @return int : 1 if successful, 0 otherwise
size_t send_file(const char *fp, int sockfd);


// start listening for the the download demand and serve it
void *upload_file();
#endif