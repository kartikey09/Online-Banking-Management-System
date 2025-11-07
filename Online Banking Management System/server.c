#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<fcntl.h>
#include<unistd.h>
#include<crypt.h>
#include<semaphore.h>
#include<netinet/ip.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/sem.h>
#include<sys/wait.h>
#include<errno.h>
#include<signal.h>
#include<arpa/inet.h>
#include<pthread.h>

//Paths
#define EMPPATH "employees.txt"
#define CUSPATH "customers.txt"
#define LOANPATH "loanDetails.txt"
#define COUNTERPATH "loanCounter.txt"
#define HISTORYPATH "trans_hist.txt"
#define FEEDPATH "feedback.txt"

//Menu strings
#define MAINMENU "\n******** Login As ********\n1. Customer\n2. Employee\n3. Manager\n4. Admin\n5. Exit\nEnter your choice: "
#define ADMINMENU "\n******** Admin ********\n1. Add New Bank Employee\n2. Modify Customer/Employee Details\n3. Manage User Roles\n4. Logout\nEnter your choice: "
#define CUSMENU "\n******** Customer ********\n1. Deposit\n2. Withdraw\n3. View Balance\n4. Apply for a loan\n5. Money Transfer\n6. Change Password\n7. View Transaction\n8. Add Feedback\n9. Logout\n10. Exit\nEnter your choice: "
#define EMPMENU "\n******** Employee ********\n1. Add New Customer\n2. Modify Customer Details\n3. Approve/Reject Loans\n4. View Assigned Loan Applications\n5. View Customer Transactions\n6. Change Password\n7. Logout\n8. Exit\nEnter your choice: "
#define MNGMENU "\n******** Manager ********\n1. Activate/Deactivate Customer Accounts\n2. Assign Loan Application Processes to Employees\n3. Review Customer Feedback\n4. Change Password\n5. Logout\n6. Exit\nEnter your choice: "

#define PORT 8080
#define BUFFER_SIZE 4096
#define CONN_QUEUE_SIZE 5
#define HASHKEY "$6$saltsalt$"
#define MAX_ACTIVE_USERS 100


// to run: gcc server.c -o server -I/opt/homebrew/include -L/opt/homebrew/lib -lcrypt

void employeeMenu(int* connectionFD);
void managerMenu(int* connectionFD);
void adminMenu(int* connectionFD);
void *connectionHandler(void* connectionFD);


int active_employees[MAX_ACTIVE_USERS] = {0}; 
int active_customers[MAX_ACTIVE_USERS] = {0};

pthread_mutex_t employee_login_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t customer_login_mutex = PTHREAD_MUTEX_INITIALIZER;

#include "allStruct.h"
#include "Admin.h"
#include "Customer.h"
#include "Employee.h"
#include "Manager.h"


int main(){
    int socketFD, connectionFD;
    int bindStatus;
    int listenStatus;
    int clientSize;
    pthread_t thread_id;

    struct sockaddr_in server_add, client_add;

    socketFD = socket(AF_INET, SOCK_STREAM, 0); //SOCK_STREAM for TCP and SOCK_DGRAM for UDP

    if(socketFD == -1){
        perror("Something went wrong while creating the socket");
        exit(-1);
    }

    int opt = 1;
    if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(socketFD);
        return 1;
    }

    server_add.sin_addr.s_addr = htonl(INADDR_ANY); // can also not wrap in htonl since INADDR_ANY == 0
    server_add.sin_family = AF_INET;
    server_add.sin_port = htons(8080);

    if (bind(socketFD, (struct sockaddr *)&server_add, sizeof(server_add)) < 0) {
        perror("Bind failed");
        close(socketFD);
        return 1;
    }

    listen(socketFD, CONN_QUEUE_SIZE); //max 5 clients can be queued, 6th one will get a connection failed error

    printf("Waiting for connections...\n");
    clientSize = sizeof(struct sockaddr_in);

    while ((connectionFD = accept(socketFD, (struct sockaddr *)&client_add, &clientSize))) {
        printf("Connection accepted from %s:%d\n",
        inet_ntoa(client_add.sin_addr), ntohs(client_add.sin_port)); //ip and port THROUGH which the client sent conn request

        
        int *new_fd = malloc(sizeof(int));
        *new_fd = connectionFD;

        // Create a new thread to handle client
        if (pthread_create(&thread_id, NULL, connectionHandler, (void *)new_fd) < 0) {
            perror("Could not create thread");
            free(new_fd);
            close(connectionFD);
        } else {
            // Detach the thread so its resources are automatically freed on exit
            pthread_detach(thread_id);
        }
    }

    if (connectionFD < 0) {
        perror("Accept failed");
        close(socketFD);
        return 1;
    }

    close(socketFD);
    return 0;
    
}

void *connectionHandler(void* fd){
    int* connectionFD = (int*)fd;
    char readBuffer[4096], writeBuffer[4096];
    int readBytes, writeBytes, choice;

    while(1){
        writeBytes = write(*connectionFD, MAINMENU, sizeof(MAINMENU));
        if(writeBytes == -1){
            printf("Error while sending data");
        } else {
            memset(readBuffer, '\0', sizeof(readBuffer));
            readBytes = read(*connectionFD, readBuffer, sizeof(readBuffer));
            
            if(readBytes == -1)
            {
                printf("Unable to read data from client\n");
            }
            else if(readBytes == 0)
            {
                printf("No data was sent to the server\n");
            }
            else
            {
                choice = atoi(readBuffer);
                printf("Client entered: %d\n", choice);
                switch (choice) 
                {
                    case 1:
                        // Customer
                        customerMenu(*connectionFD);
                        break;
                    
                    case 2:
                        // Employee
                        employeeMenu(connectionFD);
                        break;
                            
                    case 3:
                        // Manager
                        managerMenu(connectionFD);
                        break;

                    case 4:
                        // Admin
                        adminMenu(connectionFD);
                        break;

                    case 5:
                        exitClient(*connectionFD, 0);
                        return NULL;

                    default:
                        memset(readBuffer, '\0', sizeof(readBuffer));
                        printf("Invalid Input for login menu\n");
                        write(*connectionFD, "Invalid Choice for Login menu^", sizeof("Invalid Choice for Login menu^"));
                        read(*connectionFD, readBuffer, sizeof(readBuffer));                                
                }
            }
        }
    }
    return NULL;
}
