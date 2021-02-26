#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

void isp(int, int);
char ** splitString(char *);
int executeCommand(char **);
int hasPipeSymbol(char *);

int main(int argc, char *argv[]) {
    if( argc == 3 ) {
        int byteAmt = atoi(argv[1]);
        int mode = atoi(argv[2]); // 1 -> normal mode, 2 -> tapped mode

        isp(byteAmt, mode);
    } else {
        printf("\nYou should enter [BUFFER_SIZE] [MODE] arguments.\n");
    }

    return 0;
}

// To execute built-in cd command
int cdCommand( char ** args ) {
    if (args[1] == NULL) {
        fprintf( stderr, "No arguments given to \"cd\"\n" );
    } else {
        if( chdir(args[1]) != 0 ) {
            perror( "" );
        }
    }
    return 1;
}

// Returns position of pipe symbol if exists. If not return -1
int hasPipeSymbol(char * str) {
    char * index;
    index = strchr(str, '|');
    if( index != NULL ) {
        return (int)(index - str);
    }

    return -1;
}

// Below function extracts characters present in src
// between m and n (excluding n)
char* substr(const char *src, int i, int j) {
    int size = j - i;
    char * result = (char *) malloc( sizeof(char) * (size + 1) );
    strncpy( result, (src + i), size );
    return result;
}

#define READ_END	0
#define WRITE_END	1
// Two pipes.
int tappedMode( char ** args1, char ** args2, const int numOfBytes ) {

    size_t charCount = 0; // Character count
    size_t readCount = 0; // Read count
    size_t writeCount = 0; // Write count

    const int BUFFER_SIZE = numOfBytes;
    char read_msg[BUFFER_SIZE];

    // Create two pipes:
    int pipe1[2];
    int pipe2[2];
    if (pipe(pipe1) == -1) { fprintf(stderr,"Pipe 1 failed"); return 1;}
    if (pipe(pipe2) == -1) { fprintf(stderr,"Pipe 2 failed"); return 1;}

    pid_t childOne, childTwo;
    childOne = fork();

    if( childOne == -1 ) { fprintf(stderr, "Fork failed (child one)"); return 1; }
    if( childOne == 0 ) { // First Child:

        //Close Unneeded Ends:
        close(pipe2[0]); // **** close unnecessary
        close(pipe2[1]); // **** close unnecessary
        close(pipe1[0]); // **** close unnecessary

        dup2( pipe1[1], 1 ); // Write to pipe1
        close( pipe1[1] );

        if( execvp(args1[0], args1) < 0 ) { // First child executes first command
            fprintf(stderr, "\nError in cmd1\n");
            exit(0);
        }

    } else { // Parent
        childTwo = fork(); // Create second child

        if( childTwo == -1 ) { fprintf(stderr, "Fork failed (child two)"); return 1; }
        if( childTwo == 0 ) { // Second Child:


            //Close Unneeded Ends:
            close( pipe1[0] ); // **** close unnecessary
            close( pipe1[1] ); // **** close unnecessary

            close( pipe2[1] ); // Close write end of pipe2
            dup2( pipe2[0], 0 ); // Read from pipe2
            close( pipe2[0] ); // Close end

            if( execvp(args2[0], args2) < 0 ) { // Second child executes second command
                fprintf(stderr, "\nError in cmd2\n");
                exit(0);
            }
        } else { // Parent
            // Main process will read the incoming stream of bytes from pipe1 and write into pipe2:
            int status = 1;
            int count = 0;

            // Read from pipe1 BUFFER_SIZE at a time and write to pipe2 after reading.
            while( status > 0 ) {

                //Close Unneeded Ends:
                close( pipe2[0] ); // **** close unnecessary
                close( pipe1[1] ); // **** close unnecessary

                // READ FROM PIPE 1:
                status = read(pipe1[0], read_msg, BUFFER_SIZE);
                if( feof(stdin) ) { close( pipe1[0] ); } // Close the end of pipe when it is EOF

                if( status <= 0 ) { // Escape at last read before writing if read unsuccesful.
                    break;
                }

                readCount++;
                charCount += status;

                count++;

                // WRITE TO PIPE 2:
                close( pipe2[0] ); // **** close unnecessary
                close( pipe1[1] ); // **** close unnecessary

                write(pipe2[1], read_msg, status); // Write to pipe2 with size of read (BUFFER_SIZE)
                writeCount++;
                if( feof(stdin) ) { close( pipe2[1] ); } // Close the end of pipe when it is EOF
            }

            // Close pipes and wait
            close(pipe1[READ_END]);
            close(pipe1[WRITE_END]);

            close(pipe2[READ_END]);
            close(pipe2[WRITE_END]);

            int s;
            waitpid(childTwo, &s, 0);

            printf("\ncharacter-count: %lu", charCount);
            printf("\nread-call-count: %lu", readCount);
            printf("\nwrite-call-count: %lu", writeCount);
        }
    }
    return 0;
}

