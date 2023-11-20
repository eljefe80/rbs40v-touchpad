#RBS40v Touchpad
CONFIG_INPUT_RBS40V_TOUCHPAD=m

rbs40v-touchpad-objs := rbs40v-touchpad.o
obj-$(CONFIG_INPUT_RBS40V_TOUCHPAD) += input-rbs40v-touchpad.o

modules:
	@echo Got here
	$(MAKE)ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(shell pwd) modules
