#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "node.h"

const int MAX_LENGTH = 80;
const char NULL_CHAR = '\0';
const char NEWLINE = '\n';
int daeCounter;
const char *EXIT = "exit";
const char *ARG_CHECK = "argcheck";
const char SEPARATOR = '|';
const char INPUT = '>';
const char OUTPUT = '<';
const char *REDO = "!!";
const char *DAECHECK = "daemon";
const char *KD = "kill-daemon";
const char *HELP = "help";
const char *CD = "cd";

const char *HELP_STATEMENT = "\n"
                             "Key words: \n"
                             "                  cd change directory\n"
                             "                  > file out, < file in\n"
                             "                  !! redo\n"
                             "                  | pipe\n"
                             "                  exit to exit\n"
                             "                  daemon to check bg processes\n"
                             "                  kill-daemon to kill all the running bg processes";

int betterGets(char *s, int arraySize);

int tokenizer(int size, char *input, char **args, char **argv);

void argFilter(char **args, char **argv, int argCount);

void f_error(char *s);

pid_t createChildProcess(int flag, const char *path, char *const argv[], int fdIn, int fdOut, int fdErr);

int symbolChecker(char **args, int argCount);

void executeProcesses(char **argv, int argCount, pid_t *daemonArray);

void daemonCheck(int *daemonArray);

void daemonSuspend(char **argv, int *daemonArray, int flag);

void loopMain(char **argv, int argCount, int *daemonArray);


int main(void) {
    int should_run = 1;
    char input[MAX_LENGTH];
    char *args[MAX_LENGTH / 2 + 1];   /* command line (of 80) has max of 40 arguments */
    pid_t *daemonArray = malloc(sizeof(pid_t) * 100);
    int daeCounter = 0;
    header = (struct HistoryNode *) malloc(sizeof(struct HistoryNode));
    tail = header;
    while (should_run) {
        int argCount = 0, size;
        char **argv;
        printf("mysh:~$");
        size = betterGets(input, MAX_LENGTH);
        argCount = tokenizer(size, input, args, argv);
        argv = (char **) malloc(argCount + 1);
        argFilter(args, argv, argCount);
        addLast(argv, argCount);

        if (strcmp(argv[0], EXIT) == 0) {
            should_run = 0;
            if (daeCounter > 0) {
                daemonSuspend(NULL, daemonArray, 1);
                emptyList();
            }
        } else {
            loopMain(argv, argCount, daemonArray);
        }
        return 0;
    }
}

void daemonCheck(int *daemonArray) {
    printf("There are currently %d daemon/s running in background\n"
           "Use kill-daemon to kill all running daemon processes in the list\n"
           "Or use the index number to kill a certain process\n", daeCounter);
    for (int i = 0; i < daeCounter; i++) {
        printf("Daemon %d: Process id = %d", i, daemonArray[i]);
    }
    printf("\n");
}

void daemonSuspend(char **argv, int *daemonArray, int flag) {
    int in;
    if (daeCounter == 0) {
        printf("No daemons currently running\n");
    } else if (flag == 1 || argv[1] == NULL) {
        for (int i = 0; i < daeCounter; i++) {
            kill(daemonArray[i], SIGKILL);
            daemonArray[in] = 0;
        }
        daeCounter = 0;
    } else if ((in = atoi(argv[1])) >= 0) {
        kill(daemonArray[in], SIGKILL);
        daemonArray[in] = 0;
        daemonArray[in] = daemonArray[daeCounter - 1];
        daeCounter--;
    }
}

int tokenizer(int size, char *input, char **args, char **argv) {
    int argCount = 0;
    char *t = NULL;
    char *tBuffer = malloc(size);
    strcpy(tBuffer, input);
    while ((t = strtok(tBuffer, " ")) != NULL) {
        tBuffer = NULL;
        args[argCount] = malloc(strlen(t));
        strcpy(args[argCount], t);
        argCount++;
    }

    return argCount;
}

pid_t createChildProcess(int flag, const char *path, char *const argv[], int fdIn, int fdOut, int fdErr) {
    pid_t pid;
    pid = fork();
    if (pid < 0) {
        f_error("fork failed");
    } else if (pid == 0) {
        if (fdIn != 0) {
            if (dup2(fdIn, 0) < 0) f_error("dup2 failed for file input");
            close(fdIn);
        }
        if (fdOut != 1) {
            if (dup2(fdOut, 1) < 0) f_error("dup2 failed for file output");
            close(fdOut);
        }
        if (fdErr != 2) {
            if (dup2(fdErr, 2) < 0) f_error("dup2 failed for file error");
            close(fdErr);
        }
        for (int i = 3; i <= 8; i++) close(i);
        if (flag == 1) {
            daemon(1, 0);
        }
        if (execvp(path, argv) == -1)
            f_error("Could not execute command");
        exit(0);
    }
    return pid;
}

