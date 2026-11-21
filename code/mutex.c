#include "platform_thread.h"

#include <errno.h>

int pt_mutex_init(pt_mutex_t *mutex)
{
    if (mutex == NULL) {
#ifdef _WIN32
        return (int)ERROR_INVALID_PARAMETER;
#else
        return EINVAL;
#endif
    }

#ifdef _WIN32
    mutex->handle = CreateMutex(NULL, FALSE, NULL);
    if (mutex->handle == NULL) {
        return (int)GetLastError();
    }
    return 0;
#else
    return pthread_mutex_init(&mutex->handle, NULL);
#endif
}