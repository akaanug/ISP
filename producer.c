#include <stdlib.h>
#include <stdio.h>

void printRandom(int);

int main(int argc, char *argv[]) {

    if( argc == 2 ) {
        long iteration = strtol(argv[1],NULL,10);
        printRandom(iteration);
    } else {
        printf("Producer error. Enter a single argument.\n");
    }


}

void printRandom(int iteration) {
    for( long i = 0; i < iteration; i++ ) {
        char randomletter = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"[ random() % 62 ];
        printf("%c", randomletter);
    }
}