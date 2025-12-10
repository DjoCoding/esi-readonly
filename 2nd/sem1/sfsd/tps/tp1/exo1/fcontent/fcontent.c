#include <fcontent.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char *fcontent(const char *filename) {
    FILE *f = fopen(filename, "r");
    if(!f) {
        fprintf(stderr, "fcontent failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *content = calloc(1, size + 1);
    if(content == NULL) {
        fprintf(stderr, "fcontent failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } 

    size_t read = fread(content, size, 1, f);
    if(read != 1) {
        fprintf(stderr, "fcontent failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    fclose(f);

    return content;
}