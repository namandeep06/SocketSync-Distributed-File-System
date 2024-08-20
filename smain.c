#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
 
#define BUFFER_SIZE 10025
// Made the functions for various processes that will take place in the program
void prcclient(int client_socket);
void forward_file(int client_socket, const char *destination, const char *server_ip, int server_port, char *, char *, char *);
void send_file(int socket, const char *filename);



void create_tar(const char *source_dir, const char *tar_file, const char *file_type) {
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "find %s/smain -name '*.%s' | xargs tar -cvf %s", source_dir, file_type, tar_file);
    fprintf(stderr,command,strlen(command));
   
    system(command);
}


void fforward_file(int client_socket, const char *server_ip, int server_port, const char *command) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    fprintf(stderr,"helllo %s",command);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {

        perror("Socket creation failed");
        write(client_socket, "ERR", 3);
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
  
  
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(sock);
        write(client_socket, "ERR", 3);
        return;
    }

  
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
      
        perror("Connection failed");
        close(sock);
        write(client_socket, "ERR", 3);
        return;
    }
    if (write(sock, command, strlen(command)) < 0) {
        // fprintf(stderr,"on line 440 %s",command); 
        perror("Write to server failed");
        close(sock);
        write(client_socket, "ERR", 3);
        return;
    }

    //char response[4] = {0};
    // if (read(sock, response, 3) <= 0 || strcmp(response, "FIL") != 0) {
    //     write(client_socket, "ERR", 3);
    //     close(sock);
    //     return;
    // }
 
    // write(client_socket, "FIL", 3);

    ssize_t bytes_read;
    while ((bytes_read = read(sock, buffer, BUFFER_SIZE)) > 0) {
        write(client_socket, buffer, bytes_read);
    }

    close(sock);
}



void forward_comand(int client_socket, const char *destination, const char *server_ip, int server_port, char *full_path, char *filename, char *shortcmdbuffer)
{
    // storing connection details
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    // error handling
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        return;
    }
    // connection information
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    // if the address is invalid or not supported
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address or address not supported");
        close(sock);
        return;
    }
    // making the connection
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return;
    }
    // Writinf into the files
    if (write(sock, shortcmdbuffer, 200) < 0)
    {
        perror("Write to server failed");
        close(sock);
        return;
    }
    while ((bytes_read = read(sock, buffer, BUFFER_SIZE)) > 0)
    {
        write(client_socket, buffer, bytes_read);
    }
    // calling send file function to write in the file
    // send_file(sock, full_path);
    close(sock);
}
 



// This function is responsible of downloading the file

