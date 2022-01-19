#!/bin/bash
BUILD_START=$(date +"%s")
tcdir=${HOME}/android/TOOLS/GCC

[ -d "out" ] && rm -rf out && mkdir -p out || mkdir -p out

[ -d $tcdir ] && \
echo "ARM64 TC Present." || \
echo "ARM64 TC Not Present. Downloading..." | \
git clone --depth=1 https://github.com/LineageOS/android_prebuilts_gcc_linux-x86_aarch64_aarch64-linux-android-4.9 $tcdir/los-4.9-64

[ -d $tcdir ] && \
echo "ARM32 TC Present." || \
echo "ARM32 TC Not Present. Downloading..." | \
git clone --depth=1 https://github.com/LineageOS/android_prebuilts_gcc_linux-x86_arm_arm-linux-androideabi-4.9 $tcdir/los-4.9-32

make O=out ARCH=arm E2M-perf_defconfig

PATH="$tcdir/los-4.9-32/bin:${PATH}" \
make    O=out \
        ARCH=arm \
        CROSS_COMPILE=arm-linux-androideabi- \
        KCFLAGS=-mno-android \
        -j$(nproc --all) || exit

cp out/arch/arm64/boot/Image out

