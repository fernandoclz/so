#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#define TRUE 1
#define FALSE 0

char **parse_command(const char *cmd, int *argc);
pid_t launch_command(char **argv)
{
    /* To be completed */
    pid_t PID = fork();
    if (PID == 0)
    {
        if (execvp(argv[0], argv) == -1)
        {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        exit(1);
    }
    return PID;
}

void leerFichero(char *f)
{
    FILE *file;

    if (!(file = fopen(f, "r")))
    {
        perror("fopen");
        return;
    }

    char buffer[512];
    char **cmd_argv;
    int cmd_argc;
    int cont = 0;
    pid_t pid;
    int status;
    while (fgets(buffer, 512, file) != NULL)
    {
        printf("@@ Running command #%d: %s\n", cont, buffer);
        cmd_argv = parse_command(buffer, &cmd_argc);
        pid = launch_command(cmd_argv);
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            printf("@@ Command #%d terminated (pid: %d, status: %d)\n", cont, pid, WEXITSTATUS(status));
        }
        cont++;
    }
    fclose(file);
    for (int i = 0; cmd_argv[i] != NULL; i++)
    {
        free(cmd_argv[i]); // Free individual argument
    }

    free(cmd_argv); // Free the cmd_argv array
}

void leerFicherob(char *f)
{
    FILE *file;

    if (!(file = fopen(f, "r")))
    {
        perror("fopen");
        return;
    }

    char buffer[512];
    char **cmd_argv;
    int cmd_argc;
    int cont = 0;
    pid_t pid[512];
    int status;
    while (fgets(buffer, 512, file) != NULL)
    {
        printf("@@ Running command #%d: %s\n", cont, buffer);
        cmd_argv = parse_command(buffer, &cmd_argc);
        pid[cont] = launch_command(cmd_argv);
        cont++;

        for (int i = 0; cmd_argv[i] != NULL; i++)
        {
            free(cmd_argv[i]); // Free individual argument
        }

        free(cmd_argv); // Free the cmd_argv array
    }
    fclose(file);

    int aux = cont;
    while (aux > 0)
    {
        pid_t pidAct = wait(&status);
        if (pidAct > 0)
        {
            int encontrado = FALSE;
            for (int i = 0; i < cont && !encontrado; i++)
            {
                if (pid[i] == pidAct)
                {
                    printf("@@ Command #%d terminated (pid: %d, status: %d)\n", i, pid[i], WEXITSTATUS(status));
                    encontrado = TRUE;
                }
            }
            if (encontrado)
            {
                aux--;
            }
        }
    }
}

char **parse_command(const char *cmd, int *argc)
{
    // Allocate space for the argv array (initially with space for 10 args)
    size_t argv_size = 10;
    const char *end;
    size_t arg_len;
    int arg_count = 0;
    const char *start = cmd;
    char **argv = malloc(argv_size * sizeof(char *));

    if (argv == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    while (*start && isspace(*start))
        start++; // Skip leading spaces

    while (*start)
    {
        // Reallocate more space if needed
        if (arg_count >= argv_size - 1)
        { // Reserve space for the NULL at the end
            argv_size *= 2;
            argv = realloc(argv, argv_size * sizeof(char *));
            if (argv == NULL)
            {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }

        // Find the start of the next argument
        end = start;
        while (*end && !isspace(*end))
            end++;

        // Allocate space and copy the argument
        arg_len = end - start;
        argv[arg_count] = malloc(arg_len + 1);

        if (argv[arg_count] == NULL)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strncpy(argv[arg_count], start, arg_len);
        argv[arg_count][arg_len] = '\0'; // Null-terminate the argument
        arg_count++;

        // Move to the next argument, skipping spaces
        start = end;
        while (*start && isspace(*start))
            start++;
    }

    argv[arg_count] = NULL; // Null-terminate the array

    (*argc) = arg_count; // Return argc

    return argv;
}

void ejercicio2A()
{
    int salir = FALSE;

    while (salir == FALSE)
    {

        printf("> ");
        fflush(stdout);
        char buffer[512];

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            printf("Salir\n");
            salir = TRUE;
            return;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strlen(buffer) == 0)
        {
            continue;
        }

        char **cmd_argv;
        int cmd_argc;
        cmd_argv = parse_command(buffer, &cmd_argc);
        pid_t pid = launch_command(cmd_argv);
        waitpid(pid, NULL, 0);
        for (int i = 0; cmd_argv[i] != NULL; i++)
        {
            free(cmd_argv[i]);
        }
        free(cmd_argv);
    }
}

int main(int argc, char *argv[])
{
    char **cmd_argv;
    int cmd_argc;
    int opt;
    char *entrada;
    int isB = FALSE;
    int isS = FALSE;
    int isI = FALSE;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s \"command\"\n", argv[0]);
        return EXIT_FAILURE;
    }

    while ((opt = getopt(argc, argv, "x:s:bi")) != -1)
    {
        switch (opt)
        {
        case 'x':
            cmd_argv = parse_command(optarg, &cmd_argc);
            launch_command(cmd_argv);
            for (int i = 0; cmd_argv[i] != NULL; i++)
            {
                free(cmd_argv[i]);
            }
            free(cmd_argv);
            break;
        case 's':
            entrada = optarg;
            isS = TRUE;
            break;
        case 'i':
            isI = TRUE;
            break;
        case 'b':
            isB = TRUE;
        default:
            break;
        }
    }

    if (isI)
    {
        ejercicio2A();
    }

    if (isS)
    {
        if (isB)
            leerFicherob(entrada);
        else
            leerFichero(entrada);
    }

    return EXIT_SUCCESS;
}