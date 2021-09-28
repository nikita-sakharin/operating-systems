#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h> // false
#include <stdio.h> // fprintf, perror, stderr
#include <stdlib.h> // EXIT_FAILURE, exit

#define EXIT_IF(condition, message) \
    do { \
        if (condition) { \
            fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); \
            perror(message); \
            exit(EXIT_FAILURE); \
        } \
    } while (false)

typedef signed char schar;
typedef unsigned char uchar;
typedef short shrt;
typedef unsigned short ushrt;
typedef unsigned uint;
typedef unsigned long ulong;
typedef long long llong;
typedef unsigned long long ullong;
typedef float flt;
typedef double dbl;
typedef long double ldbl;

#endif // COMMON_H
