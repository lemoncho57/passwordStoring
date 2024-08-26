#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>

#define PASSWORD_ARRAY_LENGHT 50

char mainPass[100];
bool running = true;
struct termios oldTermios;

void clearInputBuffer();

char *encryptString(char *string)
{
    while (*string != '\0')
    {
        *string += 23;
        ++string;
    }
    return string;
}

char *decryptString(char *string)
{
    while (*string != '\0')
    {
        *string -= 23;
        ++string;
    }
    return string;
}

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

void askForPassNameInput(char *name, char *password)
{
    disableEcho(&oldTermios);
    fprintf(stdout, "Enter a password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;
    enableEcho(&oldTermios);

    clearInputBuffer();
    fprintf(stdout, "Enter a name associated with the password: ");
    fgets(name, sizeof(name), stdin);
}

bool askForMainPassword(size_t mainPassLenght)
{
    char s[mainPassLenght];

    disableEcho(&oldTermios);
    fprintf(stdout, "Enter main password: ");
    fgets(s, sizeof(s), stdin);
    s[strcspn(s, "\n")] = 0;
    fprintf(stdout, "\n");
    enableEcho(&oldTermios);

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
        fprintf(stdout,"\n");
    }
    return 0;
}