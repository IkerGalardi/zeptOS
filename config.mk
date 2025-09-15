# Wether or not to enable landing pad extension support on user processes
# Possible values:
#   disabled: landing pads are disabled
#   enabled:  landing pads are enabled
CONFIG_USER_LANDING_PAD=enabled

# Wether or not to enable landing pad extension support on kernel
# Possible values:
#   disabled: landing pads are disabled
#   enabled:  landing pads are enabled
CONFIG_KERNEL_LANDING_PAD=disabled

# Wether or not to enable shadow stack sanitization
# Possible values:
#   disabled: shadow stack is disabled
#   software: software based shadow stack
#   hardware: shadow stack using hardware extensions
CONFIG_USER_SHADOW_STACK=hardware
