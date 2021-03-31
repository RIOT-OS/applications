# name of your application
APPLICATION = configuration_over_nimble

# If no BOARD is found in the environment, use this default:
BOARD ?= nrf52dk

# So far, NimBLE only works on nRF52 based platforms
BOARD_WHITELIST := nrf52dk nrf52840dk

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT

# Include NimBLE
USEPKG += nimble

USEMODULE += nimble_svc_gap
USEMODULE += nimble_svc_gatt

FEATURES_REQUIRED += periph_flashpage

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

CFLAGS += -DDEBUG_ASSERT_VERBOSE

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
