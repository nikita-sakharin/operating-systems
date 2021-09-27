#include <assert.h> // assert
#include <limits.h> // ULLONG_MAX
#include <stdbool.h> // bool, false, true
#include <stddef.h> // size_t
#include <stdio.h> // printf

#include <pthread.h> // pthread_mutex_*
#include <unistd.h> // nanosleep

#include "common.h"
#include "philo.h"

#define FORMAT ("%5llu %zu %s\n")

typedef enum {
    FORK,
    EATING,
    SLEEPING,
    THINKING,
    DIED
} Message;

extern inline ullong getMilliTime(void);
extern inline void milliSleep(ullong);
static const char *msgToStr(Message);
static bool testRefreshPrint(Arg * restrict, Message);

static const char *msgToStr(const Message message) {
    switch (message) {
        case FORK:
            return "has taken a fork";
        case EATING:
            return "is eating";
        case SLEEPING:
            return "is sleeping";
        case THINKING:
            return "is thinking";
        case DIED:
            return "died";
        default:
            assert(false);
            return NULL;
    }
}

void *philo(void * const ptr) {
    Arg * const arg = (Arg *) ptr;
    const size_t next = (arg->rank + 1) % arg->size;
    pthread_mutex_t
        * const first = arg->forks + (arg->rank % 2 == 0 ? arg->rank : next),
        * const second = arg->forks + (arg->rank % 2 == 0 ? next : arg->rank);
    for (bool isValid = true; isValid; ) {
        EXIT_IF(pthread_mutex_lock(first) != 0, "pthread_mutex_lock");
        isValid = testRefreshPrint(arg, FORK);
        EXIT_IF(pthread_mutex_lock(second) != 0, "pthread_mutex_lock");
        isValid = (isValid ? testRefreshPrint(arg, FORK) : false);
        isValid = (isValid ? testRefreshPrint(arg, EATING) : false);
        if (isValid)
            milliSleep(arg->timeToEat);
        EXIT_IF(pthread_mutex_unlock(second) != 0, "pthread_mutex_unlock");
        EXIT_IF(pthread_mutex_unlock(first) != 0, "pthread_mutex_unlock");
        isValid = (isValid ? testRefreshPrint(arg, SLEEPING) : false);
        if (isValid)
            milliSleep(arg->timeToSleep);
        isValid = (isValid ? testRefreshPrint(arg, THINKING) : false);
    }
    return NULL;
}

bool testIsValid(const uint timeToDie, Arg * const restrict arg) {
    EXIT_IF(pthread_mutex_lock(arg->shared) != 0, "pthread_mutex_lock");
    const ullong now = getMilliTime();
    const size_t size = arg->size;
    for (size_t i = 0; i < size; ++i)
        if (now - timeToDie >= arg[i].lastMeal) {
            *arg->isDied = true;
            printf(FORMAT, now - arg->start, arg[i].rank + 1, msgToStr(DIED));
            break;
        }
    EXIT_IF(pthread_mutex_unlock(arg->shared) != 0, "pthread_mutex_unlock");
    return !*arg->isDied;
}

static bool testRefreshPrint(Arg * const restrict arg, const Message msg) {
    const ullong now = getMilliTime();
    EXIT_IF(pthread_mutex_lock(arg->shared) != 0, "pthread_mutex_lock");
    bool isValid = !*arg->isDied && arg->mustEat != 0;
    if (isValid) {
        if (msg == EATING) {
            arg->lastMeal = now;
            if (arg->mustEat != -1)
                --arg->mustEat;
            if (arg->mustEat == 0) {
                arg->lastMeal = ULLONG_MAX;
                isValid = false;
            }
        }
        printf(FORMAT, now - arg->start, arg->rank + 1, msgToStr(msg));
    }
    EXIT_IF(pthread_mutex_unlock(arg->shared) != 0, "pthread_mutex_unlock");
    return isValid;
}
