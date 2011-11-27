DRIVER_OBJS := timer.o 
DRIVER_OBJS := $(DRIVER_OBJS:%=$(KDIR)/drivers/%)

KOBJS += $(DRIVER_OBJS)