void forward_and_receive_file(int client_socket, const char *destination, const char *server_ip, int server_port, char *full_path, char *filename, char *shortcmdbuffer)
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    // maintaining the socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        return;
    }
 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    // if some error takes place in connecting
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address or address not supported");
        close(sock);
        return;
    }
    // connecting it to the client
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return;
    }
    // error handling when the server fails to write
    if (write(sock, shortcmdbuffer, 200) < 0)
    {
        perror("Write to server failed");
        close(sock);
        return;
    }
    // This function is responsible of recieving the file from subserver, wherever located and send it back to the client server
    ssize_t bytes_read;
    while ((bytes_read = read(sock, buffer, BUFFER_SIZE)) > 0)
    {
        write(client_socket, buffer, bytes_read);
    }
    close(sock);
}
// This function is responsible for creating the directory paths to store the files,
void create_directories(const char *path)
{
    char temp[512];
    // This part is copying the input path into a temporary buffer
    snprintf(temp, sizeof(temp), "%s", path);
    // looping over the temporary path
    for (char *p = temp + 1; *p; p++)
    {
        // considering '/' as directory seperator
        if (*p == '/')
        {
            // temporarily replacing '/' with null character to create a sub-path of the files
            *p = '\0';
            if (mkdir(temp, 0755) < 0 && errno != EEXIST)
            {
                // error handling, if not able to create the directory
                perror("Failed to create directory");
                exit(EXIT_FAILURE);
            }
            // Restoring the / character for next iteration
            *p = '/';
        }
    }
    // making the directory with the path given
    if (mkdir(temp, 0755) < 0 && errno != EEXIST)
    {
        // error if mkdir fails
        perror("Failed to create directory");
        exit(EXIT_FAILURE);
    }
}
// This function is basically made to handle the requests of the user to uplaod the file or download the file
void prcclient(int client_socket)
{
   
    while(1){
         char pathname[256];
    // buffer and shortcmdbuffer are used to store data received from the client and to manipulate the commands.
    char buffer[BUFFER_SIZE];
    char shortcmdbuffer[BUFFER_SIZE];
    // filename and destination hold the file name and its destination directory.
    char filename[256];
    char destination[512];
    // the number of bytes read
    int bytes_read;
    // full_path and full_dir_path store the complete paths for file operations.
    char full_path[512];
    char full_dir_path[512];
    // file_fd is the file descriptor for the file being handled.
    int file_fd;
    // home_dir holds the path to the user's home directory, retrieved using getenv("HOME")
    const char *home_dir = getenv("HOME");
    // The function reads the command sent by the client. If the read operation is successful (bytes_read > 0), it checks the command type (ufile or dfile) using sscanf
 
 
        bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
            // Check if the client has disconnected
            if (bytes_read <= 0)
            {
                printf("Client disconnected\n");
                // break;
            }
            // ensures that the data read into the buffer is treated as a valid C string.
            buffer[bytes_read] = '\0';
    
            // Determine if the command is ufile or dfile
            if (sscanf(buffer, "ufile %s %s", filename, destination) == 2)
            {
                strcpy(shortcmdbuffer, buffer);
                if (strstr(filename, ".c"))
                {
                    // if it is a c file then it will work accordingly
                    if (!home_dir)
                    {
                        perror("Failed to get home directory");
                        // close(client_socket);
                        return;
                    }
                    // This statement is making path to store the files
                    snprintf(full_path, sizeof(full_path), "%s/smain/%s/%s", home_dir, destination, filename);
                    snprintf(full_dir_path, sizeof(full_dir_path), "%s/smain/%s", home_dir, destination);
                    fprintf(stderr, "The full path is %s ,the destination is %s filename is %s and fulldirectory path is  %s", full_path, destination, filename, full_dir_path);
                    // directories are createc
                    create_directories(full_dir_path);
                    // Opening the file
                    file_fd = open(full_path, O_WRONLY | O_CREAT, 0644);
                    // if unable to open the file
                    if (file_fd < 0)
                    {
                        perror("File open failed 87");
                        // close(client_socket);
                        return;
                    }
                    // reading the bytes to store it in the destined location
                    while ((bytes_read = read(client_socket, buffer, BUFFER_SIZE)) > 0)
                    {
                        if (write(file_fd, buffer, bytes_read) < 0)
                        {
                            perror("File write failed");
                            close(file_fd);
                            close(client_socket);
                            return;
                        }

                     
                        // When bytes read exceeds the buffer size then we can break, so that it can write
                        if (bytes_read < BUFFER_SIZE)
                        {
                            break;
                        }
                    }
                 
                    // closing the fd
                    close(file_fd);
                    // Giving the response to the client
                    write(client_socket, "File stored in Smain server.", strlen("File stored in Smain server."));
                }
                // if the file is pdf then act accordingly
                else if (strstr(filename, ".pdf"))
                {
                    // Developing the paths to the file storing processes
                    snprintf(full_path, sizeof(full_path), "%s/smain/tmp/%s", home_dir, filename);
                    snprintf(full_dir_path, sizeof(full_dir_path), "%s/smain/tmp", home_dir);
                    // creating the directories
                    create_directories(full_dir_path);
                    // opening the file
                    file_fd = open(full_path, O_WRONLY | O_CREAT, 0644);
                    // error handling, if in opening the file
                    if (file_fd < 0)
                    {
                        // error handling
                        perror("File open failed");
                        close(client_socket);
                        return;
                    }
                    // Reading the bytes
                    while ((bytes_read = read(client_socket, buffer, BUFFER_SIZE)) > 0)
                    {
                        if (write(file_fd, buffer, bytes_read) < 0)
                        {
                            // error occurred if the write was failed
                            perror("File write failed");
                            close(file_fd);
                            close(client_socket);
                            return;
                        }
                        // if bytes read were more than required
                        if (bytes_read < BUFFER_SIZE)
                        {
                            break;
                        }
                        //fprintf(stderr, "For the confirmation, the number of bytes read are: %d", bytes_read);
                    }
                    close(file_fd);
                    // Calling the client using forward function to forward the files, the port number of spdf is defined already
                    forward_file(client_socket, destination, "10.60.8.51", 5003, full_path, filename, shortcmdbuffer);
                    write(client_socket, "File stored in Smain server.", strlen("File stored in Smain server."));
                }
                // if any text file is in question
                else if (strstr(filename, ".txt"))
                {
                    snprintf(full_path, sizeof(full_path), "%s/smain/tmp/%s", home_dir, filename);
                    snprintf(full_dir_path, sizeof(full_dir_path), "%s/smain/tmp", home_dir);
                    create_directories(full_dir_path);
                    file_fd = open(full_path, O_WRONLY | O_CREAT, 0644);
                    if (file_fd < 0)
                    {
                        perror("File open failed");
                        close(client_socket);
                        return;
                    }
                    while ((bytes_read = read(client_socket, buffer, BUFFER_SIZE)) > 0)
                    {
                        if (write(file_fd, buffer, bytes_read) < 0)
                        {
                            perror("File write failed");
                            close(file_fd);
                            close(client_socket);
                            return;
                        }
                        if (bytes_read < BUFFER_SIZE)
                        {
                            break;
                        }
                    }
                    close(file_fd);
                    forward_file(client_socket, destination, "10.60.8.51", 5004, full_path, filename, shortcmdbuffer);
                    write(client_socket, "File stored in Smain server.", strlen("File stored in Smain server."));
                }
                else
                {
                    // error handling
                    write(client_socket, "Unsupported file type.", strlen("Unsupported file type."));
                }
            }
            // now if downloading the file is task
            else if (sscanf(buffer, "dfile %s", filename) == 1)
            {
                // copying the content for later use
                strcpy(shortcmdbuffer, buffer);
                // If it is a c file
                if (strstr(filename, ".c"))
                {
                    // constructing the path to the c file
                    snprintf(full_path, sizeof(full_path), "%s/smain/%s", home_dir, filename);
                    // calling the send_file function
                    send_file(client_socket, full_path);
                    // write(client_socket,"File downloaded successfully", sizeof("File downloaded successfully"));
                }
                // if the file is pdf file
                else if (strstr(filename, ".pdf"))
                {
                    // Calling the function made
                    forward_and_receive_file(client_socket, "", "10.60.8.51", 5003, full_path, filename, shortcmdbuffer);
                }
                else if (strstr(filename, ".txt"))
                {
                    // Handle forwarding request to Stext server and send the file back
                    forward_and_receive_file(client_socket, "", "10.60.8.51", 5004, full_path, filename, shortcmdbuffer);
                }
                else
                {
                    // error handling
                    write(client_socket, "Unsupported file type.", strlen("Unsupported file type."));
                }
            }
            else if (sscanf(buffer, "display %s", pathname) == 1)
            {
                handle_display_command(client_socket, pathname);
            }
             else if (sscanf(buffer, "exit") == 1)
            {
                // close(client_socket);
                break;
            }
            else if (sscanf(buffer, "rmfile %s", filename) == 1) {
                strcpy(shortcmdbuffer,buffer);
                if (strstr(filename, ".c")) {
                    snprintf(full_path, sizeof(full_path), "%s/smain/%s", home_dir, filename);
                    if (remove(full_path) == 0) {
                        write(client_socket, "File removed from Smain server.", strlen("File removed from Smain server."));
                    } else {
                        write(client_socket, "File removal failed.", strlen("File removal failed."));
                    }
                } else if (strstr(filename, ".pdf")) {
                    // Forward the delete request to Spdf server
                    forward_comand(client_socket, "", "10.60.8.51", 5003, full_path, filename, shortcmdbuffer);
                } else if (strstr(filename, ".txt")) {
                    // Forward the delete request to Stext server
                    forward_comand(client_socket, "", "10.60.8.51", 5004, full_path, filename, shortcmdbuffer);
                } 
            } else if (sscanf(buffer, "dtar %s", filename) == 1) { // here filname means filetype
                strcpy(shortcmdbuffer,buffer);
                if (strcmp(filename, ".c") == 0) {
                    char tar_file[512];
                    snprintf(tar_file, sizeof(tar_file), "%s/smain/cfiles.tar", home_dir);
                    create_tar(home_dir, tar_file, "c");
                    
                    send_file(client_socket, tar_file);
                } else if (strcmp(filename, ".pdf") == 0) {
                    fforward_file(client_socket, "10.60.8.51", 5003, shortcmdbuffer);
                } else if (strcmp(filename, ".txt") == 0) {
                    fforward_file(client_socket, "10.60.8.51", 5004, shortcmdbuffer);
                } else {
                    write(client_socket, "ERR", 3);
                    write(client_socket, "Unsupported file type.", strlen("Unsupported file type."));
                }
            }
            else
            {
                // The response to the client
                write(client_socket, "Invalid command.", strlen("Invalid command."));
            }
        }
    
    close(client_socket);
}
 
