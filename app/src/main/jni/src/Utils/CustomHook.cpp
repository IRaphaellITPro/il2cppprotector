#include "CustomHook.h"

Il2CppDomain *(*il2cpp_domain_get)();
Il2CppAssembly *(*il2cpp_domain_assembly_open)(Il2CppDomain*, const char*);
Il2CppAssembly **(*il2cpp_domain_get_assemblies)(Il2CppDomain*, size_t*);
Il2CppImage *(*il2cpp_assembly_get_image)(Il2CppAssembly*);
Il2CppClass *(*il2cpp_class_from_name)(Il2CppImage*, const char*, const char*);
MethodInfo *(*il2cpp_class_get_method_from_name)(Il2CppClass*, const char*, int);
const char *(*il2cpp_type_get_name)(Il2CppType*);
Il2CppClass *(*il2cpp_class_from_type)(Il2CppType*);
Il2CppObject *(*il2cpp_object_new)(Il2CppClass*);
int (*il2cpp_method_get_param_count)(MethodInfo*);
Il2CppObject *(*il2cpp_runtime_invoke)(MethodInfo*, void*, void**, void**);
MethodInfo *(*il2cpp_class_get_methods)(Il2CppClass*, void**);
Il2CppType *(*il2cpp_class_get_type)(Il2CppClass*);
Il2CppObject *(*il2cpp_type_get_object)(Il2CppType*);
FieldInfo *(*il2cpp_class_get_field_from_name)(Il2CppClass*, const char*);
void (*il2cpp_field_static_get_value)(FieldInfo*, void*);
FieldInfo *(*il2cpp_class_get_fields)(Il2CppClass*, void**);
size_t (*il2cpp_field_get_offset)(FieldInfo*);
int (*il2cpp_method_get_flags)(MethodInfo*, int*);
size_t (*il2cpp_image_get_class_count)(Il2CppImage*);
Il2CppClass *(*il2cpp_image_get_class)(Il2CppImage*, size_t);
bool (*il2cpp_type_is_byref)(Il2CppType*);
int (*il2cpp_field_get_flags)(FieldInfo*);
const char *(*il2cpp_field_get_name)(FieldInfo*);

typedef void (*il2cpp_register_object_callback)(void **arr, int size, void *userdata);
typedef void *(*il2cpp_liveness_reallocate_callback)(void *ptr, size_t size, void *userdata);

void *(*il2cpp_unity_liveness_allocate_struct)(void *filter, int max_object_count, il2cpp_register_object_callback callback, void *userdata, il2cpp_liveness_reallocate_callback reallocate);
void (*il2cpp_unity_liveness_calculation_from_statics)(void *state);
void (*il2cpp_unity_liveness_finalize)(void *state);
void (*il2cpp_unity_liveness_free_struct)(void *state);
void (*il2cpp_stop_gc_world)();
void (*il2cpp_start_gc_world)();
void (*il2cpp_free)(void *ptr);
void *(*il2cpp_alloc)(size_t size);
void *(*il2cpp_string_new)(const char*);
monoArray<void*> *(*il2cpp_array_new)(Il2CppClass*, int);
void *(*il2cpp_value_box)(Il2CppClass*, void*);

void *NewSTR(const char *text) {
    return il2cpp_string_new(text);
}

const char *GetPKG() {
    static char id[256] = {0};
    FILE *fp = fopen("/proc/self/cmdline", "r");
    if (fp) {
        fread(id, 1, sizeof(id) - 1, fp);
        fclose(fp);
    }
    return id;
}

