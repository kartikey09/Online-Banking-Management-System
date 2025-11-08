void employeeMenu(int* connectionFD);
int loginEmployee(int connectionFD, int empID, char *password);
void addCustomer(int connectionFD);
void approveRejectLoan(int connectionFD, int empID);
void viewAssignedLoan(int connectionFD, int empID);
int changeEMPPassword(int connectionFD, int empID);
int accountNoExists(int accNo);


void employeeMenu(int* fd)
{
    int connectionFD = *fd;
    struct Employee employee;
    int empID;
    int accountNumber, response = 0;
    char password[40];
    char newPassword[40];
    int choice;

label1:
    memset(writeBuffer, '\0', sizeof(writeBuffer));
    strcpy(writeBuffer, "\nEnter Employee ID: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    memset(readBuffer, '\0' , sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    empID = atoi(readBuffer);
    printf("Employee entered his ID: %d\n", empID);

    memset(writeBuffer, 0, sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter password: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    memset(readBuffer, '\0' , sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    strcpy(password, readBuffer);


    if(loginEmployee(connectionFD, empID, password))
    {
        memset(writeBuffer, 0, sizeof(writeBuffer));
        memset(readBuffer, '\0' , sizeof(readBuffer));
        printf("%d logged In\n", empID);
        strcpy(writeBuffer, "\nLogin Successfully^");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));

        while(1)
        {
            memset(writeBuffer, 0, sizeof(writeBuffer));
            strcpy(writeBuffer, EMPMENU);
            write(connectionFD, writeBuffer, sizeof(writeBuffer));

            memset(readBuffer, '\0' , sizeof(readBuffer));
            read(connectionFD, readBuffer, sizeof(readBuffer));
            choice = atoi(readBuffer);

            printf("Employee choose: %d\n", choice);
            switch(choice)
            {
                case 1:
                    // Add New Customer
                    addCustomer(connectionFD);
                    break;
                case 2:
                    // Modify Customer Details
                    modifyCE(connectionFD, 1);
                    break;
                case 3:
                    // Approve/Reject Loans
                    approveRejectLoan(connectionFD, empID);
                    break;
                case 4:
                    // View Assigned Loan Applications
                    viewAssignedLoan(connectionFD, empID);
                    break;
                case 5:
                    // View Customer Transactions
                    memset(writeBuffer, 0, sizeof(writeBuffer));
                    strcpy(writeBuffer, "Enter Account Number: ");
                    write(connectionFD, writeBuffer, sizeof(writeBuffer));
                    
                    memset(readBuffer, '\0' , sizeof(readBuffer));
                    read(connectionFD, readBuffer, sizeof(readBuffer));
                    accountNumber = atoi(readBuffer);

                    transactionHistory(connectionFD, accountNumber);
                    break;
                case 6:
                    // Change Password
                    response = changeEMPPassword(connectionFD, empID);
                    if(!response)
                    {
                        memset(writeBuffer, 0, sizeof(writeBuffer));
                        strcpy(writeBuffer, "Unable to change password\n^");
                        write(connectionFD, writeBuffer, sizeof(writeBuffer));
                        read(connectionFD, readBuffer, sizeof(readBuffer));
                    }   
                    else
                    {
                        memset(writeBuffer, 0, sizeof(writeBuffer));
                        memset(readBuffer, '\0' , sizeof(readBuffer));
                        strcpy(writeBuffer,"Password changed successfully\nLogin with new password...\n^");
                        write(connectionFD, writeBuffer, sizeof(writeBuffer));
                        read(connectionFD, readBuffer, sizeof(readBuffer));
                    }   
                    logout(connectionFD, empID);                                             
                    goto label1;
                case 7:
                    // Logout
                    printf("Employee ID: %d Logged Out!\n", empID);
                    logout(connectionFD, empID);
                    return ;
                case 8:
                    // Exit
                    printf("Employee ID: %d Exited!\n", empID);
                    exitClient(connectionFD, empID);
                default:
                    memset(readBuffer, '\0' , sizeof(readBuffer));
                    printf("Invalid input for employee menu\n");
                    write(connectionFD, "Invalid Choice for Employee menu^", sizeof("Invalid Choice for Employee menu^"));
                    read(connectionFD, readBuffer, sizeof(readBuffer));                                
              
            }
        }
    }
    else
    {
        memset(writeBuffer, 0, sizeof(writeBuffer));
        memset(readBuffer, '\0' , sizeof(readBuffer));
        strcpy(writeBuffer, "\nInvalid ID or Password^");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
        goto label1;
    }
}

// ================= Login Employee ==================
int loginEmployee(int connectionFD, int empID, char *password)
{
    struct Employee employee;
    
    int file = open(EMPPATH, O_RDONLY); 
    if (file == -1) {
        perror("loginEmployee: Could not open EMPPATH");
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
        if (employee.empID == empID && strcmp(employee.password, crypt(password, HASHKEY)) == 0 && employee.role == 1) {
            
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

// ================= Add customer ==================
void addCustomer(int connectionFD) {
    struct Customer customer;
    char name[20], password[20], transactionBuffer[1024];

    struct trans_histroy th;

    time_t s, val = 1;
	struct tm* current_time;
	s = time(NULL);
	current_time = localtime(&s);

    int file = open(CUSPATH, O_RDWR | O_APPEND | O_CREAT, 0644);
    int fp = open(HISTORYPATH, O_RDWR | O_APPEND | O_CREAT, 0644);

    if (file == -1) {
        printf("Error opening file!\n");
        return;
    }

    // Customer Name
    memset(writeBuffer, 0, sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter Name: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    memset(readBuffer, '\0' , sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    strcpy(customer.customerName, readBuffer);

    // Customer Password
    memset(writeBuffer, 0, sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter Password: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    memset(readBuffer, '\0' , sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    strcpy(customer.password, crypt(readBuffer, HASHKEY));


    // Customer Account Number
    memset(writeBuffer, 0, sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter Account Number: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    memset(readBuffer, '\0' , sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    customer.accountNumber = atoi(readBuffer);

    // Checking if Account exists
    if(accountNoExists(customer.accountNumber)){
        printf("Account number %d already exists\n", customer.accountNumber);
        memset(writeBuffer, '\0', sizeof(writeBuffer));
        memset(readBuffer, '\0', sizeof(readBuffer));

        strcpy(writeBuffer, "account number unavailable^");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));   
        read(connectionFD, readBuffer, sizeof(readBuffer));
        return;
    }

    // Customer Initial Balance
    memset(writeBuffer, 0, sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter Opening Balance: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    memset(readBuffer, '\0' , sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    customer.balance = atof(readBuffer);

    bzero(transactionBuffer, sizeof(transactionBuffer));
    sprintf(transactionBuffer, "%.2f Opening Balance %02d:%02d:%02d %d-%d-%d\n", customer.balance, current_time->tm_hour, current_time->tm_min,current_time->tm_sec, (current_time->tm_year)+1900, (current_time->tm_mon)+1, current_time->tm_mday);
    
    bzero(th.hist, sizeof(th.hist));
    strcpy(th.hist, transactionBuffer);
    th.acc_no = customer.accountNumber;
    write(fp, &th, sizeof(th));

    close(fp);

    customer.activeStatus = 1;

    write(file, &customer, sizeof(customer));
    close(file);

    memset(readBuffer, '\0' , sizeof(readBuffer));
    memset(writeBuffer, '\0', sizeof(writeBuffer));
    printf("Employee added customer whose account number is: %d\n", customer.accountNumber);
    strcpy(writeBuffer, "Customer added successfully!^");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
}

// ================= Approve/Reject Loan =================
// void approveRejectLoan(int connectionFD, int empID)
// {
//     char transactionBuffer[4096];
//     struct LoanDetails ld;
    
//     struct trans_histroy th;
//     time_t s, val = 1;
// 	struct tm* current_time;
// 	s = time(NULL);
// 	current_time = localtime(&s);

//     int lID;
//     struct Customer cs;

//     int file = open(LOANPATH, O_CREAT | O_RDWR, 0644);
//     int fp = open(HISTORYPATH, O_RDWR | O_APPEND);
//     lseek(file, 0, SEEK_SET);

//     memset(writeBuffer, '\0', sizeof(writeBuffer));
//     strcpy(writeBuffer, "Enter Loan ID: ");
//     write(connectionFD, writeBuffer, sizeof(writeBuffer));

//     memset(readBuffer, '\0' , sizeof(readBuffer));
//     read(connectionFD, readBuffer, sizeof(readBuffer));
//     lID = atoi(readBuffer);

//     int srcOffset1 = -1, sourceFound1 = 0, srcOffset2 = -1, sourceFound2 = 0;
//     while (read(file, &ld, sizeof(ld)) != 0)
//     {
//         if(ld.loanID == lID)
//         {
//             srcOffset1 = lseek(file, -sizeof(struct LoanDetails), SEEK_CUR);
//             sourceFound1 = 1;
//         }
//         if(sourceFound1)
//             break;
//     }

//     if(srcOffset1 == -1) {
//         memset(writeBuffer, 0, sizeof(writeBuffer));
//         sprintf(writeBuffer, "Loan ID %d not found^", lID);
//         write(connectionFD, writeBuffer, sizeof(writeBuffer));
//         read(connectionFD, readBuffer, sizeof(readBuffer));
//         close(file);
//         return;
//     }

//     printf("Found Loan ID %d: empID=%d, status=%d, accountNo=%d\n", ld.loanID, ld.empID, ld.status, ld.accountNumber);

//     struct flock fl1 = {F_WRLCK, SEEK_SET, srcOffset1, sizeof(struct LoanDetails), getpid()};
//     int result1 = fcntl(file, F_SETLKW, &fl1);

//     int approveFlag = 0, rejectFlag = 0, devactiveFlag = 0;

//     if(result1 != -1)
//     {
//         int file2 = open(CUSPATH, O_CREAT | O_RDWR, 0644);
//         while (read(file2, &cs, sizeof(cs)) != 0)
//         {
//             if(cs.accountNumber == ld.accountNumber)
//             {
//                 srcOffset2 = lseek(file2, -sizeof(struct Customer), SEEK_CUR);
//                 sourceFound2 = 1;
//             }
//             if(sourceFound2)
//                 break;
//         }
//         struct flock fl2 = {F_WRLCK, SEEK_SET, srcOffset2, sizeof(struct Customer), getpid()};
//         int result2 = fcntl(file2, F_SETLKW, &fl2);

//         int choice;
//         memset(writeBuffer, 0, sizeof(writeBuffer));
//         strcpy(writeBuffer, "Enter 1 to Approve Loan\nEnter 2 to Reject Loan: ");
//         write(connectionFD, writeBuffer, sizeof(writeBuffer));

//         memset(readBuffer, '\0' , sizeof(readBuffer));
//         read(connectionFD, readBuffer, sizeof(readBuffer));
//         choice = atoi(readBuffer);

//         if(choice == 1)
//         {
//             if(cs.activeStatus == 0)
//             {
//                 ld.status = 3;   // rejected                         
//                 printf("%d rejected loan for account number: %d reason customer is deactive\n", empID, cs.accountNumber);
//                 write(file, &ld, sizeof(ld));
//                 devactiveFlag = 1;
//             }
//             else
//             {
//                 cs.balance += ld.loanAmount;
//                 ld.status = 2; // approved

//                 printf("%d approved loan for account number: %d\n", empID, cs.accountNumber);

//                 bzero(transactionBuffer, sizeof(transactionBuffer));
//                 sprintf(transactionBuffer, "%d credited by loan id %d at %02d:%02d:%02d %d-%d-%d\n", ld.loanAmount, lID, current_time->tm_hour, current_time->tm_min,current_time->tm_sec, (current_time->tm_year)+1900, (current_time->tm_mon)+1, current_time->tm_mday);
            
//                 bzero(th.hist, sizeof(th.hist));
//                 strcpy(th.hist, transactionBuffer);
//                 th.acc_no = cs.accountNumber;
//                 write(fp, &th, sizeof(th));

//                 write(file2, &cs, sizeof(cs));
//                 write(file, &ld, sizeof(ld));

//                 fl2.l_type = F_UNLCK;
//                 fl2.l_whence = SEEK_SET;
//                 fl2.l_start = srcOffset2;
//                 fl2.l_len = sizeof(struct Customer);
//                 fl2.l_pid = getpid();

//                 fcntl(file2, F_UNLCK, &fl2);

//                 close(file2);
//                 close(fp);
//                 approveFlag = 1;
//             }
//         }
//         else if(choice == 2)
//         {

//             printf("%d rejected loan for account number: %d\n", empID, cs.accountNumber);
//             ld.status = 3;
//             write(file, &ld, sizeof(ld));
//             rejectFlag = 1;

//             fl2.l_type = F_UNLCK;
//             fl2.l_whence = SEEK_SET;
//             fl2.l_start = srcOffset2;
//             fl2.l_len = sizeof(struct Customer);
//             fl2.l_pid = getpid();

//             fcntl(file2, F_UNLCK, &fl2);

//             close(file2);
//         }
//         else
//         {
//             printf("Invalid Choice\n");
//         }
//     }
//     else
//     {
//         memset(writeBuffer, '\0', sizeof(writeBuffer));
//         memset(readBuffer, '\0' , sizeof(readBuffer));
//         sprintf(writeBuffer, "System busy. Try again later for Loan ID %d^", lID);
//         write(connectionFD, writeBuffer, sizeof(writeBuffer));
//         read(connectionFD, readBuffer, sizeof(readBuffer)); 
//     }

//     fl1.l_type = F_UNLCK;
//     fl1.l_whence = SEEK_SET;
//     fl1.l_start = srcOffset1;
//     fl1.l_len = sizeof(struct LoanDetails);
//     fl1.l_pid = getpid();
//     fcntl(file, F_UNLCK, &fl1);

//     close(file);

//     memset(readBuffer, '\0' , sizeof(readBuffer));
//     memset(writeBuffer, 0, sizeof(writeBuffer));
//     if(approveFlag == 1)
//     {
//         strcat(writeBuffer, "Loan Approved\n");
//     }
//     else if(rejectFlag == 1)
//     {
//         strcat(writeBuffer, "Loan rejected\n");
//     }
//     else if(devactiveFlag == 1)
//     {
//         strcat(writeBuffer, "Account is already deactivate so can't approve/reject loan\n");
//     }
//     strcat(writeBuffer, "^");
//     write(connectionFD, writeBuffer, sizeof(writeBuffer));
//     read(connectionFD, readBuffer, sizeof(readBuffer));
// }

void approveRejectLoan(int connectionFD, int empID)
{
    char transactionBuffer[4096];
    struct LoanDetails ld;
    struct Customer cs;
    struct trans_histroy th;

    time_t s = time(NULL);
    struct tm* current_time = localtime(&s);

    int lID;
    int approveFlag = 0, rejectFlag = 0, deactiveFlag = 0;

    // ---------- Step 1: Ask for Loan ID ----------
    memset(writeBuffer, 0, sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter Loan ID: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    memset(readBuffer, 0, sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    lID = atoi(readBuffer);

    // ---------- Step 2: Open loan file ----------
    int file = open(LOANPATH, O_RDWR);
    if (file == -1) {
        perror("Error opening loan file");
        strcpy(writeBuffer, "System Error: Cannot open loan file^");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
        return;
    }

    // ---------- Step 3: Locate loan record ----------
    int srcOffset1 = -1;
    lseek(file, 0, SEEK_SET);
    while (read(file, &ld, sizeof(ld)) == sizeof(ld)) {
        if (ld.loanID == lID) {
            srcOffset1 = lseek(file, -sizeof(ld), SEEK_CUR);
            break;
        }
    }

    if (srcOffset1 == -1) {
        // Loan not found
        strcpy(writeBuffer, "Invalid Loan ID^");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
        close(file);
        return;
    }

    // ---------- Step 4: Lock the loan record ----------
    struct flock fl1;
    memset(&fl1, 0, sizeof(fl1));
    fl1.l_type = F_WRLCK;
    fl1.l_whence = SEEK_SET;
    fl1.l_start = srcOffset1;
    fl1.l_len = sizeof(struct LoanDetails);
    fl1.l_pid = getpid();

    if (fcntl(file, F_SETLKW, &fl1) == -1) {
        perror("fcntl lock error on loan file");
        sprintf(writeBuffer, "System busy. Try again later for Loan ID %d^", lID);
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
        close(file);
        return;
    }

    // ---------- Step 5: Re-read locked record ----------
    lseek(file, srcOffset1, SEEK_SET);
    read(file, &ld, sizeof(ld));

    // If loan is already processed
    if (ld.status == 2 || ld.status == 3) {
        sprintf(writeBuffer, "Loan ID %d is already approved or rejected^", lID);
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));

        // Unlock & exit
        fl1.l_type = F_UNLCK;
        fcntl(file, F_SETLK, &fl1);
        close(file);
        return;
    }

    // ---------- Step 6: Lock corresponding customer record ----------
    int file2 = open(CUSPATH, O_RDWR);
    if (file2 == -1) {
        perror("Error opening customer file");
        strcpy(writeBuffer, "System Error: Cannot open customer file^");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
        fl1.l_type = F_UNLCK;
        fcntl(file, F_SETLK, &fl1);
        close(file);
        return;
    }

    int srcOffset2 = -1;
    lseek(file2, 0, SEEK_SET);
    while (read(file2, &cs, sizeof(cs)) == sizeof(cs)) {
        if (cs.accountNumber == ld.accountNumber) {
            srcOffset2 = lseek(file2, -sizeof(cs), SEEK_CUR);
            break;
        }
    }

    if (srcOffset2 == -1) {
        strcpy(writeBuffer, "Customer not found for this loan^");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
        // Unlock loan and close files
        fl1.l_type = F_UNLCK;
        fcntl(file, F_SETLK, &fl1);
        close(file);
        close(file2);
        return;
    }

    struct flock fl2;
    memset(&fl2, 0, sizeof(fl2));
    fl2.l_type = F_WRLCK;
    fl2.l_whence = SEEK_SET;
    fl2.l_start = srcOffset2;
    fl2.l_len = sizeof(struct Customer);
    fl2.l_pid = getpid();

    if (fcntl(file2, F_SETLKW, &fl2) == -1) {
        perror("fcntl lock error on customer file");
        sprintf(writeBuffer, "System busy. Try again later for account %d^", cs.accountNumber);
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
        // Unlock loan
        fl1.l_type = F_UNLCK;
        fcntl(file, F_SETLK, &fl1);
        close(file);
        close(file2);
        return;
    }

    memset(writeBuffer, 0, sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter 1 to Approve Loan\nEnter 2 to Reject Loan: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));
    memset(readBuffer, 0, sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    int choice = atoi(readBuffer);

    int fp = open(HISTORYPATH, O_RDWR | O_APPEND | O_CREAT, 0644);


    if (choice == 1) {
        if (cs.activeStatus == 0) {
            ld.status = 3; // rejected
            deactiveFlag = 1;
            printf("%d rejected loan %d (inactive customer)\n", empID, ld.loanID);
        } else {
            cs.balance += ld.loanAmount;
            ld.status = 2; // approved
            printf("%d approved loan ID %d for account %d\n", empID, ld.loanID, cs.accountNumber);


            bzero(transactionBuffer, sizeof(transactionBuffer));
            sprintf(transactionBuffer,
                    "%d credited by loan id %d at %02d:%02d:%02d %d-%d-%d\n",
                    ld.loanAmount, lID,
                    current_time->tm_hour, current_time->tm_min, current_time->tm_sec,
                    (current_time->tm_year) + 1900, (current_time->tm_mon) + 1, current_time->tm_mday);

            bzero(th.hist, sizeof(th.hist));
            strcpy(th.hist, transactionBuffer);
            th.acc_no = cs.accountNumber;
            write(fp, &th, sizeof(th));

            approveFlag = 1;
        }
    } else if (choice == 2) {
        ld.status = 3; // rejected
        rejectFlag = 1;
        printf("%d rejected loan ID %d\n", empID, ld.loanID);
    } else {
        printf("Invalid choice entered for loan approval menu\n");
    }


    if (approveFlag || rejectFlag || deactiveFlag) {
        lseek(file, srcOffset1, SEEK_SET);
        write(file, &ld, sizeof(ld));

        lseek(file2, srcOffset2, SEEK_SET);
        write(file2, &cs, sizeof(cs));
    }


    fl2.l_type = F_UNLCK;
    fcntl(file2, F_SETLK, &fl2);
    fl1.l_type = F_UNLCK;
    fcntl(file, F_SETLK, &fl1);


    close(fp);
    close(file);
    close(file2);


    memset(writeBuffer, 0, sizeof(writeBuffer));
    if (approveFlag)
        strcpy(writeBuffer, "Loan Approved\n^");
    else if (rejectFlag)
        strcpy(writeBuffer, "Loan Rejected\n^");
    else if (deactiveFlag)
        strcpy(writeBuffer, "Account is deactivated, loan rejected automatically\n^");
    else
        strcpy(writeBuffer, "No action taken\n^");

    write(connectionFD, writeBuffer, sizeof(writeBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
}

// ================= View Assigned Loan =================
void viewAssignedLoan(int connectionFD, int empID)
{
    struct LoanDetails ld;
    int file = open(LOANPATH, O_RDONLY);
    if(file == -1)
    {
        printf("Error in opening file\n");
        return;
    }

    int flag = 1;
    while(read(file, &ld, sizeof(ld)) != 0)
    {

        if(ld.empID == empID && ld.status == 1)
        {
            flag = 0;
            memset(readBuffer, '\0' , sizeof(readBuffer));
            memset(writeBuffer, '\0', sizeof(writeBuffer));
            sprintf(writeBuffer, "Loan ID: %d\nAccount Number: %d\nLoan Amount: %d^", ld.loanID, ld.accountNumber, ld.loanAmount);
            write(connectionFD, writeBuffer, sizeof(writeBuffer));
            read(connectionFD, readBuffer, sizeof(readBuffer));
        }
    }
    if(flag){
        memset(readBuffer, '\0' , sizeof(readBuffer));
        memset(writeBuffer, '\0', sizeof(writeBuffer));
        sprintf(writeBuffer, "\nNo Assigned loans\n^");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
    }
    close(file);
    
    memset(readBuffer, '\0' , sizeof(readBuffer));
    memset(writeBuffer, '\0', sizeof(writeBuffer));
    strcpy(writeBuffer, "^");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
}

// ================= Change Password =================
int changeEMPPassword(int connectionFD, int empID)
{
    char newPassword[20];

    struct Employee emp;
    int file = open(EMPPATH, O_CREAT | O_RDWR, 0644);
    
    lseek(file, 0, SEEK_SET);

    int srcOffset = -1, sourceFound = 0;

    while (read(file, &emp, sizeof(emp)) != 0)
    {
        if(emp.empID == empID)
        {
            srcOffset = lseek(file, -sizeof(struct Employee), SEEK_CUR);
            sourceFound = 1;
        }
        if(sourceFound)
            break;
    }

    struct flock fl1 = {F_WRLCK, SEEK_SET, srcOffset, sizeof(struct Employee), getpid()};
    fcntl(file, F_SETLKW, &fl1);

    memset(writeBuffer, 0, sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter password: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));
    
    memset(readBuffer, '\0' , sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    strcpy(newPassword, readBuffer);

    strcpy(emp.password, crypt(newPassword, HASHKEY));
    write(file, &emp, sizeof(emp));

    fl1.l_type = F_UNLCK;
    fl1.l_whence = SEEK_SET;
    fl1.l_start = srcOffset;
    fl1.l_len = sizeof(struct Employee);
    fl1.l_pid = getpid();

    fcntl(file, F_UNLCK, &fl1);
    close(file);

    printf("Employee %d changed password\n", empID);
    return 1;
}

// ================= Check if Account Exists =================
int accountNoExists(int accNo){
    int fd = open(CUSPATH, O_RDONLY, 0644);
    struct Customer temp;
    while(read(fd, &temp, sizeof(struct Customer)) != 0){
        if(temp.accountNumber == accNo)
            return 1;
    }

    close(fd);
    return 0;
}
