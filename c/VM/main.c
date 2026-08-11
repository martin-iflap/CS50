#include <stdio.h>
#include <stdlib.h>

#include "vm/vm.h"
#include "arena/arena_allocator.h"


#include <stdio.h>
#include <stdlib.h>

int main() {
    // Open the file in read mode and ensure its opened well
    FILE *filePointer = fopen("source.txt", "r");

    if (filePointer == NULL) {
        printf("Error: Could not open source file.\n");
        return 1; 
    }

    // Measure the exact file size
    fseek(filePointer, 0, SEEK_END);
    long fileSize = ftell(filePointer);
    rewind(filePointer);

    // Allocate exact memory +1 for null-terminator
    char *source = (char *)malloc(fileSize + 1);
    if (source == NULL) {
        printf("Error: Memory allocation failed.\n");
        fclose(filePointer);
        return 1;
    }

    size_t bytesRead = fread(source, 1, fileSize, filePointer);
    source[bytesRead] = '\0'; 

    runVM(source);

    fclose(filePointer);
    free(source); // don't leak mem

    return 0;
}


// take a look at elifs, rn double if creates unterminated stmt in parser, check out why
// keep in mind return is inconsistent
// add a way to print newlines

// COMPILE:
// gcc arena/arena_allocator.c lexer/lexer.c parser/parser.c compiler/compiler.c vm/vm.c main.c -o code