int betterGets(char *s, int arraySize) {
    int i = 0, maxIndex = arraySize - 1;
    char c;
    while (i < maxIndex && (c = getchar()) != NEWLINE) {
        s[i] = c;
        i = i + 1;
    }
    s[i] = NULL_CHAR;
    return i;
}

void f_error(char *s) {
    if (errno != 0)
        perror(s);
    else
        fprintf(stderr, "%s\n", s);
}

void argFilter(char **args, char **argv, int argCount) {
    for (int i = 0; i < argCount; i++) {
        argv[i] = malloc(strlen(args[i] + 1));
        strcpy(argv[i], args[i]);
    }
    argv[argCount] = NULL;
}

int symbolChecker(char **args, int argCount) {
    for (int i = 1; i < argCount; ++i) {
        if (strlen(args[i]) == 1) {
            if (args[i][0] == SEPARATOR || args[i][0] == INPUT || args[i][0] == OUTPUT)
                return i;
        }
    }
    return -2;
}

void executeProcesses(char **argv, int argCount, pid_t *daemonArray) {
    int sepIndex, fdIn = STDIN_FILENO, fdOut = STDOUT_FILENO, fdErr = STDERR_FILENO, fdPipe[2];
    pid_t p, q;
    sepIndex = symbolChecker(argv, argCount);
    if (argv[argCount - 1][0] == '&') {
        argv[argCount - 1] = NULL;
        p = createChildProcess(1, argv[0], argv, fdIn, fdOut, fdErr);
        daemonArray[daeCounter] = p;
        daeCounter++;
        printf("[%ld] %s Started in background\n", p, argv[0]);
    } else if (sepIndex != -2 && argv[sepIndex][0] == SEPARATOR) {
        int secArgs = sepIndex + 1;
        argv[sepIndex] = NULL;
        if (pipe(fdPipe) < 0) {
            f_error("Pipe creation failed");
        }

        p = createChildProcess(0, argv[0], argv,
                               fdIn, fdPipe[1], fdErr);

        q = createChildProcess(0, argv[secArgs], argv + secArgs,
                               fdPipe[0], fdOut, fdErr);
        if (0 > close(fdPipe[0])) f_error("Unable to close pipe 0");
        if (0 > close(fdPipe[1])) f_error("Unable to close pipe 1");
        wait(&p);
        wait(&q);

    } else {


        if (sepIndex != -2 && argv[sepIndex][0] == OUTPUT) {
            printf("We see an input at %d\n", sepIndex);
            char *fName = argv[sepIndex + 1];
            if (fName == NULL) {
                f_error("No input file provided");
            } else if ((fdIn = open(fName, O_RDONLY)) < 0) {
                f_error("Unable to open the file");
            }
            argv[sepIndex] = NULL;
        } else if (sepIndex != -2 && argv[sepIndex][0] == INPUT) {
            char *fName = argv[sepIndex + 1];
            if (fName == NULL) {
                f_error("File name not found");
            } else if ((fdOut = open(fName, O_RDWR | O_CREAT, 0666)) < 0) {
                f_error("Unable to write on the file");
            }
            argv[sepIndex] = NULL;
        }
        p = createChildProcess(0, argv[0], argv, fdIn, fdOut, fdErr);
        if (fdIn != STDIN_FILENO) {
            if (0 > close(fdIn)) f_error("could not close input");
        }
        if (fdOut != STDOUT_FILENO) {
            if (0 > close(fdOut)) f_error("could not close output");
        }
        if (fdIn != STDERR_FILENO) {
            if (0 > close(fdErr)) f_error("could not close error");
        }
        wait(&p);
    }
}

void loopMain(char **argv, int argCount, int *daemonArray) {
    if (strcmp(argv[0], HELP) == 0) {
        printf("%s\n", HELP_STATEMENT);
    } else if (strcmp(argv[0], ARG_CHECK) == 0) {
        for (int i = 0; i < argCount; i++) {
            printf("%s", argv[i]);
        }
        printf("\n");
    } else if (strcmp(argv[0], CD) == 0) {
        chdir(argv[1]);
    } else if (strcmp(argv[0], DAECHECK) == 0) {
        daemonCheck(daemonArray);
    } else if (strcmp(argv[0], KD) == 0) {
        daemonSuspend(argv, daemonArray, 0);
    } else if (strcmp(argv[0], REDO) == 0) {
        if (tail == header) {
            printf("No commands in history.");
        } else if (strcmp(tail->previous->input[0], REDO) == 0) {
            printf("Action restricted. The last action was a redo. Which will create an infinite loop\n");
        } else {
            struct HistoryNode *cur = tail->previous;
            printf("Executing \t");
            for (int i = 0; i < cur->argc; ++i) {
                printf("%s ", cur->input[i]);
            }
            printf("\n");
            loopMain(cur->input, cur->argc, daemonArray);
        }
    } else {
        executeProcesses(argv, argCount, daemonArray);
    }

}