void HookInit() {
    void *handle = NULL;

    if (std::strcmp(GetPKG(), "com.herogame.gplay.lastdayrulessurvival") == 0) {
        handle = xdl_open("/data/data/com.herogame.gplay.lastdayrulessurvival/files/libil2cpp_real.so", XDL_DEFAULT);
    } else if (std::strcmp(GetPKG(), "com.yingxiong.hero.f399") == 0) {
        handle = xdl_open("/data/data/com.yingxiong.hero.f399/files/libil2cpp.so", XDL_DEFAULT);
    } else if (std::strcmp(GetPKG(), "com.yingxiong.gw.Android.hero") == 0) {
        handle = xdl_open("/data/data/com.yingxiong.gw.Android.hero/files/libil2cpp.so", XDL_DEFAULT);
    } else {
        handle = xdl_open("libil2cpp.so", XDL_DEFAULT);
    }
    
    il2cpp_domain_get = (Il2CppDomain *(*)()) xdl_sym(handle, "il2cpp_domain_get", nullptr);
    il2cpp_domain_assembly_open = (Il2CppAssembly *(*)(Il2CppDomain*, const char*)) xdl_sym(handle, "il2cpp_domain_assembly_open", nullptr);
    il2cpp_domain_get_assemblies = (Il2CppAssembly **(*)(Il2CppDomain*, size_t*)) xdl_sym(handle, "il2cpp_domain_get_assemblies", nullptr);
    il2cpp_assembly_get_image = (Il2CppImage *(*)(Il2CppAssembly*)) xdl_sym(handle, "il2cpp_assembly_get_image", nullptr);
    il2cpp_class_from_name = (Il2CppClass *(*)(Il2CppImage*, const char*, const char*)) xdl_sym(handle, "il2cpp_class_from_name", nullptr);
    il2cpp_class_get_method_from_name = (MethodInfo *(*)(Il2CppClass*, const char*, int)) xdl_sym(handle, "il2cpp_class_get_method_from_name", nullptr);
    il2cpp_type_get_name = (const char *(*)(Il2CppType*)) xdl_sym(handle, "il2cpp_type_get_name", nullptr);
    il2cpp_class_from_type = (Il2CppClass *(*)(Il2CppType*)) xdl_sym(handle, "il2cpp_class_from_type", nullptr);
    il2cpp_object_new = (Il2CppObject *(*)(Il2CppClass*)) xdl_sym(handle, "il2cpp_object_new", nullptr);
    il2cpp_method_get_param_count = (int (*)(MethodInfo*)) xdl_sym(handle, "il2cpp_method_get_param_count", nullptr);
    il2cpp_runtime_invoke = (Il2CppObject *(*)(MethodInfo*, void*, void**, void**)) xdl_sym(handle, "il2cpp_runtime_invoke", nullptr);
    il2cpp_class_get_methods = (MethodInfo *(*)(Il2CppClass*, void**)) xdl_sym(handle, "il2cpp_class_get_methods", nullptr);
    il2cpp_class_get_type = (Il2CppType *(*)(Il2CppClass*)) xdl_sym(handle, "il2cpp_class_get_type", nullptr);
    il2cpp_type_get_object = (Il2CppObject *(*)(Il2CppType*)) xdl_sym(handle, "il2cpp_type_get_object", nullptr);
    il2cpp_class_get_field_from_name = (FieldInfo *(*)(Il2CppClass*, const char*)) xdl_sym(handle, "il2cpp_class_get_field_from_name", nullptr);
    il2cpp_field_static_get_value = (void (*)(FieldInfo*, void*)) xdl_sym(handle, "il2cpp_field_static_get_value", nullptr);
    il2cpp_class_get_fields = (FieldInfo *(*)(Il2CppClass*, void**)) xdl_sym(handle, "il2cpp_class_get_fields", nullptr);
    il2cpp_field_get_offset = (size_t (*)(FieldInfo*)) xdl_sym(handle, "il2cpp_field_get_offset", nullptr);
    il2cpp_method_get_flags = (int (*)(MethodInfo*, int*)) xdl_sym(handle, "il2cpp_method_get_flags", nullptr);
    il2cpp_image_get_class_count = (size_t (*)(Il2CppImage*)) xdl_sym(handle, "il2cpp_image_get_class_count", nullptr);
    il2cpp_image_get_class = (Il2CppClass *(*)(Il2CppImage*, size_t)) xdl_sym(handle, "il2cpp_image_get_class", nullptr);
    il2cpp_type_is_byref = (bool (*)(Il2CppType*)) xdl_sym(handle, "il2cpp_type_is_byref", nullptr);
    il2cpp_field_get_flags = (int (*)(FieldInfo*)) xdl_sym(handle, "il2cpp_field_get_flags", nullptr);
    il2cpp_field_get_name = (const char *(*)(FieldInfo*)) xdl_sym(handle, "il2cpp_field_get_name", nullptr);
    
    il2cpp_unity_liveness_allocate_struct = (void *(*)(void*, int, il2cpp_register_object_callback, void*, il2cpp_liveness_reallocate_callback)) xdl_sym(handle, "il2cpp_unity_liveness_allocate_struct", nullptr);
    il2cpp_unity_liveness_calculation_from_statics = (void (*)(void*)) xdl_sym(handle, "il2cpp_unity_liveness_calculation_from_statics", nullptr);
    il2cpp_unity_liveness_finalize = (void (*)(void*)) xdl_sym(handle, "il2cpp_unity_liveness_finalize", nullptr);
    il2cpp_unity_liveness_free_struct = (void (*)(void*)) xdl_sym(handle, "il2cpp_unity_liveness_free_struct", nullptr);
    il2cpp_stop_gc_world = (void (*)()) xdl_sym(handle, "il2cpp_stop_gc_world", nullptr);
    il2cpp_start_gc_world = (void (*)()) xdl_sym(handle, "il2cpp_start_gc_world", nullptr);
    il2cpp_free = (void (*)(void*)) xdl_sym(handle, "il2cpp_free", nullptr);
    il2cpp_alloc = (void *(*)(size_t)) xdl_sym(handle, "il2cpp_alloc", nullptr);
    il2cpp_string_new = (void *(*)(const char*)) xdl_sym(handle, "il2cpp_string_new", nullptr);
    il2cpp_array_new = (monoArray<void*> *(*)(Il2CppClass*, int)) xdl_sym(handle, "il2cpp_array_new", nullptr);
    il2cpp_value_box = (void *(*)(Il2CppClass*, void*)) xdl_sym(handle, "il2cpp_value_box", nullptr);
}

