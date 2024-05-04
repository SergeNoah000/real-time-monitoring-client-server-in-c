/*
        ---- Licence UY1 -----
    This file is one of the HomeWork of the Programming Computing Course for
    Computer Science field in Yaoundé I University. 

    You can use it if and only if you include the participation of Computer Sciences
    of Yaoundé I University Especially Serge Noah, matricule: 20U2669 it creator
*/

#ifndef SERVER_H

#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>



#define MAX_THREADS 10
#define BUFFER_SIZE 10240
#define LOG_FILE "server_action.log"
#define FILE_DATA "server_data.log"



typedef struct {
    int client_socket;
    struct sockaddr_in client_address;
} Connection;

typedef struct {
    char filename[256];
    int size;
    char modification_date[30];
} file_infos;


// Structure for file information
struct FileInfoToSend {
    char ip[16]; // As it's an IPv4 address
    int size;
    char filename[256];
    char modification_date[32]; // Increased buffer size to accommodate date and time
};





//Declaration of procedures and functions that will be used in the code.


//clear num_lines on the current console
void clear_last_lines(int num_lines);


//convert time to a valid  string [ Dy YYYY-mm-dd HH:MM:SS ]
void convert_time(char *str,  size_t size, time_t time);


//Log an action into the  server log file
void log_action(const char *action, char* danger) ;


//Encode a given string in ascci to a corresponding in utf-8
char *ascii_to_utf8(const char *ascii_str) ;


//log the received message of a client to the server_data.log file
void replace_log_message(const char *ip_address, const char *new_message);



// Function to read and send  formated content to the client
void read_and_send_files_infos(const char *filename, int socket_fd) ;


#endif  //SERVER_H

