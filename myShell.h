#include <stdio.h>              /* Input/Output */
#include <stdlib.h>             /* General Utilities */
#include <unistd.h>             /* Symbolic Constants */
#include <sys/types.h>          /* Primitive System Data Types */
#include <sys/wait.h>           /* Wait for Process Termination */
#include <errno.h>              /* Errors */
#include <string.h>             /* String Library */
#include <limits.h>

//cnstants for piping file descriptors
#define READ 0
#define WRITE 1

void execPipe(char **parameters, int pipeIndex);
void execCommand(char **parameters);
void execInputRedirect(char **parameters, int savedStdin, int tmpStdin);
void execOutputRedirect(char **parameters, int savedStdout, int tmpStdout);
void execBackground(char **parameters);
void changeDir(char * location,char *path);
void printToHistory(char **params);
void execHistory(char *param);