monoArray<void*> *CreateArrayFromClass(Il2CppClass *klass, int size) {
    if (!klass)
        return nullptr;
    return klass->CreateArray(size);
}

Il2CppImage *GetImage(const char *name) {
    Il2CppDomain *Domain = il2cpp_domain_get();
    Il2CppAssembly *Assembly = il2cpp_domain_assembly_open(Domain, name);
    return il2cpp_assembly_get_image(Assembly);
}
Il2CppClass *GetClass(const char *image, const char *name) {
    Il2CppImage *img = GetImage(image);
    return il2cpp_class_from_name(img, "", name);
}
Il2CppClass *GetClass(const char *image, const char *namespaze, const char *name) {
    Il2CppImage *img = GetImage(image);
    return il2cpp_class_from_name(img, namespaze, name);
}
std::vector<Il2CppImage*> GetAllImage() {
    std::vector<Il2CppImage*> list;
    size_t count_ass;
    Il2CppAssembly **assems = il2cpp_domain_get_assemblies(il2cpp_domain_get(), &count_ass);
    for (int i = 0; i < count_ass; ++i) {
        Il2CppAssembly *assem = assems[i];
        Il2CppImage *img = il2cpp_assembly_get_image(assem);
        list.push_back(img);
    }
    return list;
}

std::vector<Il2CppClass*> GetAllClasses() {
    std::vector<Il2CppClass*> list;
    std::vector<Il2CppImage*> list_image = GetAllImage();
    for (int i = 0; i < list_image.size(); ++i) {
        Il2CppImage *image = list_image[i];
        size_t class_count = il2cpp_image_get_class_count(image);
        for (int m = 0; m < class_count; ++m) {
            Il2CppClass *klass = il2cpp_image_get_class(image, m);
            list.push_back(klass);
        }
    }
    return list;
}

std::vector<MethodInfo*> GetAllMethods() {
    std::vector<MethodInfo*> list;
    std::vector<Il2CppClass*> list_class = GetAllClasses();
    for (int i = 0; i < list_class.size(); ++i) {
        Il2CppClass *klass = list_class[i];
        void *method_iter = NULL;
        while (MethodInfo *method = il2cpp_class_get_methods(klass, &method_iter)) {
            if (method->getAddress() == 0)
                continue;
            list.push_back(method);
        }
    }
    return list;
}

