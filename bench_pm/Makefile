APPLICATION = bench_pm
BOARD ?= frdm-kw41z

RIOTBASE ?= $(CURDIR)/../../RIOT
QUIET ?= 1
DEVELHELP ?= 1

FEATURES_OPTIONAL = periph_rtt periph_gpio_irq

# Disable auto_init to avoid starting any extra services
DISABLE_MODULE += auto_init

include $(RIOTBASE)/Makefile.include
