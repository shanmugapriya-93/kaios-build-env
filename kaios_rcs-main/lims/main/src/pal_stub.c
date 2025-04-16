// pal_stub.c
#include <stdio.h>

typedef void* KPALHandle;
typedef void* KMutexHandle;
typedef void* KLogHandle;

unsigned int pal_Init(KLogHandle logHandle, KPALHandle* palHandle) {
    printf("Stub: pal_Init()\n");
    *palHandle = (void*)0x1; // fake handle
    return 0; // success
}

unsigned int pal_Deinit(KPALHandle palHandle) {
    printf("Stub: pal_Deinit()\n");
    return 0;
}

KLogHandle pal_LogInit(void* filename, int output, int mask, int level, int component, int type) {
    printf("Stub: pal_LogInit()\n");
    return (KLogHandle)0x3; // fake log handle
}

unsigned int pal_LogDeinit(KLogHandle* logHandle) {
    printf("Stub: pal_LogDeinit()\n");
    return 0;
}

unsigned int pal_SocketSetDeviceName(KPALHandle palHandle, const char* name) {
    printf("Stub: pal_SocketSetDeviceName('%s')\n", name);
    return 0;
}

unsigned int pal_MutexCreate(KPALHandle palHandle, KMutexHandle* mutexHandle) {
    printf("Stub: pal_MutexCreate()\n");
    *mutexHandle = (void*)0x2; // fake mutex handle
    return 0;
}

unsigned int pal_MutexLock(KMutexHandle mutexHandle) {
    printf("Stub: pal_MutexLock()\n");
    return 0;
}

unsigned int pal_MutexUnlock(KMutexHandle mutexHandle) {
    printf("Stub: pal_MutexUnlock()\n");
    return 0;
}

void pal_MemoryFree(void* ptr) {
    printf("Stub: pal_MemoryFree()\n");
    // No operation
}

unsigned int pal_UtilityRandomNumber() {
    printf("Stub: pal_UtilityRandomNumber()\n");
    return 42; // fixed dummy value
}

