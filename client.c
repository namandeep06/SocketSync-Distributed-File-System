#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <libgen.h>
 
#define BUFFER_SIZE 10025
 


 void receive_file_tar(int socket, const char *filename) {
    char CMDBuffer[4] = {0};
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    if (read(socket, CMDBuffer, 3) <= 0) {
        printf("Error: Failed to read server response.\n");
        return;
    }

    if (strcmp(CMDBuffer, "ERR") == 0) {
        printf("Server response: Error retrieving the file.\n");
        return;
    }

    if (strcmp(CMDBuffer, "FIL") != 0) {
        printf("Error: Unexpected server response.\n");
        return;
    }

    int file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (file_fd < 0) {
        perror("File open failed");
        printf("The file trying to open in receive is %s\n", filename);
        return;
    }

    while ((bytes_received = read(socket, buffer, BUFFER_SIZE)) > 0) {
        write(file_fd, buffer, bytes_received);
       
        if(bytes_received<BUFFER_SIZE){
            break;
        }
    }

    if (bytes_received < 0) {
        perror("Error receiving file");
        close(file_fd);
        return;
    }

    printf("File %s downloaded successfully to the current directory.\n", filename);
    close(file_fd);
}


// Function to recieve the file
void receive_file(int socket, const char *filename)
{
    // Changed the size to 4 to include the null terminator
    char CMDBuffer[4];
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    //fprintf(stderr, "20", sizeof("20"));
    read(socket, CMDBuffer, 3);
    // Null-terminate the string
    CMDBuffer[3] = '\0';
 
    //fprintf(stderr, "%s", CMDBuffer);
    // String Comparasion for debugging processes
    if (strcmp(CMDBuffer, "ERR") == 0)
    {
        // Printing the server response
        printf("Server response: Error retrieving the file.\n");
        return;
    }
    // Opening the file
    int file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (file_fd < 0)
    {
        // Error Handling
        perror("File open failed");
        printf("The file trying to open in recieve is %s", filename);
        return;
    }
    // Reading the content
    while ((bytes_received = read(socket, buffer, BUFFER_SIZE)) > 0)
    {
        write(file_fd, buffer, bytes_received);
        if (bytes_received < BUFFER_SIZE)
        {
            break;
        }
    }
    printf("File %s downloaded successfully to the current directory.\n", filename);
    close(file_fd);
}
// Function to send the filess
void send_file(int socket, const char *filename)
{
    int file_fd = open(filename, O_RDONLY);
    if (file_fd < 0)
    {
        perror("File open failed");
        return;
    }


// Reading the bytes
     char buffer[BUFFER_SIZE];
     ssize_t bytes_read;
     while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
         write(socket, buffer, bytes_read);
     }
     close(file_fd);
 }
 
// Function to handle the display of data received from the server
void handle_display(int socket)
{
    char buffer[BUFFER_SIZE]; // Buffer to store the received data
    ssize_t bytes_received; // Variable to store the number of bytes received
 
    fprintf(stderr, "Server response:\n");
    // Read data from the socket in a loop
    while ((bytes_received = read(socket, buffer, BUFFER_SIZE)) > 0)
    {
        buffer[bytes_received] = '\0';
        printf("%s\n", buffer);
        // fprintf(stderr,"%s   ----- %d", buffer, bytes_received);
        // If the received data is less than the buffer size, we assume it's the end of the list
        if (strcmp("END_OF_LIST",buffer)==0)
        {
            break; // Exit the loop
        }
   
        // Check if the read operation failed
        if (bytes_received < 0)
        {
            perror("Read failed"); // Print an error message if the read operation failed
        }
     }
}
 
// Entering the main loop and getting the arguements
int main(int argc, char *argv[])
{
    int server_socket, port_number;
    struct sockaddr_in serv_addr;
 
    if (argc != 3)
    {
        fprintf(stderr, "Warning!! The right usage is: %s <Smain_IP> <Port#>\n", argv[0]);
        exit(1);
    }
    // entering the port number
    port_number = atoi(argv[2]);
    // Conneting with the server
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }
    // Storing the necessary address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_number);
    // Error Handling
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
    {
        perror("Invalid address or address not supported");
        exit(1);
    }
    // Connecting System Call
    if (connect(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connect failed");
        exit(1);
    }
    while(1){
        char buffer[BUFFER_SIZE];
        printf("Enter the command: ");
        fgets(buffer, sizeof(buffer), stdin);
        fprintf(stderr, "%s", buffer);
        write(server_socket, buffer, strlen(buffer));
    
        // Extracting the filename from the command
        char command[16], filename[256], destination[512];
        sscanf(buffer, "%s %s %s", command, filename, destination);
        // If it is the upload file
        if (strcmp(command, "ufile") == 0)
        {
            // Calling the function
            send_file(server_socket, filename);
             int bytes_read = read(server_socket, buffer, sizeof(buffer) - 1);
            if (bytes_read < 0)
            {
                // Error Handling
                perror("Read failed");
            }
            else
            {
                buffer[bytes_read] = '\0';
                printf("Server response: %s\n", buffer);
            }
        }
        else if (strcmp(command, "dfile") == 0)
        {
            // If the dfile
            char *filename_with_extension = basename(filename);
            receive_file(server_socket, filename_with_extension);
        }
    
        else if (strcmp(command, "display") == 0)
        {
    
            write(server_socket, buffer, strlen(buffer));
            handle_display(server_socket);
        
        }
    
        else if (strcmp(command, "dtar") == 0) {
                char tar_filename[256];
                snprintf(tar_filename, sizeof(tar_filename), "%sfiles.tar", filename + 1); // filename is second argument = file_type
                receive_file_tar(server_socket, tar_filename);
                 int bytes_read = read(server_socket, buffer, sizeof(buffer) - 1);
                if (bytes_read < 0)
                {
                    // Error Handling
                    perror("Read failed");
                }
                else
                {
                    buffer[bytes_read] = '\0';
                    printf("Server response: %s\n", buffer);
                }
         }
        else if (strcmp(command, "rmfile") == 0)
        {
            write(server_socket, buffer, strlen(buffer));
            int bytes_read = read(server_socket, buffer, sizeof(buffer) - 1);
            if (bytes_read < 0)
            {
                // Error Handling
                perror("Read failed");
            }
            else
            {
                buffer[bytes_read] = '\0';
                printf("Server response: %s\n", buffer);
            }
        }
        else if (strcmp(command, "exit") == 0)
        {
            close(server_socket);
            break;
        }
        else
        {
            printf("Unsupported command.\n");
        }
        // // reading the bytes from the buffer about the server response
        // int bytes_read = read(server_socket, buffer, sizeof(buffer) - 1);
        // if (bytes_read < 0)
        // {
        //     // Error Handling
        //     perror("Read failed");
        // }
        // else
        // {
        //     buffer[bytes_read] = '\0';
        //     printf("Server response: %s\n", buffer);
        // }
    }
    return 0;
}