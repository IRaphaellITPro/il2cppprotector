
LOCAL_PATH := $(call my-dir)

# ── prebuilt: Dobby static library ─────────────────────────────────
include $(CLEAR_VARS)
LOCAL_MODULE    := dobby
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libdobby.a
include $(PREBUILT_STATIC_LIBRARY)

# ── main shared library ─────────────────────────────────────────────
include $(CLEAR_VARS)
LOCAL_MODULE := il2cpp

LOCAL_CFLAGS := \
    -w -s \
    -fvisibility=hidden \
    -fPIC \
    -fexceptions \
    -fpermissive \
    -Wno-error=format-security

LOCAL_CPPFLAGS := \
    -w -s \
    -std=c++17 \
    -fvisibility=hidden \
    -fPIC \
    -fexceptions \
    -fpermissive \
    -Wno-error=format-security \
    -Wno-error=c++11-narrowing

LOCAL_LDFLAGS := \
    $(LOCAL_PATH)/lib/$(TARGET_ARCH_ABI)/libhack_so_enc.o \
    -Wl,--gc-sections \
    -Wl,--strip-all \
    -Wl,--build-id=none

LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv3 -ldl

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/include/xdl \
    $(LOCAL_PATH)/include/Hook/Dobby \
    $(LOCAL_PATH)/include/Hook/KittyMemory \
    $(LOCAL_PATH)/include/Utils \
    $(LOCAL_PATH)/include/Utils/Unity \
    $(LOCAL_PATH)/include/Utils/il2cpp

# ── source files ────────────────────────────────────────────────────
FILE_LIST  := $(wildcard $(LOCAL_PATH)/*.cpp)
FILE_LIST  += $(wildcard $(LOCAL_PATH)/src/xdl/*.c)
FILE_LIST  += $(wildcard $(LOCAL_PATH)/src/Hook/KittyMemory/*.cpp)
FILE_LIST  += $(wildcard $(LOCAL_PATH)/src/Utils/*.cpp)

LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_STATIC_LIBRARIES := dobby

include $(BUILD_SHARED_LIBRARY)