std::vector<Il2CppClass*> GetAllClassesImage(const char *image_name) {
    std::vector<Il2CppClass*> list;
    Il2CppImage *image = GetImage(image_name);
    size_t class_count = il2cpp_image_get_class_count(image);
    for (int m = 0; m < class_count; ++m) {
        Il2CppClass *klass = il2cpp_image_get_class(image, m);
        list.push_back(klass);
    }
    return list;
}

std::vector<MethodInfo*> GetAllMethodsImage(const char *image_name) {
    std::vector<MethodInfo*> list;
    std::vector<Il2CppClass*> list_class = GetAllClassesImage(image_name);
    for (int i = 0; i < list_class.size(); ++i) {
        Il2CppClass *klass = list_class[i];
        void *method_iter = NULL;
        while (MethodInfo *method = il2cpp_class_get_methods(klass, &method_iter)) {
            if (method->getAddress() == 0)
                continue;
            list.push_back(method);
        }
    }
    return list;
}

// Il2CppImage

Il2CppClass *Il2CppImage::GetClass(const char *name) {
    return il2cpp_class_from_name(this, "", name);
}
Il2CppClass *Il2CppImage::GetClass(const char *namespaze, const char *name) {
    return il2cpp_class_from_name(this, namespaze, name);
}

// Il2CppClass

MethodInfo *Il2CppClass::GetMethod(const char *name, int count) {
    return il2cpp_class_get_method_from_name(this, name, count);
}
MethodInfo *Il2CppClass::GetMethod(const char *name, int count, int countParam, const char *nameParam, int countParam2, const char *nameParam2) {
    void *iter;
    while (MethodInfo *method = il2cpp_class_get_methods(this, &iter)) {
        if (std::string(method->getName()) != std::string(name))
            continue;
        if (method->paramCount() != count)
            continue;
        
        std::vector<Il2CppType*> listParams = method->GetParams();
        Il2CppType *type = listParams[countParam - 1];
        if (std::string(nameParam) != std::string(type->getName()))
            continue;
        if (countParam2 != 0) {
            Il2CppType *type2 = listParams[countParam2 - 1];
            if (std::string(nameParam2) != std::string(type2->getName()))
                continue;
        }
        return method;
    }
    return this->GetMethod(name, count);;
}
std::vector<MethodInfo*> Il2CppClass::GetMethods() {
    std::vector<MethodInfo*> dataK;
    void *iter = NULL;
    while (MethodInfo *method = il2cpp_class_get_methods(this, &iter)) {
        if (method->getAddress() == 0)
            continue;
        dataK.push_back(method);
    }
    return dataK;
}
const char *Il2CppClass::getName() {
    return this->name;
}
const char *Il2CppClass::getNamespace() {
    return this->namespaze;
}
Il2CppGenericClass *Il2CppClass::GetGeneric() {
    return this->generic_class;
}
Il2CppObject *Il2CppClass::CreateObject() {
    return il2cpp_object_new(this);
}
Il2CppObject *CreateObjectFromClass(Il2CppClass *klass) {
    if (!klass)
        return nullptr;
    return il2cpp_object_new(klass);
}
void *ObjectBoxV(Il2CppClass *klass, void *val) {
    return il2cpp_value_box(klass, val);
}
Il2CppObject *Il2CppClass::getType() {
    Il2CppType *type = il2cpp_class_get_type(this);
    return il2cpp_type_get_object(type);
}
std::vector<void*> Il2CppClass::FindObjects() {
    std::vector<void*> objects;
    auto callback = [](void **arr, int size, void *userdata) {
        auto objects = reinterpret_cast<std::vector<void*>*>(userdata);
        objects->insert(objects->end(), arr, arr + size);
    };
    auto realloc = [](void *ptr, size_t size, void *userdata) -> void *
    {
        if (ptr != nullptr && size == 0) {
            il2cpp_free(ptr);
            return nullptr;
        } else {
            return il2cpp_alloc(size);
        }
    };
    il2cpp_stop_gc_world();
    auto state = il2cpp_unity_liveness_allocate_struct((void*)this, 0, callback, &objects, realloc);
    il2cpp_unity_liveness_calculation_from_statics(state);
    il2cpp_unity_liveness_finalize(state);
    il2cpp_start_gc_world();
    il2cpp_unity_liveness_free_struct(state);
    return objects;
}
void *Il2CppClass::ObjectBox(void *val) {
    return il2cpp_value_box(this, val);
}
monoArray<void*> *Il2CppClass::CreateArray(int size) {
    return il2cpp_array_new(this, size);
}
FieldInfo *Il2CppClass::getField(const char *name) {
    return il2cpp_class_get_field_from_name(this, name);
}
void *Il2CppClass::GetStaticField(const char *name) {
    FieldInfo *field = this->getField(name);
    void *result;
    il2cpp_field_static_get_value(field, &result);
    return result;
}
std::vector<FieldInfo*> Il2CppClass::GetFields() {
    std::vector<FieldInfo*> list;
    void *field_iter = NULL;
    while (FieldInfo *field = il2cpp_class_get_fields(this, &field_iter)) {
        list.push_back(field);
    }
    return list;
}

