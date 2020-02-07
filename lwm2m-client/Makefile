# name of your application
APPLICATION = lwm2m-client

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT


# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules
USEMODULE += gnrc_ipv6_router_default
USEMODULE += gnrc_sock_udp
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo
USEMODULE += gcoap
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
# Include and auto-initialize all available sensors
ifneq (native,$(BOARD))
  USEMODULE += saul_default
endif

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Specific the server address
SERVER_ADDR ?= '"fd00:dead:beef::1"'

ifneq (,$(SERVER_ADDR))
  CFLAGS += -DLWM2M_SERVER_ADDR=$(SERVER_ADDR)
endif

CFLAGS += -DTHREAD_PRIORITY_MAIN=10

# SAUL driver that measures temperature; hardware dependent
SAUL_DRIVER ?= jc42
ifneq (native,$(BOARD))
  USEMODULE += $(SAUL_DRIVER)
  CFLAGS += -DSAUL_INFO_DRIVER='"$(SAUL_DRIVER)"'
else
  CFLAGS += -DSAUL_INFO_DRIVER=NULL
endif

include $(RIOTBASE)/Makefile.include
