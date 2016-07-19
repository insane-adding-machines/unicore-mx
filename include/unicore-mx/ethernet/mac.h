/** @defgroup ethernet_mac_defines MAC Generic Defines
 *
 * @brief <b>Defined Constants and Types for the Ethernet MAC</b>
 *
 * @ingroup ETH
 *
 * @version 1.0.0
 *
 * @author @htmlonly &copy; @endhtmlonly 2013 Frantisek Burian <BuFran@seznam.cz>
 *
 * @date 1 September 2013
 *
 * LGPL License Terms @ref lgpl_license
 */
/*
 * Copyright (C) 2013 Frantisek Burian <BuFran@seznam.cz>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/**@{*/

#ifndef UNICORE_MX_ETH
#define UNICORE_MX_ETH
#include <unicore-mx/cm3/common.h>

#if defined(STM32F1)
#       include <unicore-mx/ethernet/mac_stm32fxx7.h>
#elif defined(STM32F4)
#       include <unicore-mx/ethernet/mac_stm32fxx7.h>
#elif defined(STM32F7)
#       include <unicore-mx/ethernet/mac_stm32fxx7.h>
#elif defined(LM3S)
#       include <unicore-mx/ethernet/mac_lm3s.h>
#else
#       error "Ethernet not yet supported on this platform."
#endif

BEGIN_DECLS

void eth_smi_write(uint8_t phy, uint8_t reg, uint16_t data);
uint16_t eth_smi_read(uint8_t phy, uint8_t reg);
void eth_smi_bit_op(uint8_t phy, uint8_t reg, uint16_t bits, uint16_t mask);
void eth_smi_bit_clear(uint8_t phy, uint8_t reg, uint16_t clearbits);
void eth_smi_bit_set(uint8_t phy, uint8_t reg, uint16_t setbits);

void eth_set_mac(uint8_t *mac);
void eth_desc_init(uint8_t *buf, uint32_t nTx, uint32_t nRx, uint32_t cTx, uint32_t cRx, bool isext);
bool eth_tx(uint8_t *ppkt, uint32_t n);
bool eth_rx(uint8_t *ppkt, uint32_t *len, uint32_t maxlen);

void eth_init(uint8_t phy, enum eth_clk clock);
void eth_start(void);

void eth_enable_checksum_offload(void);

void eth_irq_enable(uint32_t reason);
void eth_irq_disable(uint32_t reason);
bool eth_irq_is_pending(uint32_t reason);
bool eth_irq_ack_pending(uint32_t reason);


END_DECLS

/**@}*/


#endif
