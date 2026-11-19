#include "Utils.h"
#include "Unity.h"
#include "CustomHook.h"

using namespace std;

void (*_LogWriter$$AddLog)(...);
void LogWriter$$AddLog(void *inst, MonoString *log, MonoString *stackTrace, bool needTime) {
    return;
    //return _LogWriter$$AddLog(inst, log, stackTrace, needTime);
}

void Hook() {
    
} 
