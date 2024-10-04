include $(PROJECT)/src/platform/$(PLATFORM)/platform_src.mk
include $(PROJECT)/src/examples/examples_src.mk

SRCS += $(PROJECT)/src/platform/$(PLATFORM)/main.c

INCS += $(PROJECT)/src/common/common_data.h
SRCS += $(PROJECT)/src/common/common_data.c

INCS += $(PROJECT)/src/platform/platform_includes.h

INCS += $(PROJECT)/src/platform/$(PLATFORM)/parameters.h
SRCS += $(PROJECT)/src/platform/$(PLATFORM)/parameters.c

INCS += $(INCLUDE)/no_os_delay.h     \
		$(INCLUDE)/no_os_error.h     \
		$(INCLUDE)/no_os_gpio.h      \
		$(INCLUDE)/no_os_print_log.h \
		$(INCLUDE)/no_os_spi.h       \
		$(INCLUDE)/no_os_alloc.h       \
		$(INCLUDE)/no_os_irq.h      \
		$(INCLUDE)/no_os_list.h      \
		$(INCLUDE)/no_os_uart.h      \
		$(INCLUDE)/no_os_lf256fifo.h \
		$(INCLUDE)/no_os_util.h 	\
		$(INCLUDE)/no_os_units.h	\
                $(INCLUDE)/no_os_mutex.h	

SRCS += $(DRIVERS)/api/no_os_gpio.c \
		$(NO-OS)/util/no_os_lf256fifo.c \
		$(DRIVERS)/api/no_os_irq.c  \
		$(DRIVERS)/api/no_os_spi.c  \
		$(DRIVERS)/api/no_os_uart.c \
		$(NO-OS)/util/no_os_list.c \
		$(NO-OS)/util/no_os_util.c \
		$(NO-OS)/util/no_os_alloc.c \
                $(NO-OS)/util/no_os_mutex.c	

INCS += $(DRIVERS)/digital-io/max149x6/max149x6-base.h
SRCS += $(DRIVERS)/digital-io/max149x6/max149x6-base.c

INCS += $(DRIVERS)/digital-io/max149x6/max14906.h
SRCS += $(DRIVERS)/digital-io/max149x6/max14906.c
