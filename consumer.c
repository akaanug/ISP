#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void readStdin(int);

int main(int argc, char *argv[]) {

    if( argc == 2 ) {
        long iteration = strtol(argv[1],NULL,10);
        readStdin(iteration);
    } else {
        printf("Consumer error. Enter a single argument.\n");
    }


}

// Read from standard input iteration times
void readStdin(int iteration) {
    char c;

    for( long i = 0; i < iteration; i++ ) {
        c = getchar();
        if(c == EOF) {
            break;
        } /*else {
            printf("\nRead: %c", c);
        }*/
    }

}