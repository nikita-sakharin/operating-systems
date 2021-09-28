#ifndef PHILO_H
#define PHILO_H

#include <stdbool.h> // bool
#include <stddef.h> // NULL, size_t

#include <pthread.h> // pthread_mutex_t
#include <sys/time.h> // gettimeofday, timeval

#include "common.h"

typedef struct {
    ullong start, lastMeal;
    pthread_mutex_t *shared, *forks;
    bool *isDied;
    size_t rank, size;
    uint timeToEat, timeToSleep;
    int mustEat;
} Arg;

inline ullong getMilliTime(void) {
    struct timeval now;
    EXIT_IF(gettimeofday(&now, NULL) != 0, "gettimeofday");
    return (ullong) now.tv_sec * 1000 + (ullong) now.tv_usec / 1000;
}

inline void milliSleep(const ullong milliseconds) {
    const struct timespec duration = {
        .tv_sec = (llong) milliseconds / 1000,
        .tv_nsec = (llong) milliseconds % 1000 * 1000000
    };
    EXIT_IF(nanosleep(&duration, NULL) != 0, "nanosleep");
}

extern void *philo(void *);
extern bool testIsValid(uint, Arg * restrict);

#endif // PHILO_H
