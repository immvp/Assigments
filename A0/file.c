#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef enum FileTypes {
    empty,
    ASCII,
    ISO,
    UTF,
    data
} FileTypes;

// Assumes: errnum is a valid error number
int print_error(char *path, int errnum) {
    return fprintf(stdout, "%s: cannot determine (%s)\n",
        path, strerror(errnum));
}

int PrintSuccess(char str[], FileTypes fileTypes) {
    char fileType[100];

    switch (fileTypes)
    {
    case empty:
        strcpy(fileType, "empty");
        break;
    default:
        strcpy(fileType, "data");
        break;
    }

    fprintf(stdout, "%s: %s\n", str, fileType);
    return 1;
}

enum FileTypes CheckType(FILE *file) {
    long size;

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);
    
    if (size == 0) {
        return empty;
    }

    return empty;
}



int main(int argc, char* argv[]) {
    FILE* file;

    // Check if we are using illegal argument format
    if (argc != 2) {
        fprintf(stderr, "Usage: file path\n");
        exit(EXIT_FAILURE);
    }

    // Determine if the file exists
    if ((file = fopen(argv[1], "r")) != NULL) {
        
        CheckType(file);
        PrintSuccess(argv[1], empty);
        exit(EXIT_SUCCESS);
        // Determine if it's empty
        

        // We close file when done using it
        fclose(file);
        exit(EXIT_SUCCESS);
    }
    else {
        print_error(argv[1], errno);
    }
}