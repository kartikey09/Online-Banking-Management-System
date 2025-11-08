void managerMenu(int* connectionFD);
int loginManager(int connectionFD, int mngID, char *password);
void changeStatus(int connectionFD);
void assignLoanApplication(int connectionFD);
void readFeedBack(int connectionFD);
int changeMNGPassword(int connectionFD, int mngID);

char readBuffer[4096], writeBuffer[4096];   

void managerMenu(int* fd)
{
    int connectionFD = *fd;
    struct Employee manager;
    int mngID, response = 0;
    char password[20];
    char newPassword[20];
    int choice;

label1:
    memset(writeBuffer, '\0', sizeof(writeBuffer));
    strcpy(writeBuffer, "\nEnter Manager ID: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    memset(readBuffer, '\0', sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    mngID = atoi(readBuffer);

    memset(writeBuffer, '\0', sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter password: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    memset(readBuffer, '\0', sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    strcpy(password, readBuffer);


    if(loginManager(connectionFD, mngID, password)){
        memset(writeBuffer, '\0', sizeof(writeBuffer));
        memset(readBuffer, '\0', sizeof(readBuffer));
        printf("Manager with ID: %d logged In\n", mngID);
        strcpy(writeBuffer, "Login Successfully\n^");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));

        while(1){
            memset(writeBuffer, '\0', sizeof(writeBuffer));
            strcpy(writeBuffer, MNGMENU);
            write(connectionFD, writeBuffer, sizeof(writeBuffer));

            memset(readBuffer, '\0', sizeof(readBuffer));
            read(connectionFD, readBuffer, sizeof(readBuffer));
            choice = atoi(readBuffer);

            printf("Manager entered: %d\n", choice);
            switch(choice)
            {
                case 1:
                    // Activate/Deactivate Customer Accounts
                    changeStatus(connectionFD);
                    break;
                case 2:
                    // Assign Loan Application Processes to Employees
                    assignLoanApplication(connectionFD);
                    break;
                case 3:
                    // Review Customer Feedback
                    readFeedBack(connectionFD);
                    break;
                case 4:
                    // Change Password
                    response = changeMNGPassword(connectionFD, mngID);
                    if(!response)
                    {
                        memset(writeBuffer, '\0', sizeof(writeBuffer));
                        strcpy(writeBuffer, "Unable to change password\n^");
                        write(connectionFD, writeBuffer, sizeof(writeBuffer));
                        read(connectionFD, readBuffer, sizeof(readBuffer));
                    }   
                    else
                    {
                        memset(writeBuffer, '\0', sizeof(writeBuffer));
                        memset(readBuffer, '\0', sizeof(readBuffer));
                        strcpy(writeBuffer,"Password changed successfully\nLogin with new password...\n^");
                        write(connectionFD, writeBuffer, sizeof(writeBuffer));
                        read(connectionFD, readBuffer, sizeof(readBuffer));   
                        logout(connectionFD, mngID);                                                                
                        goto label1;
                    }
                case 5:
                    // Logout
                    printf("Manager Logged out!\n");
                    logout(connectionFD, mngID);
                    return ;
                case 6:
                    // Exit
                    printf("Manager exited\n");
                    exitClient(connectionFD, mngID);
                    return;
                default:
                    memset(readBuffer, '\0', sizeof(readBuffer));
                    printf("Invalid Input for manager menu\n");
                    write(connectionFD, "Invalid Choice for Manager menu^", sizeof("Invalid Choice for Manager menu^"));
                    read(connectionFD, readBuffer, sizeof(readBuffer));                                
            }
        }
    }
    else
    {
        memset(writeBuffer, '\0', sizeof(writeBuffer));
        memset(readBuffer, '\0', sizeof(readBuffer));
        strcpy(writeBuffer, "\nInvalid ID or Password^");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
        goto label1;
    }
}

// ==================== Login Manager ======================
int loginManager(int connectionFD, int empID, char *password)
{
struct Employee employee;
    
    int file = open(EMPPATH, O_RDONLY); 
    if (file == -1) {
        perror("Could not open EMPPATH");
        return 0;
    }

    if (isEmployeeActive(empID)) {
        printf("Employee %d is already logged in!\n", empID);
        close(file);
        return 0; 
    }

    password[strcspn(password, "\r\n")] = '\0';

    
    while(read(file, &employee, sizeof(struct Employee)) == sizeof(struct Employee))
    {
        if (employee.empID == empID && strcmp(employee.password, crypt(password, HASHKEY)) == 0 && employee.role == 0) {
            
            
            if (addActiveEmployee(empID)) {
                close(file);
                printf("Emp %d logged in\n", empID);
                return 1; 
            } else {
                printf("Error: Login table full for %d\n", empID);
                close(file);
                return 0; 
            }
        }
    }
    
    printf("Employee with the id: %d not found or password incorrect\n", empID);
    close(file);
    return 0; 
}

// ==================== Activate/Deactivate Customer Accounts ====================
void changeStatus(int connectionFD)
{
    int file = open(CUSPATH, O_CREAT | O_RDWR, 0644);
    struct Customer cs;

    int accNo, choice;

    memset(writeBuffer, '\0', sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter Account Number: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    memset(readBuffer, '\0', sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    accNo = atoi(readBuffer);

    while(read(file, &cs, sizeof(cs)) != 0)
    {
        if(cs.accountNumber == accNo)
        {
            memset(writeBuffer, '\0', sizeof(writeBuffer));
            strcpy(writeBuffer, "Enter 1 to Deactivate\nEnter 2 to activate\nEnter your choice: ");
            write(connectionFD, writeBuffer, sizeof(writeBuffer));

            memset(readBuffer, '\0', sizeof(readBuffer));
            read(connectionFD, readBuffer, sizeof(readBuffer));
            choice = atoi(readBuffer);

            if(choice == 1)
            {
                lseek(file,-sizeof(struct Customer), SEEK_CUR);
                cs.activeStatus = 0;
                write(file, &cs, sizeof(cs));
                close(file);

                printf("Manager activated customer %d\n", accNo);

                memset(readBuffer, '\0', sizeof(readBuffer));
                memset(writeBuffer, '\0', sizeof(writeBuffer));
                strcpy(writeBuffer, "Status Changed Successfully^");
                write(connectionFD, writeBuffer, sizeof(writeBuffer));
                read(connectionFD, readBuffer, sizeof(readBuffer));
                return ;
            }
            else if(choice == 2)
            {
                lseek(file, -sizeof(struct Customer), SEEK_CUR);
                cs.activeStatus = 1;
                write(file, &cs, sizeof(cs));
                close(file);

                printf("Manager deactivated customer %d\n", accNo);

                memset(readBuffer, '\0', sizeof(readBuffer));
                memset(writeBuffer, '\0', sizeof(writeBuffer));
                strcpy(writeBuffer, "Status Changed Successfully^");
                write(connectionFD, writeBuffer, sizeof(writeBuffer));
                read(connectionFD, readBuffer, sizeof(readBuffer));
                return ;
            }
        }
    }
    close(file);
    memset(readBuffer, '\0', sizeof(readBuffer));
    memset(writeBuffer, '\0', sizeof(writeBuffer));
    printf("Manager entered invalid account number: %d\n", accNo);
    strcpy(writeBuffer, "Invalid account number^");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    return ;
}

// ==================== Read Feedback =========================
void readFeedBack(int connectionFD)
{
    struct FeedBack fb;
    char tempBuffer[4096];

    int file = open(FEEDPATH, O_RDONLY);
    if(file == -1)
    {
        printf("Error in opening file\n");
    }

    bzero(tempBuffer, sizeof(tempBuffer));
    while(read(file, &fb, sizeof(fb)) != 0)
    {
        strcat(tempBuffer, fb.feedback);
        strcat(tempBuffer, "\n");
    }
    strcat(tempBuffer, "^");
    printf("Manager reading customers feedback\n");
    memset(writeBuffer, '\0', sizeof(writeBuffer));
    memset(readBuffer, '\0', sizeof(readBuffer));
    strcpy(writeBuffer, tempBuffer);
    write(connectionFD, writeBuffer, sizeof(writeBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
}

// ==================== Assign Loan Application Processes to Employees ====================
// void assignLoanApplication(int connectionFD)
// {
//     struct LoanDetails ld;
//     int file = open(LOANPATH, O_CREAT | O_RDWR, 0644);
//     if(file == -1)
//     {
//         printf("Error in opening file\n");
//         return ;
//     }

//     lseek(file, 0, SEEK_SET); 
//     while(read(file, &ld, sizeof(ld)) == sizeof(ld))
//     {
//         if(ld.empID == -1)
//         {
//             memset(writeBuffer, '\0', sizeof(writeBuffer));
//             sprintf(writeBuffer,
//                 "Loan ID: %d\nAccount Number: %d\nLoan Amount: %d^",
//                 ld.loanID, ld.accountNumber, ld.loanAmount);
//             write(connectionFD, writeBuffer, sizeof(writeBuffer));
//             read(connectionFD, readBuffer, sizeof(readBuffer));
//         }
//     }


//     lseek(file, 0, SEEK_SET); 
//     int lID, eID;
//     memset(writeBuffer, '\0', sizeof(writeBuffer));
//     strcpy(writeBuffer, "Enter Loan ID: ");
//     write(connectionFD, writeBuffer, sizeof(writeBuffer));
    
//     memset(readBuffer, '\0', sizeof(readBuffer));
//     read(connectionFD, readBuffer, sizeof(readBuffer));
//     lID = atoi(readBuffer);


//     int srcOffset = -1;
//     lseek(file, 0, SEEK_SET);
//     while(read(file, &ld, sizeof(ld)) == sizeof(ld))
//     {
//         if(ld.loanID == lID)
//         {
//             srcOffset = lseek(file, -sizeof(struct LoanDetails), SEEK_CUR);
//             break;
//         }
//     }

//     if(srcOffset < 0)
//     {
//         sprintf(writeBuffer, "Invalid loan ID^");
//         write(connectionFD, writeBuffer, sizeof(writeBuffer));
//         read(connectionFD, readBuffer, sizeof(readBuffer));
//         close(file);
//         return;
//     }


//     struct flock fl1;
//     memset(&fl1, 0, sizeof(fl1));
//     fl1.l_type = F_WRLCK;
//     fl1.l_whence = SEEK_SET;
//     fl1.l_start = srcOffset;
//     fl1.l_len = sizeof(struct LoanDetails);
//     fl1.l_pid = getpid();

//     if (fcntl(file, F_SETLKW, &fl1) == -1)
//     {
//         perror("Error locking record");
//         close(file);
//         return;
//     }


//     lseek(file, srcOffset, SEEK_SET);
//     read(file, &ld, sizeof(ld));


//     if(ld.empID != -1)
//     {
//         sprintf(writeBuffer, "Employee already assigned for loan ID %d^", lID);
//         write(connectionFD, writeBuffer, sizeof(writeBuffer));
//         read(connectionFD, readBuffer, sizeof(readBuffer));
//     }
//     else
//     {
//         memset(writeBuffer, '\0', sizeof(writeBuffer));
//         strcpy(writeBuffer, "Enter Employee ID: ");
//         write(connectionFD, writeBuffer, sizeof(writeBuffer));

//         memset(readBuffer, '\0', sizeof(readBuffer));
//         read(connectionFD, readBuffer, sizeof(readBuffer));
//         int eID = atoi(readBuffer);

//         ld.empID = eID;
//         ld.status = 1; // Pending
//         lseek(file, srcOffset, SEEK_SET);
//         write(file, &ld, sizeof(ld));

//         printf("Manager assigned employee %d to loan %d\n", eID, lID);
//         sprintf(writeBuffer, "Loan ID %d assigned to Employee %d successfully^", lID, eID);
//         write(connectionFD, writeBuffer, sizeof(writeBuffer));
//         read(connectionFD, readBuffer, sizeof(readBuffer));
//     }

//     fl1.l_type = F_UNLCK;
//     fcntl(file, F_SETLK, &fl1);
//     close(file);
// }

void assignLoanApplication(int connectionFD)
{
    struct LoanDetails ld;
    int file = open(LOANPATH, O_RDWR | O_CREAT, 0644);
    if (file == -1) {
        perror("Error opening loan file");
        return;
    }

    lseek(file, 0, SEEK_SET);
    int hasUnassigned = 0;

    while (read(file, &ld, sizeof(ld)) == sizeof(ld)) {
        if (ld.empID == -1) {
            hasUnassigned = 1;
            memset(writeBuffer, '\0', sizeof(writeBuffer));
            sprintf(writeBuffer,
                "Loan ID: %d\nAccount Number: %d\nLoan Amount: %d\nStatus: %d^",
                ld.loanID, ld.accountNumber, ld.loanAmount, ld.status);
            write(connectionFD, writeBuffer, sizeof(writeBuffer));
            read(connectionFD, readBuffer, sizeof(readBuffer));
        }
    }

    if (!hasUnassigned) {
        sprintf(writeBuffer, "No unassigned loan applications found.^");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
        close(file);
        return;
    }

    memset(writeBuffer, '\0', sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter Loan ID to assign: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));
    
    memset(readBuffer, '\0', sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    int loanID = atoi(readBuffer);

    int offset = -1;
    lseek(file, 0, SEEK_SET);
    while (read(file, &ld, sizeof(ld)) == sizeof(ld)) {
        if (ld.loanID == loanID) {
            offset = lseek(file, -sizeof(ld), SEEK_CUR);
            break;
        }
    }

    if (offset == -1) {
        sprintf(writeBuffer, "Invalid Loan ID.^");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
        close(file);
        return;
    }

    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = offset;
    lock.l_len = sizeof(struct LoanDetails);
    lock.l_pid = getpid();

    if (fcntl(file, F_SETLKW, &lock) == -1) {
        perror("Lock failed");
        close(file);
        return;
    }

    lseek(file, offset, SEEK_SET);
    read(file, &ld, sizeof(ld));

    if (ld.empID != -1) {
        sprintf(writeBuffer, "Loan ID %d is already assigned to Employee %d.^", ld.loanID, ld.empID);
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
    } else {
        memset(writeBuffer, '\0', sizeof(writeBuffer));
        strcpy(writeBuffer, "Enter Employee ID to assign: ");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));

        memset(readBuffer, '\0', sizeof(readBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
        int empID = atoi(readBuffer);

        ld.empID = empID;
        ld.status = 1; // Pending

        lseek(file, offset, SEEK_SET);
        write(file, &ld, sizeof(ld));

        printf("Assigned Employee %d to Loan %d successfully.\n", empID, ld.loanID);
        sprintf(writeBuffer, "Loan ID %d assigned to Employee %d successfully.^", ld.loanID, empID);
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
    }

    lock.l_type = F_UNLCK;
    fcntl(file, F_SETLK, &lock);
    close(file);
}

// ==================== Change Password ====================
int changeMNGPassword(int connectionFD, int mngID)
{
    char newPassword[20];

    struct Employee m;
    int file = open(EMPPATH, O_CREAT | O_RDWR, 0644);
    
    lseek(file, 0, SEEK_SET);

    int srcOffset = -1, sourceFound = 0;

    while (read(file, &m, sizeof(m)) != 0)
    {
        if(m.empID == mngID)
        {
            srcOffset = lseek(file, -sizeof(struct Employee), SEEK_CUR);
            sourceFound = 1;
        }
        if(sourceFound)
            break;
    }

    struct flock fl1 = {F_WRLCK, SEEK_SET, srcOffset, sizeof(struct Employee), getpid()};
    fcntl(file, F_SETLKW, &fl1);

    memset(writeBuffer, '\0', sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter password: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));
    
    memset(readBuffer, '\0', sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    strcpy(newPassword, readBuffer);

    strcpy(m.password, crypt(newPassword, HASHKEY));
    write(file, &m, sizeof(m));

    fl1.l_type = F_UNLCK;
    fl1.l_whence = SEEK_SET;
    fl1.l_start = srcOffset;
    fl1.l_len = sizeof(struct Employee);
    fl1.l_pid = getpid();

    fcntl(file, F_UNLCK, &fl1);
    close(file);

    printf("Manager %d changed password\n", mngID);
    return 1;

}