// Function send file
void send_file(int socket, const char *filename)
{
    // opening the file
    int file_fd = open(filename, O_RDONLY);
    // error, if found any
    if (file_fd < 0)
    {
        perror("File open failed");
        write(socket, "ERR", 3);
        return;
    }
    // creating the buffersize
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    // writing into the file
    write(socket, "FIL", 3);
    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0)
    {
        write(socket, buffer, bytes_read);
    }
 
    close(file_fd);
}
// the function for forwarding the files
void forward_file(int client_socket, const char *destination, const char *server_ip, int server_port, char *full_path, char *filename, char *shortcmdbuffer)
{
    // storing connection details
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    // error handling
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        return;
    }
    // connection information
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    // if the address is invalid or not supported
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address or address not supported");
        close(sock);
        return;
    }
    // making the connection
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return;
    }
    // Writinf into the files
    if (write(sock, shortcmdbuffer, 200) < 0)
    {
        perror("Write to server failed");
        close(sock);
        return;
    }
    // calling send file function to write in the file
    send_file(sock, full_path);
    close(sock);
}

// Function to execute a command and send the output to a client socket
void print_files(int client_socket, const char *command, const char *header)
{
    char line[BUFFER_SIZE]; // Buffer to store each line of output
    FILE *fp = popen(command, "r"); // Check if the pipe was opened successfully
    if (fp == NULL)
    {
        perror("popen"); // Print error message if pipe opening failed
        return;
    }

   // write(client_socket, header, sizeof(header));
    // Read each line of the command's output
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        line[strcspn(line, "\n")] = 0; // Removing newline character from the end of the line
        // fprintf(stderr, " %s", line);
        write(client_socket, line, sizeof(line));
    }
    pclose(fp); // Close the pipe
}
 
