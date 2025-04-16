#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Stub for pal_StringNCopy to match signature in lims_wrapper.o
unsigned int pal_StringNCopy(char* dest, const char* src, unsigned int n, unsigned int someOtherParam) {
    printf("Stub: pal_StringNCopy('%s', '%s', %u, %u)\n", dest, src, n, someOtherParam);
    strncpy(dest, src, n);
    return n;
}

// Stub for pal_StringNConcatenate to match signature in lims_wrapper.o
unsigned int pal_StringNConcatenate(char* dest, const char* src, unsigned int n, unsigned int someOtherParam) {
    printf("Stub: pal_StringNConcatenate('%s', '%s', %u, %u)\n", dest, src, n, someOtherParam);
    strncat(dest, src, n);
    return n;
}

// Stub for pal_UtilityRandomNumber
unsigned int pal_UtilityRandomNumber() {
    printf("Stub: pal_UtilityRandomNumber()\n");
    return 12345;  // Example random number
}

// Stub for pal_MutexLock
unsigned int pal_MutexLock(void* mutexHandle) {
    printf("Stub: pal_MutexLock()\n");
    return 0;  // Success
}

// Stub for pal_MutexUnlock
unsigned int pal_MutexUnlock(void* mutexHandle) {
    printf("Stub: pal_MutexUnlock()\n");
    return 0;  // Success
}

// Stub for pal_MemoryFree
void* pal_MemoryFree(void* ptr) {
    printf("Stub: pal_MemoryFree()\n");
    free(ptr);  // Free memory
    return NULL;  // Return NULL to indicate memory has been freed
}

// Stub for pal_LogInit
unsigned int pal_LogInit() {
    printf("Stub: pal_LogInit()\n");
    return 0;  // Success
}

// Stub for pal_Init
unsigned int pal_Init() {
    printf("Stub: pal_Init()\n");
    return 0;  // Success
}

// Stub for pal_Deinit
unsigned int pal_Deinit() {
    printf("Stub: pal_Deinit()\n");
    return 0;  // Success
}

// Stub for pal_SocketSetDeviceName
unsigned int pal_SocketSetDeviceName(const char* deviceName) {
    printf("Stub: pal_SocketSetDeviceName('%s')\n", deviceName);
    return 0;  // Success
}

// Stub for pal_LogDeinit
unsigned int pal_LogDeinit() {
    printf("Stub: pal_LogDeinit()\n");
    return 0;  // Success
}

// Stub for pal_StringLength
unsigned int pal_StringLength(const char* str) {
    printf("Stub: pal_StringLength('%s')\n", str);
    return strlen(str);  // Return string length
}
