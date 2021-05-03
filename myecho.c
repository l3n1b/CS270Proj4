/* myecho.c */

#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[]){
    char lineIn[256]; 
    fgets(lineIn, 256, stdin);
    printf("HA: %s", lineIn);
    
    for (int j = 0; j < argc; j++){
        printf("argv[%d]: %s\n", j, argv[j]);
    }

    exit(EXIT_SUCCESS);
}