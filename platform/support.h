#ifndef SUPPORT_H
#define SUPPORT_H

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
# define EXTERN_C extern "C"
#else
# define EXTERN_C
#endif

# define f_ssprintf(...)                                \
    ({ int _ss_size = snprintf(0, 0, ##__VA_ARGS__);    \
    char *_ss_ret = (char*)alloca(_ss_size+1);          \
    snprintf(_ss_ret, _ss_size+1, ##__VA_ARGS__);       \
    _ss_ret; })

/// Errors and logging

EXTERN_C void print_err(const char *fmt, ...);
EXTERN_C void print_bug_message(FILE *stream);
EXTERN_C void print_bug(const char *fmt, ...);

EXTERN_C void print_matrix(FILE *stream, const char *name, float matrix[]);
EXTERN_C void hex_dump(FILE *stream, void *mem, int  sz);

/// Memory

EXTERN_C char *strfindreplace(char *str, const char *find, const char *replace);
EXTERN_C char *strremove(char *str, const char *sub);

/// I/O

EXTERN_C bool exec_cmd(const char *cmd, char *result, int result_size);
EXTERN_C char *read_file(char* path, int* len);
EXTERN_C char *cat_file(const char *path);

/// Hardware

EXTERN_C const char *get_build();

#endif // SUPPORT_H