// Il2CppType

void *Il2CppType::GetDummy() {
    return this->data.dummy;
}
Il2CppGenericClass *Il2CppType::GetGeneric() {
    return this->data.generic_class;
}
const char *Il2CppType::getName() {
    return il2cpp_type_get_name(this);
}
Il2CppClass *Il2CppType::GetClass() {
    return il2cpp_class_from_type(this);
}
bool Il2CppType::isByref() {
    return il2cpp_type_is_byref(this);
}
Il2CppTypeEnum Il2CppType::TypeEnum() {
    return this->type;
}

// MethodInfo

uintptr_t MethodInfo::getAddress() {
    return (uintptr_t)this->methodPointer;
}
uintptr_t MethodInfo::getVirtualAddress() {
    return (uintptr_t)this->virtualMethodPointer;
}
const char *MethodInfo::getName() {
    return this->name;
}
Il2CppClass *MethodInfo::GetClass() {
    return this->klass;
}
Il2CppType *MethodInfo::GetReturn() {
    return this->return_type;
}
int MethodInfo::paramCount() {
    return il2cpp_method_get_param_count(this);
}
std::vector<Il2CppType*> MethodInfo::GetParams() {
    std::vector<Il2CppType*> list;
    for (int i = 0; i < this->paramCount(); ++i)
        list.push_back(this->parameters[i]);
    return list;
}
Il2CppObject *MethodInfo::RuntimeInvoke(void *obj, void **params, void **exc) {
    return il2cpp_runtime_invoke(this, obj, params, exc);
}
Il2CppGenericMethod *MethodInfo::getGeneric() {
    return this->genericMethod;
}
bool MethodInfo::isStatic() {
    int iflags = 0;
    int flags = il2cpp_method_get_flags(this, &iflags);
    if (flags & 0x0010)
        return true;
    return false;
}

// FieldInfo

size_t FieldInfo::getOffset() {
    return il2cpp_field_get_offset(this);
}
Il2CppType *FieldInfo::getType() {
    return this->type;
}
const char *FieldInfo::getName() {
    return il2cpp_field_get_name(this);
}
int FieldInfo::getFlag() {
    return il2cpp_field_get_flags(this);
}

// Il2CppObject

Il2CppClass *Il2CppObject::GetClass() {
    return this->klass;
}

// Il2CppGenericInst

int Il2CppGenericInst::Size() {
    return this->type_argc;
}
Il2CppType *Il2CppGenericInst::GetType(int count) {
    return this->type_argv[count];
}
std::vector<Il2CppType*> Il2CppGenericInst::GetTypes() {
    std::vector<Il2CppType*> list;
    for (int i = 0; i < this->Size(); ++i)
        list.push_back(this->type_argv[i]);
    return list;
}

// Il2CppGenericContext

Il2CppGenericInst *Il2CppGenericContext::ClassInst() {
    return this->class_inst;
}
Il2CppGenericInst *Il2CppGenericContext::MethodInst() {
    return this->method_inst;
}

// Il2CppGenericClass

Il2CppGenericContext Il2CppGenericClass::getContext() {
    return this->context;
}
Il2CppClass *Il2CppGenericClass::getClass() {
    return this->cached_class;
}

// Il2CppGenericMethod

MethodInfo *Il2CppGenericMethod::getDefinition() {
    return this->methodDefinition;
}
Il2CppGenericContext Il2CppGenericMethod::getContext() {
    return this->context;
}