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
    case ASCII:
        strcpy(fileType, "ascii");
        break;
    case UTF:
        strcpy(fileType, "utf");
        break;
    default:
        strcpy(fileType, "data");
        break;
    }

    fprintf(stdout, "%s: %s\n", str, fileType);
    return 1;
}

enum FileTypes CheckType(FILE *file) {
    int size;
    unsigned char* buffer;

    // Each index corresponds to the byte number for utf-8
    int utfCounter[3];

    for (size_t i = 0; i < 3; i++)
    {
        utfCounter[i] = 0;
    }
    

    // Counter for searching for bytes
    int byteMissing = 0;
    // Byte Number we are searching for
    int numByte = 0;

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    if (size == 0) {
        return empty;
    }
    rewind(file);

    buffer = (unsigned char*) malloc(size * sizeof(unsigned char)); // Enough memory for the file
    fread(buffer, size, 1, file); // Read in the entire file
    
    // Sliding Window Pattern
    // Check om de første 3 bits er 110
    for (int i = 0; i < size; i++) {
        // If no continous byte before this one, and its between 10000000 and 10111111
        if (byteMissing == 0 && (0x80 <= buffer[i] && buffer[i] <= 0xBF)) {
            return ISO;
        }
        // 110xxxxx
        if (0xC0 <= buffer[i] && buffer[i] <= 0xDF) {
            numByte = 0;
            byteMissing = 1;
        }
        // 1110xxxx
        if (0xE0 <= buffer[i] && buffer[i] <= 0xEF) {
            numByte = 1;
            byteMissing = 2;

        }
        // 11110xxx
        if (0xF0 <= buffer[i] && buffer[i] <= 0xF7) {
            numByte = 2;
            byteMissing = 3;
        }
        // 10xxxxxx
        if (byteMissing != 0 && (0x80 <= buffer[i] && buffer[i] <= 0xBF)) {
            byteMissing--;
            if (byteMissing == 0) {
                utfCounter[numByte]++;
                numByte = 0;
            }
        }
    } 

    // Hacky solution for "end of file"-case. 
    // If the last byte of our file is a continous byte then it has to be ISO.
    if (byteMissing != 0) {
        return ISO;
    }

    if (utfCounter[1] > 0 || utfCounter[2] > 0) {
        return UTF;
    }

    return data;
}



int main(int argc, char* argv[]) {
    FILE* file;

    // Check if we are using illegal argument format
    if (argc != 2) {
        fprintf(stderr, "Usage: file path\n");
        exit(EXIT_FAILURE);
    }

    // Determine if the file exists
    if ((file = fopen(argv[1], "rb")) != NULL) {
        
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