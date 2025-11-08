#define ADMINNAME "admin"
#define PASSWORD "admin"

int addEmployee(int connectionFD);
void modifyCE(int connectionFD, int modifyChoice);
void manageRole(int connectionFD);
int employeeAlreadyExists(int id, int fd);
void adminMenu(int* fd);

char readBuffer[4096], writeBuffer[4096];

void adminMenu(int* fd)
{
    int connectionFD = *fd;
    char password[20];
    memset(writeBuffer, '\0', sizeof(writeBuffer));
    memset(readBuffer, '\0', sizeof(readBuffer));
    strcat(writeBuffer, "Enter password: ");

    write(connectionFD, writeBuffer, sizeof(writeBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    strcpy(password, readBuffer);

    if(strcmp(PASSWORD, password) == 0)
    {
        memset(writeBuffer, '\0', sizeof(writeBuffer));
        memset(readBuffer, '\0', sizeof(readBuffer));
        strcpy(writeBuffer, "\nLogin Successfully^");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
    }
    else
    {
        memset(writeBuffer, '\0', sizeof(writeBuffer));
        memset(readBuffer, '\0', sizeof(readBuffer));
        strcpy(writeBuffer, "\nInvalid credential^");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
        return adminMenu(fd);
    }

    while(1)
    {
        int modifyChoice;
        int choice;

        memset(writeBuffer, '\0', sizeof(writeBuffer));
        memset(readBuffer, '\0', sizeof(readBuffer));

        strcpy(writeBuffer, ADMINMENU);
        write(connectionFD, writeBuffer, sizeof(writeBuffer));

        read(connectionFD, readBuffer, sizeof(readBuffer));
        choice = atoi(readBuffer);
        printf("Admin entered: %d\n", choice);

        switch (choice) {
            case 1:
                // Add new bank employee
                if(addEmployee(connectionFD)){
                    memset(writeBuffer, '\0', sizeof(writeBuffer));
                    memset(readBuffer, '\0', sizeof(readBuffer));
                    strcpy(writeBuffer, "Employee successfully added\n^");
                    write(connectionFD, writeBuffer, sizeof(writeBuffer));
                    read(connectionFD, readBuffer, sizeof(readBuffer));
                }
                break;
            case 2:
                // Modify Customer/Employee details
                memset(writeBuffer, '\0', sizeof(writeBuffer));
                strcpy(writeBuffer, "********Enter your choice ********\n1. Modify Customer\n2. Modify Employee \n");
                write(connectionFD, writeBuffer, sizeof(writeBuffer));
                
                memset(readBuffer, '\0', sizeof(readBuffer));
                read(connectionFD, readBuffer, sizeof(readBuffer));
                modifyChoice = atoi(readBuffer);

                modifyCE(connectionFD, modifyChoice);
                break;
            case 3:
                // Manage User Roles
                manageRole(connectionFD);
                break;
            case 4:
                // Logout
                return;
            default:
                memset(writeBuffer, '\0', sizeof(writeBuffer));
                memset(readBuffer, '\0', sizeof(readBuffer));
                strcpy(writeBuffer, "Invalid choice! Please try again.^");
                write(connectionFD, writeBuffer, sizeof(writeBuffer));
                read(connectionFD, readBuffer, sizeof(readBuffer));
        }
    }    
}

// ================ Add New Bank Employee ================
int addEmployee(int connectionFD)
{
    struct Employee emp;

    // Employee ID
    memset(writeBuffer, '\0', sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter Employee ID: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    memset(readBuffer, '\0', sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    emp.empID = atoi(readBuffer);
    printf("Admin entered empID: %d\n", emp.empID);

    // Employee FirstName
    memset(writeBuffer, '\0', sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter FirstName: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    memset(readBuffer, '\0', sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    strcpy(emp.firstName, readBuffer);
    printf("Admin entered firstName: %s\n", emp.firstName);

    // Employee LastName
    memset(writeBuffer, '\0', sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter LastName: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    memset(readBuffer, '\0', sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    strcpy(emp.lastName, readBuffer);
    printf("Admin entered lastName: %s\n", emp.lastName);

    // Employee Password
    memset(writeBuffer, '\0', sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter Password: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    struct crypt_data data;
    data.initialized = 0;

    memset(readBuffer, '\0', sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    strcpy(emp.password, crypt_r(readBuffer, HASHKEY, &data));

    emp.role = 1;

    int file = open(EMPPATH, O_CREAT | O_RDWR, 0644);
    if(file == -1)
    {
        printf("Error opening file!\n");
        return 0;
    }

    if(employeeAlreadyExists(emp.empID, file)){
        memset(writeBuffer, '\0', sizeof(writeBuffer));
        memset(readBuffer, '\0', sizeof(readBuffer));

        snprintf(writeBuffer, sizeof(writeBuffer), "Employee ID - %d is taken\n^", emp.empID);
        write(connectionFD, writeBuffer, sizeof(writeBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
        return 0;
    }
    

    lseek(file, 0, SEEK_END);
    int returnValue = write(file, &emp, sizeof(emp));
    if(returnValue == -1){
        printf("Error in adding employee!\n");
        return 0;
    }
    
    close(file);
    return 1;
}

// ===================== See if Employee exists ==================
int employeeAlreadyExists(int id, int fd){
    struct Employee emp;
    lseek(fd, 0, SEEK_SET);
    while(read(fd, &emp, sizeof(emp)) != 0){
        if(emp.empID == id)
            return 1;
    }

    return 0;
}

// ===================== Modify Customer / Employee ==================
void modifyCE(int connectionFD, int modifyChoice)
{
    // 1. Modify Customer
    // 2. Modify Employee
    if(modifyChoice == 1)
    {
        printf("Admin chose 1\n");
        int file = open(CUSPATH, O_CREAT | O_RDWR , 0644);
        if(file == -1)
        {
            printf("Error opening file!\n");
            return ;
        }

        memset(writeBuffer, '\0', sizeof(writeBuffer));
        strcpy(writeBuffer, "Enter Account Number: ");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));

        int accNo;
        memset(readBuffer, '\0', sizeof(readBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
        accNo = atoi(readBuffer);
        printf("Admin entered account number: %d\n", accNo);


        struct Customer c;    
        lseek(file, 0, SEEK_SET);

        int srcOffset = -1, sourceFound = 0;

        while (read(file, &c, sizeof(c)) != 0)
        {
            if(c.accountNumber == accNo)
            {
                srcOffset = lseek(file, -sizeof(struct Customer), SEEK_CUR);
                sourceFound = 1;
            }
            if(sourceFound)
                break;
        }

        struct flock fl1 = {F_WRLCK, SEEK_SET, srcOffset, sizeof(struct Customer), getpid()};
        fcntl(file, F_SETLKW, &fl1);

        if(sourceFound)
        {
            char newName[20];
            memset(writeBuffer, '\0', sizeof(writeBuffer));
            strcpy(writeBuffer, "Enter New Name: ");
            write(connectionFD, writeBuffer, sizeof(writeBuffer));

            memset(readBuffer, '\0', sizeof(readBuffer));
            read(connectionFD, readBuffer, sizeof(readBuffer));
            strcpy(newName, readBuffer);
            printf("Admin entered new Name: %s\n", newName);

            strcpy(c.customerName, newName);

            write(file, &c, sizeof(c));

            memset(writeBuffer, '\0', sizeof(writeBuffer));
            strcpy(writeBuffer, "^");
            write(connectionFD, writeBuffer, sizeof(writeBuffer));

            memset(readBuffer, '\0', sizeof(readBuffer));
            read(connectionFD, readBuffer, sizeof(readBuffer));
        }
        else
        {
            memset(writeBuffer, '\0', sizeof(writeBuffer));
            strcpy(writeBuffer, "Invalid Account Number\n");
            write(connectionFD, writeBuffer, sizeof(writeBuffer));

            memset(readBuffer, '\0', sizeof(readBuffer));
            read(connectionFD, readBuffer, sizeof(readBuffer));

        }
        fl1.l_type = F_UNLCK;
        fl1.l_whence = SEEK_SET;
        fl1.l_start = srcOffset;
        fl1.l_len = sizeof(struct Customer);
        fl1.l_pid = getpid();

        fcntl(file, F_UNLCK, &fl1);
        close(file);
        return ;
    }

    else if(modifyChoice == 2)
    {
        printf("Admin chose 2\n");
        struct Employee emp;
        int file = open(EMPPATH, O_CREAT | O_RDWR, 0644);
        if(file == -1)
        {
            printf("Error opening file!\n");
            return ;
        }

        memset(writeBuffer, '\0', sizeof(writeBuffer));
        strcpy(writeBuffer, "Enter Employee ID: ");
        write(connectionFD, writeBuffer, sizeof(writeBuffer));

        int id;
        memset(readBuffer, '\0', sizeof(readBuffer));
        read(connectionFD, readBuffer, sizeof(readBuffer));
        id = atoi(readBuffer);
        printf("Admin entered Employee ID: %d\n", id);

        lseek(file, 0, SEEK_SET);

        int srcOffset = -1, sourceFound = 0;

        while (read(file, &emp, sizeof(emp)) != 0)
        {
            if(emp.empID == id)
            {
                srcOffset = lseek(file, -sizeof(struct Employee), SEEK_CUR);
                sourceFound = 1;
            }
            if(sourceFound)
                break;
        }

        struct flock fl1 = {F_WRLCK, SEEK_SET, srcOffset, sizeof(struct Employee), getpid()};
        fcntl(file, F_SETLKW, &fl1);

        if(sourceFound){
            char newName[20];
            memset(writeBuffer, '\0', sizeof(writeBuffer));
            strcpy(writeBuffer, "Enter New Name: ");
            write(connectionFD, writeBuffer, sizeof(writeBuffer));

            memset(readBuffer, '\0', sizeof(readBuffer));
            read(connectionFD, readBuffer, sizeof(readBuffer));
            strcpy(newName, readBuffer);

            strcpy(emp.firstName, newName);
            printf("Admin entered new Name: %s\n", newName);
            printf("Admin changed name of employee: %d\n", id);

            write(file, &emp, sizeof(emp));
        } else {
            memset(writeBuffer, '\0', sizeof(writeBuffer));
            strcpy(writeBuffer, "Invalid Employee ID");
            write(connectionFD, writeBuffer, sizeof(writeBuffer));

            memset(readBuffer, '\0', sizeof(readBuffer));
            read(connectionFD, readBuffer, sizeof(readBuffer));        
        }
        
        fl1.l_type = F_UNLCK;
        fl1.l_whence = SEEK_SET;
        fl1.l_start = srcOffset;
        fl1.l_len = sizeof(struct Employee);
        fl1.l_pid = getpid();

        fcntl(file, F_SETLKW, &fl1);
        close(file);

        return ;   

    }
}

// ===================== Manager Role ==================
void manageRole(int connectionFD)
{
    int file = open(EMPPATH, O_CREAT | O_RDWR, 0644);
    if(file == -1)
    {
        printf("Error opening file!\n");
        return ;
    }

    memset(writeBuffer, '\0', sizeof(writeBuffer));
    strcpy(writeBuffer, "Enter ID: ");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    int id;
    memset(readBuffer, '\0', sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    id = atoi(readBuffer);

 
    struct Employee emp;
    while(read(file, &emp, sizeof(emp)) != 0)
    {
        if(emp.empID == id)
        {
            int choice;
            memset(writeBuffer, '\0', sizeof(writeBuffer));
            strcpy(writeBuffer, "Enter 1 to make manager\nEnter 2 to make employee: ");
            write(connectionFD, writeBuffer, sizeof(writeBuffer));

            memset(readBuffer, '\0', sizeof(readBuffer));
            read(connectionFD, readBuffer, sizeof(readBuffer));
            choice = atoi(readBuffer);

            lseek(file,-sizeof(struct Employee), SEEK_CUR);
            
            if(choice == 1)
            {
                printf("Admin made %d manager\n", id);
                emp.role = 0;            
                write(file, &emp, sizeof(emp));
            }
            else if(choice == 2)
            {
                printf("Admin made %d employee\n", id);
                emp.role = 1;
                write(file, &emp, sizeof(emp));
            }
            close(file);
            
            memset(writeBuffer, '\0', sizeof(writeBuffer));
            strcpy(writeBuffer, "^");
            write(connectionFD, writeBuffer, sizeof(writeBuffer));

            memset(readBuffer, '\0', sizeof(readBuffer));
            read(connectionFD, readBuffer, sizeof(readBuffer));
            return ;
        }
    }
    memset(writeBuffer, '\0', sizeof(writeBuffer));
    strcpy(writeBuffer, "Invalid ID");
    write(connectionFD, writeBuffer, sizeof(writeBuffer));

    memset(readBuffer, '\0', sizeof(readBuffer));
    read(connectionFD, readBuffer, sizeof(readBuffer));
    return ;
}


