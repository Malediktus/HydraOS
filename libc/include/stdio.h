#ifndef _STDIO_H
#define _STDIO_H 1

/*
    https://pubs.opengroup.org/onlinepubs/7908799/xsh/stdio.h.html
*/

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

typedef uint64_t FILE;

extern uint64_t _stdin;
extern uint64_t _stdout;
extern uint64_t _stderr;

#define stdin ((FILE *)&_stdin)
#define stdout ((FILE *)&_stdout)
#define stderr ((FILE *)&_stderr)

int fgetc(FILE *f);
char *fgets(char *s, int c, FILE *f);
int fputc(char c, FILE *f);
int fputs(const char *st, FILE *f);
size_t fread(char *s, size_t n, size_t u, FILE *f);
size_t fwrite(const char *st, size_t n, size_t u, FILE *f);
char *gets(char *s);
int putc(char c, FILE *f);
int putchar(char c);
int printf(const char *st, ...);
int fprintf(FILE *f, const char *st, ...);
int vfprintf(FILE *f, const char *st, va_list va);
int snprintf(char *s, size_t n, const char *ct, ...);
int sprintf(char *s, const char *ct, ...);
int vprintf(const char *st, va_list va);
int vsnprintf(char *s, size_t n, const char *ct, va_list va);
int vsprintf(char *s, const char *ct, va_list);

#endif