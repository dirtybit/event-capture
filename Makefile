APP := event-capture
# ROOT := /home/dirt/opt/android-sdk-linybit/Development/workspace/eventcapture
INSTALL_DIR := /data/native
NDK_PLATFORM_VER := 9

ANDROID_NDK_ROOT := /opt/android-ndk-r7b
ANDROID_NDK_HOST := linux-x86
ANDROID_SDK_ROOT := /opt/android-sdk-linux
PREBUILD := $(ANDROID_NDK_ROOT)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/$(ANDROID_NDK_HOST)
BIN := $(PREBUILD)/bin/

CPP := $(BIN)/arm-linux-androideabi-g++
CC := $(BIN)/arm-linux-androideabi-gcc

GDB_CLIENT := $(BIN)/arm-linux-androideabi-gdb
DEBUG = -g
CFLAGS := $(DEBUG) -fno-short-enums -I$(ANDROID_NDK_ROOT)/platforms/android-$(NDK_PLATFORM_VER)/arch-arm/usr/include
LDFLAGS := -Wl,--entry=main,-dynamic-linker=/system/bin/linker,-rpath-link=$(ANDROID_NDK_ROOT)/platforms/android-$(NDK_PLATFORM_VER)/arch-arm/usr/lib -L$(ANDROID_NDK_ROOT)/platforms/android-$(NDK_PLATFORM_VER)/arch-arm/usr/lib
LDFLAGS += -nostdlib -lc
LDFLAGS += -disable-multilib

all: $(APP)

OBJS += record.o

$(APP): $(OBJS)
	$(CPP) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@ 

install: $(APP)
	$(ANDROID_SDK_ROOT)/platform-tools/adb push $(APP) $(INSTALL_DIR)/$(APP) 
	$(ANDROID_SDK_ROOT)/platform-tools/adb shell chmod 777 $(INSTALL_DIR)/$(APP)

shell:
	$(ANDROID_SDK_ROOT)/platform-tools/adb shell

run:
	$(ANDROID_SDK_ROOT)/platform-tools/adb shell $(INSTALL_DIR)/$(APP)

debug-install:
	$(ANDROID_SDK_ROOT)/platform-tools/adb push $(PREBUILD)/../gdbserver $(INSTALL_DIR)/gdbserver
	$(ANDROID_SDK_ROOT)/platform-tools/adb shell chmod 777 $(INSTALL_DIR)/gdbserver

debug-go:
	$(ANDROID_SDK_ROOT)/platform-tools/adb forward tcp:1234: tcp:1234
	$(ANDROID_SDK_ROOT)/platform-tools/adb shell $(INSTALL_DIR)/gdbserver :1234 $(INSTALL_DIR)/$(APP)

debug:
	$(GDB_CLIENT) $(APP)

clean:
	@rm -f $(APP).o $(APP)
