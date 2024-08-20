// Reference:
// ----------
// 1. https://github.com/legatoproject/legato-3rdParty-libiio/blob/master/local.c

#include "support.h"
#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
#elif defined __linux__
  #define _GNU_SOURCE
  #include <sched.h>
#elif defined __FreeBSD__
  #include <sys/param.h>
  #include <sys/cpuset.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <assert.h>


#ifdef _WIN32

#define RED   ""
#define BOLD  ""
#define RESET ""

#else

#define RED "\x1b[31;1m"
#define BOLD "\x1b[;1m"
#define RESET "\x1b[0m"

#endif


#ifdef ARCH_X86
  static const char* ARCH_STR = "x86 / x86_64 build";
  #include "../x86/cpuid.h"
#elif ARCH_PPC
  static const char* ARCH_STR = "PowerPC build";
  #include "../ppc/ppc.h"
#elif ARCH_ARM
  static const char* ARCH_STR = "ARM build";
  #include "cpu_arm_midr.h"
#elif ARCH_RISCV
  static const char* ARCH_STR = "RISC-V build";
  #include "cpu_riscv.h"
#endif

#ifdef __linux__
  #ifdef __ANDROID__
    static const char* OS_STR = "Android";
  #else
    static const char* OS_STR = "Linux";
  #endif
#elif __FreeBSD__
  static const char* OS_STR = "FreeBSD";
#elif _WIN32
  static const char* OS_STR = "Windows";
#elif defined __APPLE__ || __MACH__
  static const char* OS_STR = "macOS";
#else
  static const char* OS_STR = "Unknown OS";
#endif

#define _S(v) #v

#ifndef GIT_FULL_VERSION
  static const char* VERSION = _S(APP_VERSION);
#endif

/// Info

void print_version(FILE *restrict stream) {
#ifdef GIT_FULL_VERSION
    fprintf(stream, "backboard %s (%s %s)\n", GIT_FULL_VERSION, OS_STR, ARCH_STR);
#else
    fprintf(stream, "backboard v%s (%s %s)\n", VERSION, OS_STR, ARCH_STR);
#endif
}

/// Errors and logging

void print_err(const char *fmt, ...) {
    int buffer_size = 4096;
    char buffer[buffer_size];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer,buffer_size, fmt, args);
    va_end(args);
    fprintf(stderr,RED "[ERROR]: "RESET "%s\n",buffer);
    fprintf(stderr,"[VERSION]: ");
    print_version(stderr);
}

void print_bug_message(FILE *restrict stream) {
#if defined(ARCH_X86) || defined(ARCH_PPC)
    fprintf(stream, "Please, create a new issue with this error message, the output of 'cpufetch' and 'cpufetch --debug' on https://github.com/Dr-Noob/cpufetch/issues\n");
#elif ARCH_ARM
    fprintf(stream, "Please, create a new issue with this error message, your smartphone/computer model, the output of 'cpufetch --verbose' and 'cpufetch --debug' on https://github.com/Dr-Noob/cpufetch/issues\n");
#endif
}

void print_bug(const char *fmt, ...) {
    int buffer_size = 4096;
    char buffer[buffer_size];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer,buffer_size, fmt, args);
    va_end(args);
    fprintf(stderr,RED "[ERROR]: "RESET "%s\n",buffer);
    fprintf(stderr,"[VERSION]: ");
    print_version(stderr);
    print_bug_message(stderr);
}


// print 4x4 "matrix[]" in standard format
void print_matrix(FILE *restrict stream, const char *name, float matrix[]) {
    fprintf(stream, "Matrix : %s\n", name);
    for (int i = 0; i < 4; i++) {
        fprintf(stream,
            "%3.2f  %3.2f  %3.2f  %3.2f \n",
            matrix[i], matrix[i+4], matrix[i+8], matrix[i+12]);
    }
}

