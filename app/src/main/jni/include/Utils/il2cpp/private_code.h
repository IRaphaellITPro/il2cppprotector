typedef enum Il2CppTypeEnum {
    IL2CPP_TYPE_END = 0x00,
    IL2CPP_TYPE_VOID = 0x01,
    IL2CPP_TYPE_BOOLEAN = 0x02,
    IL2CPP_TYPE_CHAR = 0x03,
    IL2CPP_TYPE_I1 = 0x04,
    IL2CPP_TYPE_U1 = 0x05,
    IL2CPP_TYPE_I2 = 0x06,
    IL2CPP_TYPE_U2 = 0x07,
    IL2CPP_TYPE_I4 = 0x08,
    IL2CPP_TYPE_U4 = 0x09,
    IL2CPP_TYPE_I8 = 0x0a,
    IL2CPP_TYPE_U8 = 0x0b,
    IL2CPP_TYPE_R4 = 0x0c,
    IL2CPP_TYPE_R8 = 0x0d,
    IL2CPP_TYPE_STRING = 0x0e,
    IL2CPP_TYPE_PTR = 0x0f,
    IL2CPP_TYPE_BYREF = 0x10,
    IL2CPP_TYPE_VALUETYPE = 0x11,
    IL2CPP_TYPE_CLASS = 0x12,
    IL2CPP_TYPE_VAR = 0x13,
    IL2CPP_TYPE_ARRAY = 0x14,
    IL2CPP_TYPE_GENERICINST = 0x15,
    IL2CPP_TYPE_TYPEDBYREF = 0x16,
    IL2CPP_TYPE_I = 0x18,
    IL2CPP_TYPE_U = 0x19,
    IL2CPP_TYPE_FNPTR = 0x1b,
    IL2CPP_TYPE_OBJECT = 0x1c,
    IL2CPP_TYPE_SZARRAY = 0x1d,
    IL2CPP_TYPE_MVAR = 0x1e,
    IL2CPP_TYPE_CMOD_REQD = 0x1f,
    IL2CPP_TYPE_CMOD_OPT = 0x20,
    IL2CPP_TYPE_INTERNAL = 0x21,
    IL2CPP_TYPE_MODIFIER = 0x40,
    IL2CPP_TYPE_SENTINEL = 0x41,
    IL2CPP_TYPE_PINNED = 0x45,
    IL2CPP_TYPE_ENUM = 0x55,
    IL2CPP_TYPE_IL2CPP_TYPE_INDEX = 0xff
} Il2CppTypeEnum;

struct ClassImpl {
    Il2CppImage *image;
    void *gc_desc;
    const char *name;
    const char *namespaze;
    Il2CppType *byval_arg;
    Il2CppType *this_arg;
    Il2CppClass *element_class;
    Il2CppClass *castClass;
    Il2CppClass *declaringType;
    Il2CppClass *parent;
    Il2CppGenericClass *generic_class;
    void *typeMetadataHandle;
    void *interopData;
    Il2CppClass *klass;
    void *fields;
    void *events;
    void *properties;
    MethodInfo **methods;
    Il2CppClass **nestedTypes;
    Il2CppClass **implementedInterfaces;
    void *interfaceOffsets;
    void *static_field;
};

struct TypeImpl {
    union {
        void *dummy;
        int __klassIndex;
        void *typeHandle;
        Il2CppType *type;
        void *array;
        int __genericParameterIndex;
        void *genericParameterHandle;
        Il2CppGenericClass *generic_class;
    } data;
    unsigned int attrs: 16;
    Il2CppTypeEnum type: 8;
    unsigned int num_mods: 6;
    unsigned int byref: 1;
    unsigned int pinned: 1;
};

typedef const struct ___Il2CppMetadataMethodHandle *Il2CppMetadataMethodDefinitionHandle;
typedef const struct ___Il2CppMetadataGenericContainerHandle *Il2CppMetadataGenericContainerHandle;

struct MethodImpl {
    void *methodPointer;
    void *virtualMethodPointer;
    InvokerMethod invoker_method;
    const char *name;
    Il2CppClass *klass;
    Il2CppType *return_type;
    Il2CppType **parameters;
    union {
        void *rgctx_data;
        Il2CppMetadataMethodDefinitionHandle methodMetadataHandle;
    };
    union {
        Il2CppGenericMethod *genericMethod;
        Il2CppMetadataGenericContainerHandle genericContainerHandle;
    };
};
struct ObjectImpl {
    union {
        Il2CppClass *klass;
        Il2CppClass *vtable;
    };
};
struct GenericInstImpl {
    int type_argc;
    Il2CppType **type_argv;
};
struct GenericContextImpl {
    Il2CppGenericInst *class_inst;
    Il2CppGenericInst *method_inst;
};

struct FieldInfoImpl {
    const char *name;
    Il2CppType *type;
    Il2CppClass *parent;
    int32_t offset;
    uint32_t token;
};