#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

#define PATH_BUFSIZE 1024
#define COMMAND_BUFSIZE 1024
#define MAXARG 20
#define TOKEN_BUFSIZE 64
#define TOKEN_DELIMITERS " \t\r\n\a"
#define BACKGROUND_EXECUTION 0
#define FOREGROUND_EXECUTION 1
#define PIPELINE_EXECUTION 2

struct command_segment {
    char *args[MAXARG];   // arguments array
    struct command_segment *next;
    pid_t pid;   // process ID
    pid_t pgid;   // process group ID
};

struct command {
    struct command_segment *root;   // a linked list
    int mode;   // BACKGROUND_EXECUTION or FOREGROUND_EXECUTION
};


void mysh_cd(char *path) {
   if (chdir(path) < 0)
      printf(" cd %s: No such file or directory\n", path); 
}

void mysh_exit() {
    /* Release all the child process */
    
}


int mysh_execute_buildin_command(struct command_segment *segment){
   /* Match if command name is a internal command */
    if (strcmp(segment->args[0], "cd") == 0) {
        mysh_cd(segment->args[1]);
        return 0;
    }
    else if (strcmp(segment->args[0], "exit") == 0) {
        return 1;
    }
    return 0;
}

int mysh_execute_command(struct command *command) {
    int status = 1;
    struct command_segment* cur; 
    struct command_segment* pfree;
    cur = command->root;
    pfree = cur; 
    while (cur != NULL) {
        int i;
        for (i = 0; i < MAXARG  && cur->args[i] != NULL; i++)
            printf("%s\n", cur->args[i]);
        printf("command segent end\n");
        cur = cur->next;
        free(pfree);
        pfree = cur;
    }

    cur = NULL;
    pfree = NULL;
    free(command);
    command = NULL;
    return status;
}


struct command* mysh_parse_command(char *line) {
    /* Parse line as command structure */
    struct command* command = (struct command*)malloc(sizeof(struct command));
    command->root = (struct command_segment*)malloc(sizeof(struct command_segment));
    struct command_segment* cur;
    struct command_segment* pnew;
    cur = command->root; 

    char* pStart = line;
    int count;
    while ((*pStart != '\n') && (*pStart != '\0')) {
        if (*pStart == '&') count = 1; 
	else count = 0;
        pStart++;
    }
    command->mode = count;

    char *res = line;
    char *temp;
    int i = 0;
    temp = strsep(&res, "|");
    for (i = 0; i < MAXARG - 1 && (cur->args[i] = strtok(temp, TOKEN_DELIMITERS)) != NULL; i++)
        temp  = NULL;
    cur->args[i] = NULL;
    while ((temp = strsep(&res, "|")) != NULL) {
        pnew = (struct command_segment*) malloc(sizeof(struct command_segment));
        cur->next = pnew;
        cur = pnew;
        for (i = 0; i < MAXARG - 1 && (cur->args[i] = strtok(temp, TOKEN_DELIMITERS)) != NULL; i++)
            temp = NULL;
        cur->args[i] = NULL;
    }
    cur->next = NULL;
    return command;
}

char* mysh_read_line() {
    int bufsize = COMMAND_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "-mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {    // read just one line per time
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        if (position >= bufsize) {   // handle overflow case
            bufsize += COMMAND_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "-mysh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void mysh_print_promt() {
    /* Print "<username> in <current working directory>" */
    char namebuf[PATH_BUFSIZE];
    getlogin_r(namebuf, sizeof(namebuf));
    printf("%s in ", namebuf);
    char pathbuf[PATH_BUFSIZE];
    getcwd(pathbuf, sizeof(pathbuf));
    printf("%s\n", pathbuf);

    /* Print ">mysh " */
    printf(">mysh ");
}

void mysh_print_welcome() {
    /* Print "Welcome to mysh by <student ID>!" */
    printf("Welcome to mysh by 0440052\n");
}

void mysh_loop() {
    char *line;
    struct command *command;
    int status = 1;

    do {   // an infinite loop to handle commands
        mysh_print_promt();
        line = mysh_read_line();   // read one line from terminal
        if (strlen(line) == 0) {
            continue;
        }
        command = mysh_parse_command(line);   // parse line as command structure
        status = mysh_execute_command(command);   // execute the command
        free(line);
    } while (status >= 0);
}

void mysh_init() {
    /* Do any initializations here. You may need to set handlers for signals */

}

int main(int argc, char **argv) {
//    mysh_init();
    mysh_print_welcome();
    mysh_loop();

    return EXIT_SUCCESS;
}