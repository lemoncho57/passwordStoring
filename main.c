#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#ifdef __linux__
#include <unistd.h>
#include <termios.h>
#endif

#define PASSWORD_ARRAY_LENGHT 50

char mainPass[100];
bool running = true;
#ifdef __linux__
struct termios oldTermios;
#endif

void clearInputBuffer();

char *encryptString(char *string)
{
    while (*string != '\0')
    {
        *string += 103;
        ++string;
    }
    return string;
}

char *decryptString(char *string)
{
    while (*string != '\0')
    {
        *string -= 103;
        ++string;
    }
    return string;
}

#ifdef __linux__
void enableEcho(struct termios *old)
{
    tcsetattr(STDIN_FILENO, TCSANOW, old);
}

void disableEcho(struct termios *old)
{
    struct termios new;
    tcgetattr(STDIN_FILENO, old);
    new = *old;
    new.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
}
#endif

void askForPassNameInput(char *name, char *password)
{
    #ifdef __linux__
    disableEcho(&oldTermios);
    #endif
    fprintf(stdout, "Enter a password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;
    #ifdef __linux__
    enableEcho(&oldTermios);
    #endif

    clearInputBuffer();
    fprintf(stdout, "Enter a name associated with the password: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;
}

bool askForMainPassword(size_t mainPassLenght)
{
    char s[mainPassLenght];

    #ifdef __linux__
    disableEcho(&oldTermios);
    #endif
    fprintf(stdout, "Enter main password: ");
    fgets(s, sizeof(s), stdin);
    s[strcspn(s, "\n")] = 0;
    fprintf(stdout, "\n");
    #ifdef __linux__
    enableEcho(&oldTermios);
    #endif

    if (strcmp(s, mainPass) != 0)
        return false;

    return true;
}

typedef struct
{
    char name[100];
    char password[150];
} Password;

Password *createPasswords(size_t size)
{
    Password *passwords;
    passwords = (Password *)malloc(size * sizeof(Password));
    if (!passwords)
    {
        fprintf(stderr, "Cannot allocate memory for passwords!\n");
        return NULL;
    }

    return passwords;
}

void releasePasswords(Password *p)
{
    free(p);
}

size_t checkForEmptyIndexProfile(const Password *p, size_t lenght)
{
    for (size_t i = 0; i < lenght; i++)
    {
        if (!p[i].name[0] && !p[i].password[0])
            return i;
    }

    return -1;
}

void clearInputBuffer()
{
    int i;
    while ((i = getchar()) != '\n' && i != EOF);
}

int main()
{
    sprintf(mainPass, "Ne"); // Change this to whatever password you want
    {
        FILE *f = fopen("Data.bin", "ab");
        fclose(f);
    }
    int choise = 0;
    while (running)
    {
        fprintf(stdout, "1 - Add a new password \n2 - Show all passwords \n3 - Exit \n");
        scanf("%d", &choise);
        clearInputBuffer();

        switch (choise)
        {
        case 1:
        {
            Password *passwords = createPasswords(PASSWORD_ARRAY_LENGHT);
            FILE *file;
            {
                if(!askForMainPassword(sizeof(mainPass)))
                {
                    fprintf(stderr, "Incorrect main password!\n");
                    releasePasswords(passwords);
                    return 1;
                }

                file = fopen("Data.bin", "rb");
                if (!file)
                {
                    fprintf(stderr, "Unable to open file!\n");
                    releasePasswords(passwords);
                    return 1;
                }

                fread((void *)passwords, sizeof(Password), PASSWORD_ARRAY_LENGHT, file);
                fclose(file);

                size_t emptyIndex = checkForEmptyIndexProfile(passwords, PASSWORD_ARRAY_LENGHT);
                askForPassNameInput(passwords[emptyIndex].name, passwords[emptyIndex].password);
                encryptString(passwords[emptyIndex].password);

                file = fopen("Data.bin", "wb");
                fwrite((const void *)passwords, sizeof(Password), PASSWORD_ARRAY_LENGHT, file);
                fclose(file);
            }
            releasePasswords(passwords);
        }
        break;

        case 2:
        {
            Password *passwords = createPasswords(PASSWORD_ARRAY_LENGHT);
            FILE *file;
            {
                if(!askForMainPassword(sizeof(mainPass)))
                {
                    fprintf(stderr, "Incorrect main password!\n");
                    releasePasswords(passwords);
                    return 1;
                }

                file = fopen("Data.bin", "rb");
                if (!file)
                {
                    fprintf(stderr, "Unable to open file!\n");
                    releasePasswords(passwords);
                    return 1;
                }
                fread((void *)passwords, sizeof(Password), PASSWORD_ARRAY_LENGHT, file);
                fclose(file);

                // char mainPasswd[100];
                // fprintf(stdout, "Enter the main password: ");

                // disableEcho(&oldTermios);
                // fgets(mainPasswd, sizeof(mainPasswd), stdin);
                // mainPasswd[strcspn(mainPasswd, "\n")] = 0;
                // fprintf(stdout, "\n");
                // enableEcho(&oldTermios);

                for (size_t i = 0; i < PASSWORD_ARRAY_LENGHT; i++)
                {
                    if (!passwords[i].name[0] && !passwords[i].password[0])
                        continue;
                    else
                    {
                        decryptString(passwords[i].password);
                        fprintf(stdout, "%s: %s\n", passwords[i].name, passwords[i].password);
                    }
                }
                fprintf(stdout, "\n\n\n\n Press any key to continue...\n");
                getchar();
            }
            releasePasswords(passwords);
        }
        break;
        case 3:
            running = false;
            break;
        default:
            fprintf(stderr, "Incorrect option\n");
            break;
        }
        #ifdef __linux__
        system("clear");
        #elif __WIN32__
        system("cls");
        #endif
    }
    return 0;
}
