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
//Function declaration for this program
void process_client(int client_socket);
void send_file(int client_socket, const char *full_path);
void remove_file(const char *path);


void create_tar(const char *source_dir, const char *tar_file, const char *file_type) {
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "find %s/spdf -name '*.%s' | xargs tar -cvf %s", source_dir, file_type, tar_file);
       fprintf(stderr,command,strlen(command));
    system(command);
}


//Function to create the directories as done in smain for PDF files
void create_directories(const char *path) {
    char temp[512];
    snprintf(temp, sizeof(temp), "%s", path);
    for (char *p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(temp, 0755) < 0 && errno != EEXIST) {
                perror("Failed to create directory");
                exit(EXIT_FAILURE);
            }
            *p = '/';
        }
    }
    if (mkdir(temp, 0755) < 0 && errno != EEXIST) {
        perror("Failed to create directory");
        exit(EXIT_FAILURE);
    }
}
//The function to process the clients
void process_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    char filename[256];
    char destination[512];
    ssize_t bytes_read;
    char full_path[512];
    char full_dir_path[512];
    int file_fd;
    const char *home_dir = getenv("HOME");
////reading the bytes from the client socket to get the command
    if ((bytes_read = read(client_socket, buffer, 200)) > 0) {
        buffer[bytes_read] = '\0';
        if (strncmp(buffer, "ufile", 5) == 0) {
            sscanf(buffer, "ufile %s %s", filename, destination);
            //Error Handling
            if (!home_dir) {
                perror("Failed to get home directory");
                close(client_socket);
                return;
            }
            //creating the full paths to store the files
            snprintf(full_path, sizeof(full_path), "%s/spdf/%s/%s", home_dir, destination, filename);
            snprintf(full_dir_path, sizeof(full_dir_path), "%s/spdf/%s", home_dir, destination);
            //creating the directories by calling rh function
            create_directories(full_dir_path);
            //opening the file
            file_fd = open(full_path, O_WRONLY | O_CREAT, 0644);
            fprintf(stderr,"%s",full_path);
            if (file_fd < 0) {
                perror("File open failed1");
                close(client_socket);
                return;
            }
            //Reading the bytes
            while ((bytes_read = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
                if (write(file_fd, buffer + 3, bytes_read - 3) < 0) {
                    perror("File write failed");
                    close(file_fd);
                    close(client_socket);
                    return;
                }
                //When the bytes_read exceed
                if(bytes_read<BUFFER_SIZE){
                        break;
                }
            }
            close(file_fd);
            //giving the confirmation about storing the file
            write(client_socket, "File stored in spdf server.", strlen("File stored in spdf server."));
        
        }//If the command is download the file then it will create the path 
        else if (strncmp(buffer, "dfile", 5) == 0) {
            sscanf(buffer, "dfile %s %s", filename, destination);
            
            snprintf(full_path, sizeof(full_path), "%s/spdf/%s/%s", home_dir, destination, filename);
            //Then the function would be called
                send_file(client_socket, full_path);
           
        } else if (strncmp(buffer, "rmfile", 6) == 0) {
            sscanf(buffer, "rmfile %s %s", filename, destination);
            
            snprintf(full_path, sizeof(full_path), "%s/spdf/%s/%s", home_dir, destination, filename);
            
            remove_file(full_path);
            write(client_socket, "File deleted from Spdf server.", strlen("File deleted from Spdf server."));
        
        } else if (strncmp(buffer, "dtar", 4) == 0) {
            // File name is filetype
            sscanf(buffer, "dtar %s", filename);
            
            snprintf(full_path, sizeof(full_path), "%s/spdf/pdf.tar", home_dir);
            create_tar(home_dir, full_path, "pdf");
            send_file(client_socket, full_path);
        }
        else {
            //Otherwise passing the error message
            write(client_socket, "Unsupported command.", strlen("Unsupported command."));
        }
    }

    close(client_socket);

}
//function to send the file
void send_file(int socket, const char *filename) {
    //opening the file
    int file_fd = open(filename, O_RDONLY);
    if (file_fd < 0) {
        //error handling if occurred
        perror("File open failed11");
        write(socket, "ERR", 3);
        return;
    }
//fprintf(stderr,"\n112\n",sizeof("112"));
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    write(socket, "FIL", 3);
    //fprintf(stderr,"\n116\n",sizeof("116"));
    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
      //  write(socket, buffer, bytes_read);
        if (write(socket, buffer, bytes_read) < 0) {
            perror("File write failed");
            close(file_fd);
            close(socket);
            return;
        }
        //If it exceeds then print error
        if(bytes_read<BUFFER_SIZE){
                        break;
        }
    }
    close(file_fd);
}
void remove_file(const char *path) {
    if (unlink(path) < 0) {
        perror("File removal failed");
    }
}
//Main Block
int main(int argc, char *argv[]) {
    //Storing the address
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
     //Usage error
    if (argc != 2) {
        fprintf(stderr, "Warning: The right usage: %s <Port#>\n", argv[0]);
        exit(1);
    }
//error in socket creation
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(1);
    }
//This all is used for error handling
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt error");
        exit(1);
    }
//Storing the address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(atoi(argv[1]));
//Binding
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(1);
    }
//Listening
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("Spdf server listening on port %s\n", argv[1]);
//This loop will accept the multiple children
    while (1) {
        //It will accept connections
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        if (fork() == 0) {
            //child should not handle the listening process
            close(server_fd); 
            process_client(client_socket);
            exit(0);
        } else {
            close(client_socket);
        }
    }

    close(server_fd);
    return 0;
}
