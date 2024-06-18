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
#include <setjmp.h>   // to avoid CRTL+C command in terminal
#include <openssl/sha.h>   // for calculate sha256 hash


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

#define PIECE_SIZE 262144


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
    size_t filesize;
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


/// @brief Send the specified file to the specified socket using bitTorrent protocol
/// @param fp : relative path to the file
/// @param sockfd : socket to send the file
/// @return int : 1 if successful, 0 otherwise
int send_file_with_bittorent(const char *fp, int sockfd, char request_type_l);


// start listening for the the download demand and serve it
void *upload_file();

/// @brief calculate the hash of the given data 
/// @param data 
/// @param length 
/// @param hash 
/// @return int : 1 if successful, 0 otherwise
int calculate_hash(const char *data, size_t length, unsigned char *hash) ;


/// @brief Calculte the hash of the given filename 
/// @param filename  
/// @param piece_count  the number of piece of the file to facilitate the computation
/// @param global_hash  the global hash to return
/// @return 0 if everting went correct - 1 otherwise
int  calculate_global_hash_from_file(const char *filename, int piece_count, unsigned char *global_hash) ;

/// @brief Write the piece of the given filename and index into socket buffer
/// @param sockfd 
/// @param filename 
/// @param piece_index 
/// @return  0 if successful write the data into the buffer or -1 if false
int send_piece(int sockfd, const char *filename, int piece_index);

/// @brief Write  hash list store in memory  **hashes in the socket buffer 
/// @param sockfd 
/// @param hashes 
/// @param piece_count 
/// @return 0 if successful write the data into the buffer or -1 if false
int send_hash_list(int sockfd, unsigned char **hashes, int piece_count) ;


/// @brief same as send_hash_list but just send the global hash of the file or piece
/// @param sockfd 
/// @param global_hash 
/// @return  0 if successful or -1 if false
int send_global_hash(int sockfd, unsigned char *global_hash) ;


/// @brief get and compute the hash of the file and put it in **hashes list
/// @param filename 
/// @param hashes list of hashes returned
/// @param piece_count 
/// @return 0 if successful or -1 if false
int calculate_hashes_from_file(const char *filename, unsigned char **hashes, int piece_count) ;


/// @brief  receive a file from another client know as sender with bittorent protocol
/// @param filename the name of the file
/// @param sockfd socket file descriptor
/// @return the number of bytes received or -1 if throw and error
size_t receive_file_with_bittorent(const char *filename, const int sockfd);


/// @brief  receive a file from another client know as sender with FTP protocol
/// @param filename the name of the file
/// @param sockfd socket file descriptor
/// @return the number of bytes received or -1 if throw and error
size_t receive_file_with_ftp(const char *filename, const int sockfd);

/// @brief Print the progress bar for downloaded file
/// @param received_bytes the size of the received part of the file
/// @param file_size the file size in bytes
void print_progress(size_t received_bytes, size_t file_size) ;


/// @brief Send the specified file to the specified socket using the FTP protocol
/// @param fp : relative path to the file
/// @param sockfd : socket to send the file
/// @return int : 1 if successful, 0 otherwise
size_t send_file_with_ftp(const char *fp, const int sockfd) ;
#endif