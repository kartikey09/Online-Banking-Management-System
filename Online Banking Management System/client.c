#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/ip.h>
#include<termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 4096

void connectionHandler(int socketFD);
void hide_input(char *buffer, int size);
void clearConsole();

int main(void) {
    int socketFD;
    struct sockaddr_in server_add;
    char buffer[BUFFER_SIZE];
    char server_reply[BUFFER_SIZE];

    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD == -1) {
        perror("Could not create socket");
        return 1;
    }
    printf("Socket created.\n");

    server_add.sin_family = AF_INET;
    server_add.sin_port = htons(PORT);
    server_add.sin_addr.s_addr = htonl(INADDR_ANY);

    if (connect(socketFD, (struct sockaddr *)&server_add, sizeof(server_add)) < 0) {
        perror("Connection failed");
        close(socketFD);
        return 1;
    }
    printf("Connected to server.\n");

    connectionHandler(socketFD);    

    close(socketFD);
    printf("Connection closed");
    return 0;
}

void connectionHandler(int socketFD){
    char readBuffer[4096], writeBuffer[4096], tempBuffer[4096];
    int readBytes, writeBytes;

    do{ 
        readBytes = read(socketFD, readBuffer, sizeof(readBuffer));
        if(readBytes < 0)
            perror("unable to read server response");
        else if(readBytes == 0)
            printf("Closing the connection");
        else{
            if(strcmp(readBuffer, "Enter password: ") == 0)
                hide_input(writeBuffer, sizeof(writeBuffer));
            else{
                memset(writeBuffer, 0, sizeof(writeBuffer));
                memset(tempBuffer, 0, sizeof(tempBuffer));
                if(strcmp(readBuffer, "Client logging out...\n") == 0){
                    strcpy(writeBuffer, "");
                    close(socketFD);
                    return;
                }else if(strchr(readBuffer, '^') != NULL) {
                    if(strlen(readBuffer) != 1){
                        strncpy(tempBuffer, readBuffer, strlen(readBuffer) - 1);
                        printf("%s\n", tempBuffer);
                    }
                    strcpy(writeBuffer, "");
                }else{
                    printf("%s\n", readBuffer);
                    scanf("%s", writeBuffer);
                }                
            }
            
            writeBytes = write(socketFD, writeBuffer, sizeof(writeBuffer));
            if(writeBytes == -1){
                printf("Unable to write to server\n");
                printf("Closing the connection to the server now!\n");
                break;
            }
        }


    }while(readBytes > 0);

    close(socketFD);
}

void hide_input(char *buffer, int size) {
    struct termios oldt, newt;
    
    // Get the current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Disable echoing of characters
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Read input
    printf("Enter password: ");
    scanf("%s", buffer);

    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

void clearConsole() {
    #ifdef _WIN32
        system("cls");
    #elif defined(__linux__) || defined(__APPLE__)
        system("clear");
    #endif
}