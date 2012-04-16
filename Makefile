# APP := event-capture
INSTALL_DIR := /data/native
NDK_PLATFORM_VER := 9

ANDROID_NDK_ROOT := ${HOME}/Development/tools/android-ndk
ANDROID_SDK_ROOT := ${HOME}/Development/tools/android-sdk

all:
	$(ANDROID_NDK_ROOT)/ndk-build

install: all
	$(ANDROID_SDK_ROOT)/platform-tools/adb push ./libs/armeabi/record $(INSTALL_DIR)/record
	$(ANDROID_SDK_ROOT)/platform-tools/adb shell chmod 777 $(INSTALL_DIR)/record
	$(ANDROID_SDK_ROOT)/platform-tools/adb push ./libs/armeabi/replay $(INSTALL_DIR)/replay
	$(ANDROID_SDK_ROOT)/platform-tools/adb shell chmod 777 $(INSTALL_DIR)/replay

shell:
	$(ANDROID_SDK_ROOT)/platform-tools/adb shell

record:
	$(ANDROID_SDK_ROOT)/platform-tools/adb shell $(INSTALL_DIR)/record

replay:
	$(ANDROID_SDK_ROOT)/platform-tools/adb shell $(INSTALL_DIR)/replay

debug-install:
	$(ANDROID_SDK_ROOT)/platform-tools/adb push $(PREBUILD)/../gdbserver $(INSTALL_DIR)/gdbserver
	$(ANDROID_SDK_ROOT)/platform-tools/adb shell chmod 777 $(INSTALL_DIR)/gdbserver

debug-go:
	$(ANDROID_SDK_ROOT)/platform-tools/adb forward tcp:1234: tcp:1234
	$(ANDROID_SDK_ROOT)/platform-tools/adb shell $(INSTALL_DIR)/gdbserver :1234 $(INSTALL_DIR)/$(APP)

debug:
	$(GDB_CLIENT) $(APP)

clean:
	$(ANDROID_NDK_ROOT)/ndk-build clean

