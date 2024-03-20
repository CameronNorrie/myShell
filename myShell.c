#include "myShell.h"   /* header of function file declarions */

/*
 *  CIS*3110 Assignment
 *  author: Cameron Norrie
 *  Project: myShell.c
 *  Date: Feb 12, 2021
 */

int main(int argc, char const *argv[]) {

  char *parameters[10];
  char input[1000];
  char buffer[100];
  int params;
  char *token = NULL;
  //file descriptors and booleans used for storing stdin & out if they are changed to a file
  int savedStdout;
  int tmpStdout;
  int savedStdin;
  int tmpStdin;
  int piped = 0;
  int hasOutputted;
  //environment variable
  char buff[PATH_MAX]= ""; //temp size of path
  char *HOME = malloc(PATH_MAX);
  strcpy(HOME,getcwd(buff,PATH_MAX));
  while(1) {
    hasOutputted = 0;
    printf("%s>",HOME);
    //get user Input
    if (fgets(input,1000, stdin) ==NULL) {
      //if fgets fail retry
      continue;
    }
    if(input[strlen(input)-1] == '\n') {
      input[strlen(input)-1] = '\0';
    }
    params = 0;
    token = strtok(input, " ");
    while(token != NULL) {
      parameters[params] = malloc(strlen(token) + 1);
      strcpy(parameters[params],token);
      params++;
      token = strtok(NULL, " ");
    }
    parameters[params] = NULL;
    //history command runs relative to this shell and needs it own  method
    printToHistory(parameters);
    if(strcmp(parameters[0], "history") == 0) {
      execHistory(parameters[1]);
      printToHistory(parameters);
      continue;
    }

    //check for output redirection
    for(params = 0; parameters[params] != NULL; params++) {
      //change output
      if(strcmp(parameters[params],">") == 0) {
        //remove extra params so ls will print o file correctly
        parameters[params] = NULL;
        //save stdout with dup to restore later
        savedStdout = dup(1);
        dup2(tmpStdout,1);
        freopen(parameters[params+1],"w",stdout);
        execOutputRedirect(parameters, savedStdout, tmpStdout);
        hasOutputted = 1;
        break;
      }
      //change input
      if(strcmp(parameters[params],"<") == 0) {
        //remove extra params so ls will print o file correctly
        parameters[params] = NULL;
        //save stdin with dup to restore later
        savedStdin = dup(0);
        dup2(tmpStdin,0);
        freopen(parameters[params+1], "r", stdin);
        execInputRedirect(parameters, savedStdin, tmpStdin);
        hasOutputted = 1;
        break;
      }
      if(strcmp(parameters[params], "&") == 0) {
        parameters[params] = NULL;
        execBackground(parameters);
        hasOutputted = 1;
        break;
      }
      //check for pipe
      if(strcmp(parameters[params], "|") == 0) {
        execPipe(parameters,params);
        hasOutputted = 1;
        break;
      }
      if(strcmp(parameters[params], "cd") == 0) {
        changeDir(parameters[1],HOME);
        hasOutputted =1;
      }
    }
    if(!hasOutputted) {
      execCommand(parameters);
    }
  }
  printf("Goodbye\n");
  return 0;
}
void changeDir(char * location,char *path) {
  chdir(location);
  char buff[PATH_MAX]= ""; //temp size of path
  char *HOME = getcwd(buff,PATH_MAX);
  path = realloc(path, strlen(HOME) +1);
  strcpy(path, HOME);
}
void execHistory(char *param) {
  if(param == NULL) {
    FILE *fp = fopen(".myshell_history", "r");
    char buffer[50] = "";
    while(fgets(buffer, 50, fp) != NULL) {
      printf("%s",buffer);
    }
  } else if(strcmp(param, "-c") == 0) {
    remove(".myshell_history");
  } else {
    printf("bash: history %s invalid option\n", param);
  }
}
void execCommand(char **parameters) {
  char *PATH = "/bin";
  pid_t childpid;   /* child's process id */
  int status = 0;       /* for parent process: child's exit status */
  int params;
  //Create new process

  childpid = fork();

  if(childpid < 0) {
    printf("Fork failed\n" );
    return;
  }
  //if exit is inputted kill child running and terminate the shell
  if(strcmp(parameters[0], "exit") == 0) {
    kill(childpid, SIGKILL);
    exit(0);
  }
  if (childpid >= 0) { //for succeeded
    if(childpid == 0){
      if(strcmp("echo", parameters[0]) == 0) {
        if(strcmp(parameters[1], "$PATH") == 0) {
          free(parameters[1]);
          parameters[1] = malloc(strlen(PATH) +1);
          strcpy(parameters[1], PATH);
        }
        else if(strcmp(parameters[1], "$HOME") == 0) {
          char buff[PATH_MAX]= ""; //temp size of path
          char *HOME = getcwd(buff,PATH_MAX);
          free(parameters[1]);
          parameters[1] = malloc(strlen(HOME) +1);
          strcpy(parameters[1], HOME);
        } else if(strcmp(parameters[1], "$HISTFILE") == 0) {
          char buff[PATH_MAX]= ""; //temp size of path
          char *HOME = getcwd(buff,PATH_MAX);
          free(parameters[1]);
          parameters[1] = malloc(strlen(HOME) + 20);
          strcpy(parameters[1], HOME);
          strcat(parameters[1], "/.CIS3110_history");
        }
      }
      if(strcmp("cd", parameters[0]) == 0) {
        chdir(parameters[1]);
        //exit(0);
      }
      else {
        status = execvp(parameters[0],parameters);
        exit(status);
      }
    } else {
      //wait for child to finish
      waitpid(childpid,&status,0);
      if(status != 0) {
        //check for missing file error
        if(parameters[0][0] == '.') {
          printf("-myShell: %s: No such file or directory\n",parameters[0] );
        } else if(status != 512){
          printf("bash: %s: command not found\n",parameters[0]);
        }
      }
      for(params = 0; parameters[params] != NULL; params++) {
        free(parameters[params]);
      }
      //exit(0);
    }
  } else {
    printf("Fork Failed\n");
    exit(0);

  }
}
void printToHistory(char **params) {
  FILE *fp = fopen(".myshell_history", "a+");
  int commands = 0;
  char buffer[50] = "";
  while(fgets(buffer, 50, fp) != NULL) {
    commands++;
  }
  //print command number
  fprintf(fp, " %d  ",commands+1 );
  for(int i = 0; params[i] != NULL; i++) {
    fprintf(fp, "%s ",params[i] );
  }
  fprintf(fp, "\n");
  fclose(fp);
}
void execBackground(char **parameters) {
  pid_t childpid;   /* child's process id */
  int status = 0;       /* for parent process: child's exit status */
  int params;
  //Create new process
  //if exit is inputted kill child running and terminate the shell
  if(strcmp(parameters[0], "exit") == 0) {
    kill(childpid, SIGKILL);
    exit(0);
  }
  childpid = fork();
  if (childpid >= 0) { //for succeeded
    if(childpid == 0){
      status = execvp(parameters[0],parameters);
      //setpgid(0,0);
      //exit(status);
    } else {
      //wait for child to finish
      waitpid(childpid,&status,WNOHANG);
      // if(status != 0) {
      //   printf("bash: %s: command not found\n",parameters[0]);
      // }
      for(params = 0; parameters[params] != NULL; params++) {
        free(parameters[params]);
      }
      //exit(0);
    }
  } else {
    printf("Fork failed\n");
    exit(0);

  }
}
void execInputRedirect(char **parameters, int savedStdin, int tmpStdin) {
  pid_t childpid;   /* child's process id */
  int status;       /* for parent process: child's exit status */
  int params;
  //Create new process
  childpid = fork();
  if(childpid < 0) {
    printf("Fork failed\n" );
    return;
  }
  //if exit is inputted kill child running and terminate the shell
  if(strcmp(parameters[0], "exit") == 0) {
    kill(childpid, SIGKILL);
    exit(0);
  }
  if (childpid >= 0) { //for succeeded
    if(childpid == 0){
      status = execvp(parameters[0],parameters);
      exit(status);
    } else {
      //wait for child to finish
      waitpid(childpid,&status,0);
      if(status == -1) {
        printf("Command %s failed\n",parameters[0]);
      }
      //restore stdin
      dup2(savedStdin,0);
      close(savedStdin);

      for(params = 0; parameters[params] != NULL; params++) {
        free(parameters[params]);
      }
      //exit(0);
    }
  } else {
    printf("Fork Failed\n");
    exit(0);

  }
}
void execOutputRedirect(char **parameters, int savedStdout, int tmpStdout) {
  pid_t childpid;   /* child's process id */
  int status;       /* for parent process: child's exit status */
  int params;
  //Create new process
  childpid = fork();
  if(childpid < 0) {
    printf("Fork failed\n" );
    return;
  }
  //if exit is inputted kill child running and terminate the shell
  if(strcmp(parameters[0], "exit") == 0) {
    kill(childpid, SIGKILL);
    exit(0);
  }
  if (childpid >= 0) { //for succeeded
    if(childpid == 0){
      status = execvp(parameters[0],parameters);
      exit(status);
    } else {
      //wait for child to finish
      waitpid(childpid,&status,0);
      if(status == -1) {
        printf("Command %s failed\n",parameters[0]);
      }
      //restore stdout
      dup2(savedStdout,1);
      close(savedStdout);
      for(params = 0; parameters[params] != NULL; params++) {
        free(parameters[params]);
      }
      //exit(0);
    }
  } else {
    printf("Fork Failed\n");
    exit(0);

  }
}
//executes command given by user that pipes output to another command
void execPipe(char **parameters, int pipeIndex) {
  int status = 0;
  int status2 = 0;
  //these are the parameters for the first command

  int cmd2 = pipeIndex + 1;
  char *parameters2[10];
  int i;
  int savedStdin = dup(STDIN_FILENO);
  for (i = cmd2; parameters[i] != NULL; i++) {
    parameters2[i-cmd2] = malloc(strlen(parameters[i]) + 1);
    strcpy(parameters2[i-cmd2],parameters[i]);
  }
  //i++;
  parameters2[i -cmd2] = NULL;
  parameters[pipeIndex] = NULL;
  //file descriptors for piping
  int fd[2];
  pipe(fd);
  pid_t pid, pid2;
  pid = fork();
  if(pid < 0) {
    printf("Fork failed\n" );
    return;
  }

  //child process
  if(pid == 0) {
    //CHILD 1
    pid2 = fork();
    if(pid2 < 0) {
      printf("Fork failed\n" );
      return;
    }
    if(pid2 == 0) {
      //GRANDCHILD 1
      //read end not needed for child process
      close(fd[READ]);
      //set stdout to output to fd[1]
      dup2(fd[WRITE], STDOUT_FILENO);
      //fd[WRITE] is now stdout so we can close it
      //close(fd[WRITE]);
      status2 = execvp(parameters[0], parameters);
      exit(status2);
    }
    else {
      //INNER PARENT
      //wait for grandchild to finish execution
      waitpid(pid2, &status2, 0);
      //run cmd2
      close(fd[WRITE]);
      dup2(fd[READ], STDIN_FILENO);
      status =execvp(parameters2[0], parameters2);
      exit(status);
    }

  } else {
    //PARENT
    close(fd[0]);
    close(fd[1]);
    wait(NULL);
  }
  for(i = 0; parameters[i] != NULL; i++) {
    free(parameters[i]);
  }
  for(i = 0; parameters2[i] != NULL; i++) {
    free(parameters2[i]);
  }
}