// Function to handle the display command from the client
void handle_display_command(int client_socket, const char *pathname)
{
    char command_buf[BUFFER_SIZE];
 
    // Step 1: Get .c files
    sprintf(command_buf, "find ~/smain/%s -type f -name '*.c' -exec basename {} \\; 2>/dev/null", pathname);
    print_files(client_socket, command_buf, "List of .c files:");
 
    // Step 2: Get .pdf files
    sprintf(command_buf, "find ~/spdf -type f -name '*.pdf' -exec basename {} \\; 2>/dev/null");
    print_files(client_socket, command_buf, "List of .pdf files:");
 
    // Step 3: Get .txt files
    sprintf(command_buf, "find ~/stext -type f -name '*.txt' -exec basename {} \\; 2>/dev/null");
    print_files(client_socket, command_buf, "List of .txt files:");
    // Send an end-of-list marker

    write(client_socket, "END_OF_LIST", 12);
}
 
int main(int argc, char *argv[])
{
    // in the main code
    // the server connection
    int server_socket, client_socket, port_number;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    // error handling
    if (argc != 2)
    {
        fprintf(stderr, "Error! Please Consider Right Usage: %s <Port#>\n", argv[0]);
        exit(1);
    }
    // the first command will be the port number
    port_number = atoi(argv[1]);
    // error handling
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }
    // Neccesary information about the socket
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port_number);
    // Binding.. and error handling
    if (bind(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Bind failed");
        close(server_socket);
        exit(1);
    }
    // listening.. the error handling
    if (listen(server_socket, 5) < 0)
    {
        perror("Listen failed");
        close(server_socket);
        exit(1);
    }
    // printing about its usage
    printf("Smain server listening on port %d\n", port_number);
    // Loop to continuously accepting the client connections
    while (1)
    {
        // waiting for the client to connect
        client_socket = accept(server_socket, (struct sockaddr *)&cli_addr, &cli_len);
        if (client_socket < 0)
        {
            perror("Accept failed");
            continue;
        }
        // Used fork because:while the child process manages communication with the current client, the parent process keeps listening for new client connections.
        int pid = fork();
        perror("pid");
        if (pid < 0)
        {
            perror("Fork failed");
            close(client_socket);
            continue;
        }
        if (pid == 0)
        {
            // close(server_socket);
            prcclient(client_socket);
            exit(0);
        }
        else
        {
            perror("");
            // close(client_socket);
        }
    }
    close(server_socket);
    return 0;
}