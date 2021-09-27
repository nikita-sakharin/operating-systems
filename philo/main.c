#include <assert.h> // assert
#include <limits.h> // INT_MAX
#include <stdbool.h> // bool, false
#include <stddef.h> // ptrdiff_t, size_t
#include <stdint.h> // PTRDIFF_MAX
#include <stdlib.h> // exit, malloc, strtoll, strtoull

#include <pthread.h> // pthread_*

#include "common.h"
#include "philo.h"

#define DURATION (10)

static void simmulation(size_t, uint, uint, uint, int);
static void simmulation_create(
    size_t,
    pthread_t * restrict * restrict,
    Arg * restrict * restrict,
    pthread_mutex_t * restrict * restrict
);
static void simmulation_destroy(
    size_t,
    pthread_t * restrict,
    Arg * restrict,
    pthread_mutex_t * restrict
);

int main(const int argc, char ** const argv) {
    if (argc < 5 || argc > 6) {
        fprintf(stderr, "Usage: %s NUMBER_OF_PHILOSOPHERS"
            " TIME_TO_DIE TIME_TO_EAT TIME_TO_SLEEP"
            " [NUMBER_OF_TIMES_EACH_PHILOSOPHER_MUST_EAT]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const size_t numberOfPhilosophers = strtoull(argv[1], NULL, 10);
    const ullong timeToDie = strtoull(argv[2], NULL, 10),
        timeToEat = strtoull(argv[3], NULL, 10),
        timeToSleep = strtoull(argv[4], NULL, 10);
    const llong mustEat = (argc == 5 ? -1 : strtoll(argv[5], NULL, 10));
    EXIT_IF(numberOfPhilosophers > PTRDIFF_MAX ||
        timeToDie > INT_MAX || timeToEat > INT_MAX || timeToSleep > INT_MAX ||
        mustEat < -1 || mustEat > INT_MAX,
        "invalid argument(s)"
    );
    simmulation(numberOfPhilosophers,
        (uint) timeToDie, (uint) timeToEat, (uint) timeToSleep, (int) mustEat
    );

    return 0;
}

static void simmulation(
    const size_t size,
    const uint timeToDie,
    const uint timeToEat,
    const uint timeToSleep,
    const int mustEat
) {
    pthread_t * restrict philos;
    Arg * restrict args;
    pthread_mutex_t * restrict forks;
    simmulation_create(size, &philos, &args, &forks);
    const ullong start = getMilliTime();
    bool isDied = false;
    for (size_t i = 0; i < size; ++i) {
        args[i] = (Arg) {
            .start = start, .lastMeal = start, .shared = forks + size,
            .forks = forks, .isDied = &isDied, .rank = i, .size = size,
            .timeToEat = timeToEat, .timeToSleep = timeToSleep,
            .mustEat = mustEat
        };
        EXIT_IF(pthread_create(philos + i, NULL, philo, args + i) != 0,
            "pthread_create"
        );
    }
    while (testIsValid(timeToDie, args))
        milliSleep(DURATION);
    simmulation_destroy(size, philos, args, forks);
}

static void simmulation_create(
    const size_t size,
    pthread_t * restrict * const restrict philos,
    Arg * restrict * const restrict args,
    pthread_mutex_t * restrict * const restrict forks
) {
    *philos = (pthread_t *) malloc(size * sizeof(pthread_t));
    EXIT_IF(*philos == NULL, "malloc");
    *args = (Arg *) malloc(size * sizeof(Arg));
    EXIT_IF(*args == NULL, "malloc");
    *forks = (pthread_mutex_t *) malloc((size + 1) * sizeof(pthread_mutex_t));
    EXIT_IF(*forks == NULL, "malloc");
    for (size_t i = 0; i < size + 1; ++i)
        EXIT_IF(pthread_mutex_init(*forks + i, NULL) != 0,
            "pthread_mutex_init"
        );
}

static void simmulation_destroy(
    const size_t size,
    pthread_t * const restrict philos,
    Arg * const restrict args,
    pthread_mutex_t * const restrict forks
) {
    for (size_t i = 0; i < size; ++i)
        EXIT_IF(pthread_join(philos[i], NULL) != 0, "pthread_join");
    for (size_t i = 0; i < size + 1; ++i)
        EXIT_IF(pthread_mutex_destroy(forks + i) != 0, "pthread_mutex_destroy");
    free(philos);
    free(args);
    free(forks);
}
