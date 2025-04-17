#include <stdio.h>
#include <stdint.h>

// Dummy log handle and PAL instance types
typedef void* PALHANDLE;
typedef void* LOGHANDLE;

unsigned int pal_Init(LOGHANDLE logHandle, PALHANDLE* palInstance) {
    if (logHandle == NULL) {
        printf("[ERROR] Log handle is NULL\n");
        return 1;  // Error code
    }
    printf("[STUB] pal_Init called and returning success\n");
    *palInstance = (PALHANDLE)(uintptr_t)0x9999;  // Assign a dummy non-null pointer
    return 0;  // 0 = success
}
LOGHANDLE pal_LogInit(void* fileName, int outputType, unsigned int mask, int level, int component, int type) {
    printf("[STUB] pal_LogInit called and returning dummy handle\n");
    return (LOGHANDLE)(uintptr_t)0x8888;  // Assign dummy handle
}

unsigned int pal_LogDeinit(LOGHANDLE* logHandle) {
    printf("[STUB] pal_LogDeinit called\n");
    *logHandle = NULL;
    return 0;
}
