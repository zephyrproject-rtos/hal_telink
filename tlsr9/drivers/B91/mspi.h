/*
 * Copyright (c) 2019, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of TELINK nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/********************************************************************************************************
 * @file	mspi.h
 *
 * @brief	This is the header file for B91
 *
 * @author	Driver Group
 *
 *******************************************************************************************************/
#pragma once

#include "compiler.h"
#include "gpio.h"
#include "reg_include/mspi_reg.h"

/**
  * @brief     This function servers to set the spi wait.
  * @return    none.
  */
_attribute_ram_code_sec_ static inline void mspi_wait(void){
	while(reg_mspi_status & FLD_MSPI_BUSY);
}

/**
 * @brief     This function servers to enable read triggle spi.
 * @return    none.
 */
_attribute_ram_code_sec_ static inline void mspi_fm_rd_en(void){
	reg_mspi_fm |= FLD_MSPI_RD_TRIG_EN;
}

/**
 * @brief     This function servers to disable read triggle spi.
 * @return    none.
 */
_attribute_ram_code_sec_ static inline void mspi_fm_rd_dis(void){
	reg_mspi_fm &= ~FLD_MSPI_RD_TRIG_EN;
}

/**
 * @brief     This function servers to set spi interface csn signal.
 * @return    none.
 */
_attribute_ram_code_sec_ static inline void mspi_high(void){
	reg_mspi_fm |= FLD_MSPI_CSN;
}

/**
 * @brief     This function servers to clear spi interface csn signal.
 * @return    none.
 */
_attribute_ram_code_sec_ static inline void mspi_low(void){
	reg_mspi_fm &= ~FLD_MSPI_CSN;
}

/**
 * @brief		This function servers to gets the spi data.
 * @return		the spi data.
 */
_attribute_ram_code_sec_ static inline unsigned char mspi_get(void){
	return reg_mspi_data;
}

/**
 * @brief		This function servers to write the spi.
 * @param[in]	c	- the char need to be write.
 * @return		none.
 */
_attribute_ram_code_sec_ static inline void mspi_write(unsigned char c){
	reg_mspi_data = c;
}

/**
 * @brief		This function servers to control the write.
 * @param[in]	c	- need to be write.
 * @return		none.
 */
_attribute_ram_code_sec_ static inline void mspi_fm_write(unsigned char c){
	reg_mspi_fm = c;
}

/**
 * @brief		This function servers to spi read.
 * @return		read result.
 */
_attribute_ram_code_sec_ static inline unsigned char mspi_read(void){
	mspi_write(0);		// dummy, issue clock
	mspi_wait();
	return mspi_get();
}

/**
 * @brief		This function serves to Stop XIP operation before flash.
 * @return		none.
 */
_attribute_ram_code_sec_ static inline void mspi_stop_xip(void)
{
	mspi_wait();	//wait xip busy=0
	mspi_high();	//mspi_cn=1, stop xip read
	while(gpio_get_level(GPIO_PF3) == 0);	//wait cn=1
}



