ifneq ($(CONFIG_USER_LANDING_PAD), disabled)
ifneq ($(CONFIG_USER_LANDING_PAD), enabled)
$(error "Wrong value for CONFIG_USER_LANDING_PAD, should be either 'enabled' or 'disabled'")
endif
endif

ifneq ($(CONFIG_USER_SHADOW_STACK), disabled)
ifneq ($(CONFIG_USER_SHADOW_STACK), hardware)
$(error "Wrong value for CONFIG_USER_SHADOW_STACK, should be either 'enabled' or 'hardware'")
endif
endif
