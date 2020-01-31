#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

const int MAX_LENGTH = 80;
const char NULL_CHAR = '\0';
const char NEWLINE = '\n';
const char *EXIT = "exit";
const char *ARG_CHECK = "argcheck";
const char *SEPARATOR = "&&";
const char *HELP ="help";
const char *HELP_STATEMENT="A big help statement";
int betterGets(char *s, int arraySize);
int checkPiping(char **args, int argCount);
int betterStrComp(char *a, char *b);

int main(void) {
    int should_run = 1;
    char input[MAX_LENGTH];
    char *args[MAX_LENGTH / 2 + 1];    /* command line (of 80) has max of 40 arguments */
    char *pch;
    while (should_run) {
        int argCount = 0, size, sepIndex;
        char *t = NULL;
        printf("mysh:~$");
        fflush(stdout);
        size = betterGets(input, MAX_LENGTH);
        char *tBuffer = malloc(size);
        strcpy(tBuffer, input);
        while ((t = strtok(tBuffer, " ")) != NULL) {
            tBuffer = NULL;
            args[argCount] = malloc(strlen(t));
            strcpy(args[argCount], t);
            argCount++;
        }
        printf("%d\n", argCount);
        if (strcmp(args[0], EXIT) == 0) {
            should_run = 0;
        } else if (strcmp(args[0], HELP)==0){
            printf("$s",HELP_STATEMENT);
        } else if (strcmp(args[0], ARG_CHECK) == 0) {
            for (int i = 0; i < argCount; i++) {
                printf("%s", args[i]);
            }
            printf("\n");
        } else if (sepIndex= checkPiping(args, argCount)>=0){
            printf("piping at %d\n",sepIndex);
        }
        /**
          * After reading user input, the steps are:
          * (1) fork a child process
          * (2) the child process will invoke execvp()
          * (3) if command includes &, parent and child will run concurrently
          */
        for (int j = 0; j < argCount; j++) {
            free(args[j]);
        }
    }

    return 0;
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

int checkPiping(char **args, int argCount) {
    int notFC = -2;
    for (int i = 0; i < argCount; ++i) {
        if (strcmp(args[i],SEPARATOR)== 0){
            return i;
        }

    }
    return -2;
}
