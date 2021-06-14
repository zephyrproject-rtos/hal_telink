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
 * @file	trng.c
 *
 * @brief	This is the source file for B91
 *
 * @author	Driver Group
 *
 *******************************************************************************************************/
#include "trng.h"
#include "compiler.h"
/**********************************************************************************************************************
 *                                			  local constants                                                       *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                           	local macro                                                        *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                             local data type                                                     *
 *********************************************************************************************************************/


/**********************************************************************************************************************
 *                                              global variable                                                       *
 *********************************************************************************************************************/

_attribute_data_retention_sec_ unsigned int g_rnd_m_w = 0;
_attribute_data_retention_sec_ unsigned int g_rnd_m_z = 0;

/**********************************************************************************************************************
 *                                              local variable                                                     *
 *********************************************************************************************************************/
/**********************************************************************************************************************
 *                                          local function prototype                                               *
 *********************************************************************************************************************/
/**********************************************************************************************************************
 *                                         global function implementation                                             *
 *********************************************************************************************************************/
/**
 * @brief     This function performs to get one random number.If chip in suspend TRNG module should be close.
 *            else its current will be larger.
 * @return    the value of one random number.
 */
void trng_init(void)
{
	//TRNG module Reset clear
	reg_rst2 |= FLD_RST2_TRNG;
	//turn on TRNG clock
	reg_clk_en2 |= FLD_CLK2_TRNG_EN;

	reg_trng_cr0 &= ~(FLD_TRNG_CR0_RBGEN); //disable
	reg_trng_rtcr = 0x00;				//TCR_MSEL
	reg_trng_cr0 |= (FLD_TRNG_CR0_RBGEN); //enable

	while(!(reg_rbg_sr & FLD_RBG_SR_DRDY));
	g_rnd_m_w = reg_rbg_dr;   //get the random number
	while(!(reg_rbg_sr & FLD_RBG_SR_DRDY));
	g_rnd_m_z = reg_rbg_dr;

	//Reset TRNG module
	reg_rst2 &= (~FLD_RST2_TRNG);
	//turn off TRNG module clock
	reg_clk_en2 &= ~(FLD_CLK2_TRNG_EN);

	reg_trng_cr0 &= ~(FLD_TRNG_CR0_RBGEN | FLD_TRNG_CR0_ROSEN0 | FLD_TRNG_CR0_ROSEN1 \
											| FLD_TRNG_CR0_ROSEN2 | FLD_TRNG_CR0_ROSEN3);
}

/**
 * @brief     This function performs to get one random number.
 * @return    the value of one random number.
 */
_attribute_ram_code_sec_noinline_  unsigned int trng_rand(void)  //16M clock, code in flash 23us, code in sram 4us
{

	g_rnd_m_w = 18000 * (g_rnd_m_w & 0xffff) + (g_rnd_m_w >> 16);
	g_rnd_m_z = 36969 * (g_rnd_m_z & 0xffff) + (g_rnd_m_z >> 16);
	unsigned int result = (g_rnd_m_z << 16) + g_rnd_m_w;

	return (unsigned int)( result  ^ stimer_get_tick() );
}

