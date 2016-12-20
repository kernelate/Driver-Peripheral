#KERNEL_MAKE := -C /home/software10/workspace/jadl/kernel
#KERNEL_MAKE := -C $(KERNEL_DIR)
KERNEL_MAKE := -C /home/ntekcom11/Documents/Files/Platforms/SVN/kernel_DT


obj-m := doortalk_drv.o
doortalk_drv-y += doortalk_drivers.o

default:
	make $(KERNEL_MAKE) M=$(PWD) modules
clean:
	make $(KERNEL_MAKE) M=$(PWD) clean
