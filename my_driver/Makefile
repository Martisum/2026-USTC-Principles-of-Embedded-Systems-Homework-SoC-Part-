KERNELDIR := /home/martinx/exp_project/ustc_embed_homework/build_image/build
CROSS_COMPILE := /home/martinx/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
ARCH := arm

# oled_driver.c + oled.c + font.c → key_oled.ko
obj-m := my_oled.o
my_oled-objs := oled_driver.o oled.o font.o

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
