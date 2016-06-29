#!/bin/bash

export ARM_ROOT=/home/you/workenv/android/android-ndk-r10d
export ARM_INC=$ARM_ROOT/platforms/android-14/arch-arm/usr/include/
export ARM_LIB=$ARM_ROOT/platforms/android-14/arch-arm/usr/lib/
export ARM_TOOL=$ARM_ROOT/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86/
export ARM_LIBO=$ARM_TOOL/lib/gcc/arm-linux-androideabi/4.4.3
export PATH=$ARM_TOOL/bin:$PATH
export ARM_PRE=arm-linux-androideabi
export ARM_PREFIX=/home/you/workenv/android/x264_install

./configure --disable-gpac --enable-pic --enable-strip --extra-cflags=" -I$ARM_INC -fPIC -DANDROID -fpic -mthumb-interwork -ffunction-sections -funwind-tables -fstack-protector -fno-short-enums -march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=neon -D__ARM_ARCH_7__ -D__ARM_ARCH_7A__  -Wno-psabi -msoft-float -mthumb -Os -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 -DANDROID  -Wa,--noexecstack -MMD -MP " --extra-ldflags="-nostdlib -Bdynamic -Wl,--no-undefined -Wl,-z,noexecstack  -Wl,-z,nocopyreloc -Wl,-soname,/system/lib/libz.so -Wl,-rpath-link=$ARM_LIB,-dynamic-linker=/system/bin/linker -L$ARM_LIB -nostdlib $ARM_LIB/crtbegin_dynamic.o $ARM_LIB/crtend_android.o -lc -lm -ldl -lgcc" --cross-prefix=${ARM_PRE}- --host=arm-linux --enable-static --prefix=$ARM_PREFIX

