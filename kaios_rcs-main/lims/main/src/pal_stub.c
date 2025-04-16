#include <stdio.h>
#include <string.h>  // For string manipulation functions like strncpy, strncat

typedef void* KPALHandle;
typedef void* KMutexHandle;
typedef void* KLogHandle;

unsigned int pal_Init(KLogHandle logHandle, KPALHandle* palHandle) {
    printf("Stub: pal_Init()\n");
    *palHandle = (void*)0x1; // fake handle
    return 0; // success
}

void pal_Deinit(KPALHandle palHandle) {
    printf("Stub: pal_Deinit()\n");
}

void pal_LogDeinit(KLogHandle* logHandle) {
    printf("Stub: pal_LogDeinit()\n");
}

unsigned int pal_MutexCreate(KPALHandle palHandle, KMutexHandle* mutexHandle) {
    printf("Stub: pal_MutexCreate()\n");
    *mutexHandle = (void*)0x2;
    return 0;
}

unsigned int pal_SocketSetDeviceName(KPALHandle palHandle, const char* name) {
    printf("Stub: pal_SocketSetDeviceName('%s')\n", name);
    return 0;
}

KLogHandle pal_LogInit(void* filename, int output, int mask, int level, int component, int type) {
    printf("Stub: pal_LogInit()\n");
    return (KLogHandle)0x3;
}

// Stub functions for string operations
unsigned int pal_StringLength(const char* str) {
    printf("Stub: pal_StringLength('%s')\n", str);
    return (unsigned int)strlen(str);
}

unsigned int pal_StringNCopy(char* dest, const char* src, unsigned int n) {
    printf("Stub: pal_StringNCopy('%s', '%s', %u)\n", dest, src, n);
    strncpy(dest, src, n);
    return n;
}

unsigned int pal_StringNConcatenate(char* dest, const char* src, unsigned int n) {
    printf("Stub: pal_StringNConcatenate('%s', '%s', %u)\n", dest, src, n);
    strncat(dest, src, n);
    return n;
}
