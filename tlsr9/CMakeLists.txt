#
# Copyright (c) 2021, Telink
#
# SPDX-License-Identifier: BSD-3-Clause
#

zephyr_include_directories(common)
zephyr_include_directories(drivers/B91/compatibility_pack)
zephyr_include_directories(drivers/B91/reg_include)
zephyr_include_directories(drivers/B91)

# soc.c referance sources
zephyr_sources(drivers/B91/sys.c)
zephyr_sources(drivers/B91/pm.c)
zephyr_sources(drivers/B91/clock.c)
zephyr_sources(drivers/B91/analog.c)

# Flash driver reference sources
zephyr_sources_ifdef(CONFIG_SOC_FLASH_TELINK_B91 drivers/B91/flash.c)
zephyr_sources_ifdef(CONFIG_SOC_FLASH_TELINK_B91 drivers/B91/plic.c)
zephyr_sources_ifdef(CONFIG_SOC_FLASH_TELINK_B91 drivers/B91/stimer.c)

# PWM driver reference sources
zephyr_sources_ifdef(CONFIG_PWM_TELINK_B91 drivers/B91/pwm.c)

# SPI driver reference sources
zephyr_sources_ifdef(CONFIG_SPI_TELINK_B91 drivers/B91/gpio.c)

# I2C driver reference sources
zephyr_sources_ifdef(CONFIG_I2C_TELINK_B91 drivers/B91/i2c.c)

# RF driver reference sources
zephyr_sources_ifdef(CONFIG_IEEE802154_TELINK_B91 drivers/B91/stimer.c)
zephyr_sources_ifdef(CONFIG_IEEE802154_TELINK_B91 drivers/B91/rf.c)

# Entropy driver reference sources
zephyr_sources_ifdef(CONFIG_ENTROPY_TELINK_B91_TRNG drivers/B91/trng.c)

# soc.c HW crypto sources
zephyr_sources(drivers/B91/aes.c)
zephyr_sources(drivers/B91/pke.c)
