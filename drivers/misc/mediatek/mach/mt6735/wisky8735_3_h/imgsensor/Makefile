#
# Makefile for misc devices that really don't fit anywhere else.
#
include $(srctree)/drivers/misc/mediatek/Makefile.custom

define FILTER_PROJECT_DRV
$(info ----->2 $(1))

ifneq ($(wildcard $(srctree)/drivers/misc/mediatek/mach/$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)/imgsensor/$(1)),)
platform_drv += $(1)
endif
endef

$(foreach drv,$(subst $\",,$(CONFIG_CUSTOM_KERNEL_IMGSENSOR)),$(eval $(call FILTER_PROJECT_DRV,$(drv))))

$(info ----->2-platform_drv: $(platform_drv))
custom_kernel_imgsensor := $(addsuffix /, $(shell echo $(platform_drv)))
obj-y += $(custom_kernel_imgsensor)
$(info --imgsensor drv by platform $(custom_kernel_imgsensor))
