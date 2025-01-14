// there error functions are from the book 'The Linux Programming Interface' by Michael Kerrisk

#ifndef ERROR_FUNCTIONS_H
#define ERROR_FUNCTIONS_H

void errMsg(const char *format, ...);

#ifdef __GNUC__
/* This macro stops 'gcc -Wall' from complaing that "control reaches end of non-void function" if we use the following functions to terminate another one*/

#define NORETURN __attribute__ ((__noreturn__))
#else
#define NORETURN 
#endif

void errExit(const char* format, ...) NORETURN ;
void err_exit(const char* format, ...) NORETURN ;
void errExitEN(int errnum, const char* format, ...) NORETURN ;
void fatal(const char* format, ...) NORETURN ;
void usageErr(const char* format, ...) NORETURN ;
void cmdLineErr(const char* format, ...) NORETURN ;

#endif
