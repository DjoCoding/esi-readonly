#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>

#define __BUFFER_MAX_CAP__  1024
#define __COMMAND_MAX_CAP__ 50
#define __NAME_MAX_CAP__    100
#define __PHONE_MAX_CAP__   100  

#define __START__       ".START"
#define __END__         ".END"
#define __SEP__         ","

const char *FILENAME = "./data.bin";

typedef struct {
    char *name;
    char *phone;
} Record;

typedef enum {
    ERROR_FILE_NO_ERROR     = 0,
    ERROR_FILE_CORRUPTION,
    ERROR_COMMAND_INVALID,
    ERROR_COUNT,
} ErrorCode;

char *ERRORS[ERROR_COUNT] = {
    [ERROR_FILE_NO_ERROR]       = "no error",
    [ERROR_FILE_CORRUPTION]     = "file corrupted",
    [ERROR_COMMAND_INVALID]     = "invalid command",
};

typedef enum {
    CMD_QUIT = 0,
    CMD_LIST,
    CMD_APPEND,
    CMD_COUNT,
} Command;

char *COMMANDS[CMD_COUNT] = {
    [CMD_QUIT]          = "quit",
    [CMD_LIST]          = "list",
    [CMD_APPEND]        = "append"
};

// Commands logic
void fwriterec(FILE *f, Record rec);
void flist(FILE *f);

// Helpers
size_t nextrec(char *content, char *name, char *phone, ErrorCode *errcode);
char *strtrim(char *s);
size_t strsplit(char *s, char *r);


int main(void) {
    FILE *f = fopen(FILENAME, "ab+");
    if (!f) {
        fprintf(stderr, "failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    fseek(f, 0, SEEK_END);

    char buffer[__BUFFER_MAX_CAP__] = {0};

    while (true) {
        memset(buffer, 0, __BUFFER_MAX_CAP__);

        printf("> ");
        fflush(stdout);

        if (!fgets(buffer, __BUFFER_MAX_CAP__, stdin)) continue;

        char *cmdline = strtrim(buffer);
        if (strlen(cmdline) == 0) continue;

        char cmd[__COMMAND_MAX_CAP__] = {0};

        size_t offset = strsplit(cmdline, cmd);
        cmdline = &cmdline[offset];

        if (strcmp(cmd, COMMANDS[CMD_QUIT]) == 0) break;

        if (strcmp(cmd, COMMANDS[CMD_LIST]) == 0) {
            fseek(f, 0, SEEK_SET);
            flist(f);
            fseek(f, 0, SEEK_END);
            continue;
        }

        if (strcmp(cmd, COMMANDS[CMD_APPEND]) == 0) {
            char name[__NAME_MAX_CAP__] = {0};
            offset = strsplit(cmdline, name);
            cmdline = &cmdline[offset];

            if (offset == 0) {
                fprintf(stderr, "%s\n", ERRORS[ERROR_COMMAND_INVALID]);
                continue;
            }

            char phone[__PHONE_MAX_CAP__] = {0};
            strsplit(cmdline, phone);
            cmdline = &cmdline[offset];

            fwriterec(f, (Record){ name, phone });
            fflush(f);
            continue;
        }

        fprintf(stderr, "%s\n", ERRORS[ERROR_COMMAND_INVALID]);
    }

    fclose(f);
    return 0;
}


void fwriterec(FILE *f, Record rec) {
    size_t name_len = strlen(rec.name);
    size_t phone_len = strlen(rec.phone);

    char buffer[__BUFFER_MAX_CAP__] = {0};

    size_t offset = 0;
    
    memcpy(&buffer[offset], __START__, strlen(__START__));
    offset += strlen(__START__);

    memcpy(&buffer[offset], rec.name, name_len);
    offset += name_len;

    memcpy(&buffer[offset], __SEP__, strlen(__SEP__));
    offset += strlen(__SEP__);

    memcpy(&buffer[offset], rec.phone, phone_len);
    offset += phone_len;

    memcpy(&buffer[offset], __END__, strlen(__END__));
    offset += strlen(__END__);

    fwrite(buffer, offset, 1, f);
}

void flist(FILE *f) {
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if(size == 0) {
        printf("no rows found\n");
        return;
    }

    char *content = calloc(1, size + 1);
    if(!content) {
        fprintf(stderr, "flist failed: %s\n", strerror(errno));
        return;
    }

    if(fread(content, size, 1, f) != 1) {
        fprintf(stderr, "flist failed: %s\n", strerror(errno));
        free(content);
        return;
    }

    char name[__NAME_MAX_CAP__] = {0};
    char phone[__PHONE_MAX_CAP__] = {0};
    ErrorCode errcode = ERROR_FILE_NO_ERROR;

    size_t offset = 0;
    size_t len = 0;

    while(len < size) {
        memset(name, 0, __NAME_MAX_CAP__);
        memset(phone, 0, __PHONE_MAX_CAP__);

        offset = nextrec(&content[offset], name, phone, &errcode);
        if(offset == 0) {
            fprintf(stderr, "error: %s\n", ERRORS[errcode]);
            break;
        }

        printf("%s, %s\n", name, phone);
        len += offset;
    }

    free(content);
}

size_t nextrec(char *content, char *name, char *phone, ErrorCode *errcode) {
    size_t len = strlen(content);

    if(len < strlen(__START__)) {
        if(errcode != NULL) *errcode = ERROR_FILE_CORRUPTION;
        return 0;
    }

    size_t offset = strlen(__START__);
    size_t size = 0;

    while(offset + size < len) {
        if(content[offset + size] == ',') break;
        size += 1;
    }

    if(offset + size == len) {
        if(errcode != NULL) *errcode = ERROR_FILE_CORRUPTION;
        return 0;
    }

    if(name != NULL) memcpy(name, &content[offset], size);

    offset += size + 1; // +1 for the __SEP__
    size = 0;

    while(offset + size < len) {
        if(len - (offset + size) >= strlen(__END__)) {
            if(memcmp(&content[offset + size], __END__, strlen(__END__)) == 0) break;
        } else {
            if(errcode != NULL) *errcode = ERROR_FILE_CORRUPTION;
            return 0;
        }

        size += 1;
    }

    if(offset + size == len) {
        if(errcode != NULL) *errcode = ERROR_FILE_CORRUPTION;
        return 0;
    }

    if(phone != NULL) memcpy(phone, &content[offset], size);

    offset += size;

    if(len - offset < strlen(__END__)) {
        if(errcode != NULL) *errcode = ERROR_FILE_CORRUPTION;
        return 0;
    }

    offset += strlen(__END__);
    return offset;
}

char *strtrim(char *s) {
    while (isspace((unsigned char)*s)) s++;

    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end - 1))) end--;

    *end = '\0';
    return s;
}

size_t strsplit(char *s, char *r) {
    size_t i = 0;

    while (s[i] && s[i] != ' ') {
        r[i] = s[i];
        i++;
    }
    r[i] = '\0';

    if (s[i] == '\0')
        return 0;

    return i + 1;
}