void hex_dump(FILE *restrict stream, void *mem, int  sz) {
    char printbuf[100];
    int indent = 1, index, rindex, relpos;
    struct { unsigned char *data; int sz; } buf;
    unsigned char *addr = (unsigned char *)mem;

    buf.data = (unsigned char *)addr;
    buf.sz   = sz;

    while (buf.sz > 0) {
        unsigned char *tmp = (unsigned char *)buf.data;
        int outlen  = (int)buf.sz;
        if (outlen > 16) outlen = 16;
        // create a 64-character formatted output line:
        sprintf(printbuf, " >                                                      %p", (void*)(tmp-addr));
        int outpos = outlen;
        for(index = 1+indent, rindex = 53-15+indent, relpos = 0;
            outpos;
            outpos--, index += 2, rindex++)
        {
            unsigned char uc = *tmp++;
            snprintf(printbuf + index, 100, "%02X ", (unsigned short)uc);
            if (!isprint(uc)) uc = '.'; // nonprintable char
            printbuf[rindex] = uc;
            if (!(++relpos & 3)) { // extra blank after 4 bytes
                index++; printbuf[index+2] = ' ';
            }
        }

        if (!(relpos & 3)) index--;

        printbuf[index]   = '<'; printbuf[index+1]   = ' ';
        fprintf(stream, "%s\n", printbuf);
        buf.data += outlen; buf.sz -= outlen;
    }
}

/// Memory

char *strfindreplace(char *str, const char *find, const char *replace) {
    assert(strlen(replace) <= strlen(find));

    unsigned i, j, k, n, m;

    i = j = m = n = 0;

    while (str[i] != '\0') {
        if (str[m] == find[n]) {
            m++;
            n++;
            if (find[n] == '\0') {
                for (k = 0; replace[k] != '\0'; k++, j++) {
                    str[j] = replace[k];
                }
                n = 0;
                i = m;
            }
        } else {
            str[j] = str[i];
            j++;
            i++;
            m = i;
            n = 0;
        }
    }

    for (; j < i; j++) {
        str[j] = '\0';
    }

    return str;
}

char *strremove(char *str, const char *sub) {
    char *p, *q, *r;
    if (*sub && (q = r = strstr(str, sub)) != NULL) {
        size_t len = strlen(sub);
        while ((r = strstr(p = r + len, sub)) != NULL) {
            memmove(q, p, r - p);
            q += r - p;
        }
        memmove(q, p, strlen(p) + 1);
    }
    return str;
}

void *emalloc(size_t size) {
    void* ptr = malloc(size);

    if(ptr == NULL) {
        print_err("malloc failed: %s", strerror(errno));
        exit(1);
    }

    return ptr;
}

void *ecalloc(size_t nmemb, size_t size) {
    void *ptr = calloc(nmemb, size);

    if (ptr == NULL) {
        print_err("calloc failed: %s", strerror(errno));
        exit(1);
    }

    return ptr;
}

/// I/O

bool exec_cmd(const char *cmd, char *result, int result_size) {
    FILE *fp = popen(cmd, "r");

    if (fgets(result, result_size - 1, fp) == 0) {
        pclose(fp);
        return false;
    }

    size_t len = strlen(result);

    if (result[len - 1] < ' ') {
        result[len - 1] = '\0';
    }

    pclose(fp);
    return true;
}

char *read_file(char *path, int *len) {
    int fd = open(path, O_RDONLY);

    if(fd == -1) {
        return NULL;
    }

    // File exists, read it
    int bytes_read = 0;
    int offset = 0;
    int block = 1024;
    int buf_size = block * 4;
    char* buf = emalloc(sizeof(char) *buf_size);

    while ((bytes_read = read(fd, buf + offset, block)) > 0) {
        offset += bytes_read;
        if(offset + block > buf_size) {
            buf = erealloc(buf, sizeof(char) * (buf_size + block));
            buf_size += block;
        }
    }
    buf[offset] = '\0';

    if (close(fd) == -1) {
        return NULL;
    }

    *len = offset;
    return buf;
}

#define BUF_SIZE 128

char *cat_file(const char *path) {
    char buf[BUF_SIZE];

    FILE *f = fopen(path, "re");
    if (!f)
        return NULL;

    ssize_t ret = fread(buf, 1, sizeof(buf)-1, f);
    fclose(f);
    if (ret > 0)
        buf[ret - 1] = '\0';
    else
        return NULL;

    return strndup(buf, sizeof(buf) - 1);
}