// Single pipe.
int normalMode( char ** args1, char ** args2 ) {

    // Create a pipe:
    int pipe1[2];
    if (pipe(pipe1) == -1) { fprintf(stderr,"Pipe failed"); return 1; }

    pid_t childOne, childTwo;
    childOne = fork();

    if( childOne == -1 ) { fprintf(stderr, "Fork failed (child one)"); return 1; }
    if( childOne == 0 ) { // First Child:
        close( pipe1[0] ); // Close read end
        dup2( pipe1[1], 1 ); // Output of first command will be written to pipe1
        close(pipe1[1]);

        if( execvp(args1[0], args1) < 0 ) { // First child executes first command
            fprintf(stderr, "\nError in cmd1\n");
            exit(0);
        }

        return 0;
    } else { // Parent
        childTwo = fork(); // Create second child

        if( childTwo == -1 ) { fprintf(stderr, "Fork failed (child two)"); return 1; }
        if( childTwo == 0 ) { // Second Child:

            close( pipe1[1] ); // Close write end
            dup2( pipe1[0], 0 ); // Read from pipe
            close(pipe1[0]);

            if( execvp(args2[0], args2) < 0 ) { // Second child executes second command
                fprintf(stderr, "\nError in cmd2\n");
                exit(0);

            }
            return 0;
        } else {
            // Close pipes
            close(pipe1[READ_END]);
            close(pipe1[WRITE_END]);

            // Wait for children
            wait( &childOne );
            wait( &childTwo );

        }
    }


    return 0;
}

void isp( int numOfBytes, int mode ) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    printf("\n->");
    while( (read = getline( &line, &len, stdin )) != -1 ) { // Read line
        if( read > 0 ) {
            int index = hasPipeSymbol(line);
            if( index != -1 ) { // Execute pipe command:
                // Arguments of first command:
                char * line1 = substr(line, 0, index - 1);
                char ** args1 = splitString(line1); // Split to arguments

                // Arguments of second command:
                char * line2 = substr(line, index + 1, strlen(line));
                char ** args2 = splitString(line2); // Split to arguments

                struct timeval t1, t2;
                double elapsedTime = 0.0;

                if( mode == 1 ) {
                    gettimeofday(&t1, NULL);
                    normalMode(args1, args2);
                    gettimeofday(&t2, NULL);

                    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
                    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
                    printf("\nTime elapsed for normal mode is %f milliseconds", elapsedTime);
                } else if( mode == 2 ) {
                    gettimeofday(&t1, NULL);
                    tappedMode(args1, args2, numOfBytes);
                    gettimeofday(&t2, NULL);

                    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
                    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
                    printf("\nTime elapsed for tapped mode (N = %d) is %f milliseconds", numOfBytes, elapsedTime);
                } else {
                    perror("\nInvalid mode.\n");
                }

                // Clean-up
                free(line1);
                free(line2);
                free(args1);
                free(args2);

                printf("\n->");
                continue;
            }

            // Execute non-pipe commands
            char ** args = splitString(line); // Split to arguments

            // Execute "cd"
            if(strcmp(args[0], "cd") == 0) {
                cdCommand(args);
            } else {
                executeCommand(args); // Execute.
            }

            // Clean-up
            free(args);
        }
        printf("\n->");
    }

    free(line);
}

// Execute non-piped commands.
int executeCommand(char ** args) {
    int signal;
    pid_t pid = fork();
    if (pid == 0) { // Child
        if( execvp(args[0], args) == -1 ) {
            perror("Error in execution\n");
        }
        free(args);
        exit(EXIT_FAILURE);
    } else if( pid < 0 ) {
        perror("Error in fork()\n");
    } else { // Parent
        do { // Wait for child
            waitpid( pid, &signal, WUNTRACED );
        } while( !WIFEXITED(signal) && !WIFSIGNALED(signal) );
    }

    return 0;
}

// Split the strings into arguments
#define BUFSIZE 64
#define DELIMITERS " \t\r\n\a"
char ** splitString(char * line) {
    int bufsize = BUFSIZE, position;
    char **args = malloc(bufsize * sizeof(char*));
    char *arg;

    if( !args ) {
        perror("Memory error\n");
        exit(EXIT_FAILURE);
    }

    arg = strtok( line, DELIMITERS );
    for( position = 0; arg != NULL; position++ ) {
        args[position] = arg;

        if( position >= bufsize ) { // If the buffer size is not big enough, resize.
            bufsize += BUFSIZE;
            args = realloc( args, bufsize * sizeof(char*) );
            if( !args ) {
                perror("Memory error\n");
                exit( EXIT_FAILURE );
            }
        }

        arg = strtok(NULL, DELIMITERS);
    }

    args[position] = NULL;
    return args;
}
