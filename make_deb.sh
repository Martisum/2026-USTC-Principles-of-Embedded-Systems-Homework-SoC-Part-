deb_distro=bionic
DISTRO=stable

TOOLCHAIN=/home/martinx/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf                             
export PATH=${TOOLCHAIN}/bin:${PATH} 

build_opts="-j 16"
build_opts="${build_opts} O=build_image/build"
build_opts="${build_opts} ARCH=arm"
build_opts="${build_opts} KBUILD_DEBARCH=armhf"
build_opts="${build_opts} LOCALVERSION=-imx6"
build_opts="${build_opts} KDEB_CHANGELOG_DIST=${deb_distro}"
build_opts="${build_opts} KDEB_PKGVERSION=1.$(date +%g%m)${DISTRO}"
build_opts="${build_opts} CROSS_COMPILE=${TOOLCHAIN}/bin/arm-linux-gnueabihf-"
build_opts="${build_opts} KDEB_SOURCENAME=linux-upstream"
build_opts="${build_opts} HOSTCFLAGS=-fcommon"
make ${build_opts}  npi_v7_defconfig
make ${build_opts}  
make ${build_opts}  bindeb-pkg
