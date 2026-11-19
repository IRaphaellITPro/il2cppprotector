#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <limits.h>
#include <inttypes.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#include <cstring>
#include <mutex>
#include <algorithm>
#include <cstdint>
#include "dobby.h"

using namespace std;

uintptr_t libBaseAddress = 0;

uintptr_t findLibrary(const char *library) {
    char filename[0xFF] = {0},
            buffer[1024] = {0};
    FILE *fp = NULL;
    uintptr_t address = 0, start = 0, end = 0;
    char flags[7], path[PATH_MAX];
    pid_t pid = getpid();
    sprintf(filename, "/proc/%d/maps", pid);
    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        goto done;
    }
    while (fgets(buffer, sizeof(buffer), fp)) {
        strcpy(path, "");
        sscanf(buffer, "%" PRIxPTR "-%" PRIxPTR " %s %*x %*x:%*x %*u %s\n", &start, &end, flags, path);
        if (strstr(flags, "r-xp") == 0) continue;
        if (strstr(path, library)) {
            address = start;
            goto done;
        }
    }
    done:
    if (fp) {
        fclose(fp);
    }
    return address;
}

bool isLibraryLoaded(const char *libraryName) {
    char line[512] = {0};
    FILE *fp = fopen("/proc/self/maps", "rt");
    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) {
            std::string a = line;
            if (strstr(line, libraryName)) {
                return true;
            }
        }
        fclose(fp);
    }
    return false;
}

const char *GetPackageName() {
    char *id[256];
    FILE *fp = fopen("proc/self/cmdline", "r");
    if (fp) {
        fread(id, sizeof(id), 1, fp);
        fclose(fp);
    }
    return (const char*)id;
}

bool CreateFolderMain() {
    const char *pkg = GetPackageName();
    string dir = "/storage/emulated/0/Android/data/" + string(pkg) + "/Lewis";
    int ret = mkdir(dir.c_str(), 0777);
    if (ret == 0) {
        return true;
    }
    return false;
}
bool CreateFolder(const char *name) {
    const char *pkg = GetPackageName();
    string dir = "/storage/emulated/0/Android/data/" + string(pkg) + "/Lewis/" + string(name);
    int ret = mkdir(dir.c_str(), 0777);
    if (ret == 0) {
        return true;
    }
    return false;
}
bool CreateFolder2(const char *name) {
    const char *pkg = GetPackageName();
    string dir = "/storage/emulated/0/Android/data/" + string(pkg) + "/Lewis/" + string(name);
    string temp;
    for (size_t i = 0; i < dir.size(); ++i) {
        temp += dir[i];
        if (dir[i] == '/' && temp.size() > 1) {
            mkdir(temp.c_str(), 0777);
        }
    }
    if (mkdir(dir.c_str(), 0777) != 0) {
        return false;
    }
    return true;
}

uintptr_t getAbsoluteAddress(uintptr_t addr) {
    if (addr == 0)
        return 0;
    return (libBaseAddress + addr);
}

#define LIB "libil2cpp.so"
#define gAA getAbsoluteAddress
#define DHOOK(offset, ptr, orig) DobbyHook((void*)gAA(offset), (void*)ptr, (void **)&orig)
#define MHOOK(offset, ptr, orig) DobbyHook((void*)offset, (void*)ptr, (void **)&orig)
#define RET      "C0 03 5F D6"

#define TRUE    "20 00 80 52 C0 03 5F D6"
#define FALSE   "00 00 80 52 C0 03 5F D6"

#define NUM1    "20 00 80 52 C0 03 5F D6"
#define NUM2    "40 00 80 52 C0 03 5F D6"
#define NUM7    "E0 00 80 52 C0 03 5F D6"
#define NUM10   "40 01 80 52 C0 03 5F D6"
#define NUM15   "E0 01 80 52 C0 03 5F D6"
#define NUM17   "20 02 80 52 C0 03 5F D6"

#define INT50    "A0 06 80 52 C0 03 5F D6"
#define INT255   "E0 1F 80 52 C0 03 5F D6"
#define INT999   "E7 07 80 52 C0 03 5F D6"
#define INT65500 "BC FF 81 52 C0 03 5F D6"

#define oHex MemoryPatch::createWithHex
#define MOD Modify