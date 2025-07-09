#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64
#define HISTORY_SIZE 100

char *history[HISTORY_SIZE];
int history_count = 0;

void handle_sigint(int sig) {
    write(STDOUT_FILENO, "\nMiniShell$ ", 12);
}

void add_to_history(const char *cmd) {
    if (history_count == HISTORY_SIZE) {
        free(history[0]);
        for (int i = 1; i < HISTORY_SIZE; i++)
            history[i - 1] = history[i];
        history_count--;
    }
    history[history_count++] = strdup(cmd);
}

void show_history() {
    for (int i = 0; i < history_count; i++)
        printf("%d: %s", i + 1, history[i]);
}

void parse_command(char *cmd, char **args) {
    int i = 0;
    args[i] = strtok(cmd, " \t\n");
    while (args[i] != NULL && i < MAX_ARGS - 1)
        args[++i] = strtok(NULL, " \t\n");
    args[i] = NULL;
}

int is_redirect(char *arg) {
    return strcmp(arg, ">") == 0 || strcmp(arg, "<") == 0;
}

int is_background(char **args) {
    for (int i = 0; args[i]; i++) {
        if (strcmp(args[i], "&") == 0) {
            args[i] = NULL;
            return 1;
        }
    }
    return 0;
}

int is_builtin(char **args) {
    return strcmp(args[0], "cd") == 0 || strcmp(args[0], "exit") == 0 || strcmp(args[0], "history") == 0;
}

void run_builtin(char **args) {
    if (strcmp(args[0], "cd") == 0) {
        if (args[1])
            chdir(args[1]);
        else
            fprintf(stderr, "cd: expected argument\n");
    } else if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "history") == 0) {
        show_history();
    }
}

void execute_command(char *cmd) {
    char *args[MAX_ARGS];
    char *redirect_file = NULL;
    int redirect_type = 0;

    parse_command(cmd, args);
    if (!args[0]) return;

    add_to_history(cmd);

    if (is_builtin(args)) {
        run_builtin(args);
        return;
    }

    int bg = is_background(args);

    for (int i = 0; args[i]; i++) {
        if (is_redirect(args[i])) {
            redirect_type = (strcmp(args[i], ">") == 0) ? 1 : 2;
            redirect_file = args[i + 1];
            args[i] = NULL;
            break;
        }
    }

    pid_t pid = fork();
    if (pid == 0) {
        if (redirect_type == 1 && redirect_file) {
            int fd = open(redirect_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
        } else if (redirect_type == 2 && redirect_file) {
            int fd = open(redirect_file, O_RDONLY);
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        execvp(args[0], args);
        perror("execvp failed");
        exit(1);
    } else {
        if (!bg)
            waitpid(pid, NULL, 0);
        else
            printf("[Running in background] PID: %d\n", pid);
    }
}

void execute_piped_command(char *cmd1, char *cmd2) {
    int pipefd[2];
    pipe(pipefd);

    pid_t p1 = fork();
    if (p1 == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        char *args[MAX_ARGS];
        parse_command(cmd1, args);
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    }

    pid_t p2 = fork();
    if (p2 == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        char *args[MAX_ARGS];
        parse_command(cmd2, args);
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    wait(NULL);
    wait(NULL);
}

int main() {
    char cmd[MAX_CMD_LEN];
    signal(SIGINT, handle_sigint);

    while (1) {
        printf("MiniShell$ ");
        fflush(stdout);
        if (!fgets(cmd, MAX_CMD_LEN, stdin)) break;

        if (strchr(cmd, '|')) {
            char *cmd1 = strtok(cmd, "|");
            char *cmd2 = strtok(NULL, "\n");
            if (cmd1 && cmd2)
                execute_piped_command(cmd1, cmd2);
        } else {
            execute_command(cmd);
        }
    }

    return 0;
}
