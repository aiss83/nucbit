/*
 * Copyright (c) 2010, STMicroelectronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki OS
 *
 * $Id: leds-arch.c,v 1.1 2010/10/25 09:03:38 salvopitru Exp $
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *			Leds.
 * \author
 *			Salvatore Pitrulli <salvopitru@users.sourceforge.net>
 */
/*---------------------------------------------------------------------------*/

#include PLATFORM_HEADER
#include BOARD_HEADER
#include "contiki-conf.h"
#include "dev/leds.h"
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"

#define LEDS_PORT *((volatile int32u *)(GPIO_PxOUT_BASE+(GPIO_Px_OFFSET*(LEDS_CONF_PORT/8))))

/*---------------------------------------------------------------------------*/
void leds_arch_init(void) {
//#ifdef LEDS_CONF_PORT && LEDS_CONF_RED_PIN && EDS_CONF_GREEN_PIN
	halGpioConfig(PORTx_PIN(LEDS_CONF_PORT, LEDS_CONF_RED_PIN), GPIOCFG_OUT);
	halGpioConfig(PORTx_PIN(LEDS_CONF_PORT, LEDS_CONF_GREEN_PIN), GPIOCFG_OUT);
	halGpioConfig(PORTx_PIN(LEDS_CONF_PORT, LEDS_CONF_BLUE_PIN), GPIOCFG_OUT);

	//  LEDS_PORT |= (LEDS_CONF_RED | LEDS_CONF_GREEN);
	LEDS_PORT &= ~(LEDS_CONF_BLUE | LEDS_CONF_RED | LEDS_CONF_GREEN);
//#endif
}
/*---------------------------------------------------------------------------*/
unsigned char leds_arch_get(void) {
	//  #ifdef LEDS_CONF_PORT && LEDS_CONF_RED_PIN && LEDS_CONF_GREEN_PIN
	return ((LEDS_PORT & LEDS_CONF_RED) ? 0 : LEDS_CONF_RED)
			| ((LEDS_PORT & LEDS_CONF_BLUE) ? 0 : LEDS_CONF_BLUE)
			| ((LEDS_PORT & LEDS_CONF_GREEN) ? 0 : LEDS_CONF_GREEN);
//#else
//    return 0;
//#endif
}
/*---------------------------------------------------------------------------*/
void leds_arch_set(unsigned char leds) {
//#ifdef LEDS_CONF_PORT && LEDS_CONF_RED_PIN && LEDS_CONF_GREEN_PIN
	int new = (LEDS_PORT & ~(LEDS_CONF_RED | LEDS_CONF_GREEN | LEDS_CONF_BLUE));
	//new |=
			new |= ((leds & LEDS_CONF_RED) ? LEDS_CONF_RED :0 );
			new |= ((leds & LEDS_CONF_BLUE) ? LEDS_CONF_BLUE :0 );
			new |= ((leds & LEDS_CONF_GREEN) ? LEDS_CONF_GREEN:0 );
	LEDS_PORT = new;
//#endif
}
/*---------------------------------------------------------------------------*/
