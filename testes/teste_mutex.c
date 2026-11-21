#include "../code/platform_thread.h"

#include <stdio.h>

#define THREAD_COUNT 4
#define INCREMENTS_PER_THREAD 10000

typedef struct {
    pt_mutex_t mutex;
    unsigned long counter;
} shared_data_t;

static void increment_counter(void *argument)
{
    shared_data_t *shared_data = (shared_data_t *)argument;
    int i;

    for (i = 0; i < INCREMENTS_PER_THREAD; i++) {
        int error = pt_mutex_lock(&shared_data->mutex);
        if (error != 0) {
            fprintf(stderr, "pt_mutex_lock falhou: %d\n", error);
            return;
        }

        shared_data->counter++;

        error = pt_mutex_unlock(&shared_data->mutex);
        if (error != 0) {
            fprintf(stderr, "pt_mutex_unlock falhou: %d\n", error);
            return;
        }
    }
}

int main(void)
{
    pt_thread_t threads[THREAD_COUNT];
    shared_data_t shared_data = { 0 };
    int created_threads = 0;
    int i;
    int error;

    error = pt_mutex_init(&shared_data.mutex);
    if (error != 0) {
        fprintf(stderr, "pt_mutex_init falhou: %d\n", error);
        return 1;
    }

    for (i = 0; i < THREAD_COUNT; i++) {
        error = pt_thread_create(&threads[i], increment_counter, &shared_data);
        if (error != 0) {
            fprintf(stderr, "pt_thread_create falhou: %d\n", error);
            break;
        }
        created_threads++;
    }

    for (i = 0; i < created_threads; i++) {
        error = pt_thread_join(&threads[i]);
        if (error != 0) {
            fprintf(stderr, "pt_thread_join falhou: %d\n", error);
            return 1;
        }
    }

    error = pt_mutex_destroy(&shared_data.mutex);
    if (error != 0) {
        fprintf(stderr, "pt_mutex_destroy falhou: %d\n", error);
        return 1;
    }

    if (created_threads != THREAD_COUNT
        || shared_data.counter != THREAD_COUNT * INCREMENTS_PER_THREAD) {
        fprintf(stderr, "Resultado incorreto: %lu incrementos\n", shared_data.counter);
        return 1;
    }

    printf("Sucesso: %lu incrementos protegidos por mutex.\n", shared_data.counter);
    return 0;
}
