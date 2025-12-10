#include <fcontent.h>

#include <stdio.h>
#include <stdlib.h>

const char *FILENAME = "./test.txt";

int main(void) {
    char *content = fcontent(FILENAME);
    printf("%s", content);
    free(content);
    return 0;
}