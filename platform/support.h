#ifndef SUPPORT_H
#define SUPPORT_H

#define APP_VERSION       1.05
#define APP_VERSION_MAJOR 1
#define APP_VERSION_MINOR 0.05

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>


// Errors and logging

void print_err(const char *fmt, ...);
void print_bug_message(FILE *stream);
void print_bug(const char *fmt, ...);

void print_matrix(FILE *stream, const char *name, float matrix[]);
void hex_dump(FILE *stream, void *mem, int  sz);


/// Memory

char * strfindreplace(char *str, const char *find, const char *replace);
char *strremove(char *str, const char *sub);


// I/O

bool exec_cmd(const char *cmd, char *result, int result_size);
char* read_file(char* path, int* len);
char *cat_file(const char *path);

// Hardware

const char *get_build();

#endif // SUPPORT_H
