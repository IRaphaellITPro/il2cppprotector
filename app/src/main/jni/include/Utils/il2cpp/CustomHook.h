#include <iostream>
#include <vector>
#include <cstdio>
#include <cstddef>
#include <cstring>
#include <string.h>
#include "xdl.h"

struct Il2CppDomain;
struct Il2CppAssembly;
struct Il2CppImage;
struct Il2CppClass;
struct Il2CppType;
struct MethodInfo;
struct FieldInfo;
struct Il2CppObject;
struct Il2CppGenericClass;
struct Il2CppGenericMethod;
struct Il2CppGenericContext;
struct Il2CppGenericInst;
struct FieldInfo;

typedef void (*InvokerMethod)(void*, MethodInfo*, void*, void**, void*);

template<typename T>
struct monoArray;

monoArray<void*> *CreateArrayFromClass(Il2CppClass *klass, int size);
Il2CppObject *CreateObjectFromClass(Il2CppClass *klass);
void *ObjectBoxV(Il2CppClass *klass, void *val);

#include "Structure.h"
#include "private_code.h"

void HookInit();

Il2CppImage *GetImage(const char *name);
Il2CppClass *GetClass(const char *image, const char *name);
Il2CppClass *GetClass(const char *image, const char *namespaze, const char *name);

std::vector<Il2CppImage*> GetAllImage();
std::vector<Il2CppClass*> GetAllClasses();
std::vector<MethodInfo*> GetAllMethods();

std::vector<Il2CppClass*> GetAllClassesImage(const char *image_name);
std::vector<MethodInfo*> GetAllMethodsImage(const char *image_name);

void *NewSTR(const char *text);

struct Il2CppImage {
    Il2CppClass *GetClass(const char *name);
    Il2CppClass *GetClass(const char *namespaze, const char *name);
};

struct Il2CppClass : private ClassImpl {
public:
    MethodInfo *GetMethod(const char *name, int count = 0);
    MethodInfo *GetMethod(const char *name, int count, int countParam, const char *nameParam, int countParam2 = 0, const char *nameParam2 = "");
    std::vector<MethodInfo*> GetMethods();
    const char *getName();
    const char *getNamespace();
    Il2CppGenericClass *GetGeneric();
    Il2CppObject *CreateObject();
    Il2CppObject *getType();
    std::vector<void*> FindObjects();
    void *ObjectBox(void *val);
    monoArray<void*> *CreateArray(int size);
    FieldInfo *getField(const char *name);
    void *GetStaticField(const char *name);
    std::vector<FieldInfo*> GetFields();
};

struct Il2CppType : private TypeImpl {
public:
    void *GetDummy();
    Il2CppGenericClass *GetGeneric();
    const char *getName();
    Il2CppClass *GetClass();
    bool isByref();
    Il2CppTypeEnum TypeEnum();
};

struct MethodInfo : private MethodImpl {
public:
    uintptr_t getAddress();
    uintptr_t getVirtualAddress();
    const char *getName();
    Il2CppClass *GetClass();
    Il2CppType *GetReturn();
    int paramCount();
    std::vector<Il2CppType*> GetParams();
    Il2CppObject *RuntimeInvoke(void *obj, void **params, void **exc);
    Il2CppGenericMethod *getGeneric();
    bool isStatic();
};

struct FieldInfo : private FieldInfoImpl {
public:
    size_t getOffset();
    const char *getName();
    Il2CppType *getType();
    int getFlag();
};

struct Il2CppObject : private ObjectImpl {
public:
    Il2CppClass *GetClass();
};

struct Il2CppGenericInst : private GenericInstImpl {
public:
    int Size();
    Il2CppType *GetType(int count);
    std::vector<Il2CppType*> GetTypes();
};

struct Il2CppGenericContext : private GenericContextImpl {
public:
    Il2CppGenericInst *ClassInst();
    Il2CppGenericInst *MethodInst();
};

struct Il2CppGenericClass {
private:
    Il2CppType *type;
    Il2CppGenericContext context;
    Il2CppClass *cached_class;
public:
    Il2CppGenericContext getContext();
    Il2CppClass *getClass();
};

struct Il2CppGenericMethod {
private:
    MethodInfo *methodDefinition;
    Il2CppGenericContext context;
    
public:
    MethodInfo *getDefinition();
    Il2CppGenericContext getContext();
};