/*
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║                  JEB PROTECTOR — main.cpp                        ║
 * ║         Join Telegram ➜ @Jebwuax  for updates & tools           ║
 * ╚══════════════════════════════════════════════════════════════════╝
 */
#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <string>

#include "xdl.h"
#include "dobby.h"
#include "KittyMemory.h"
#include "KittyScanner.h"
#include "MemoryPatch.h"
#include "Hook.h"

/* ──────────────────────────────────────────────────────────────────
 *  Main hack thread
 *  Waits for the target library to be mapped, then runs Hook().
 * ────────────────────────────────────────────────────────────────── */
static void *hack_thread(void *) {
    /* Spin until the target library base address is found */
    do {
        libBaseAddress = findLibrary(LIB);
        if (!libBaseAddress) usleep(50000); /* 50 ms */
    } while (libBaseAddress == 0);

    Hook();
    pthread_exit(nullptr); 
    return nullptr;
}

/* ──────────────────────────────────────────────────────────────────
 *  Library constructor — spawns the hack thread.
 *  il2cpp_wrapper.cpp has its own constructor that runs first and
 *  handles decryption / loading of the real il2cpp and hack payloads.
 * ────────────────────────────────────────────────────────────────── */
__attribute__((constructor))
static void lib_main() {
    pthread_t tid;
    pthread_create(&tid, nullptr, hack_thread, nullptr);
    pthread_detach(tid);
}

/* ──────────────────────────────────────────────────────────────────
 *  JNI_OnLoad — required so the .so is treated as a JNI library.
 * ────────────────────────────────────────────────────────────────── */
extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void * /*reserved*/) {
    JNIEnv *env = nullptr;
    vm->GetEnv((void **)&env, JNI_VERSION_1_6);
    return JNI_VERSION_1_6;
}
