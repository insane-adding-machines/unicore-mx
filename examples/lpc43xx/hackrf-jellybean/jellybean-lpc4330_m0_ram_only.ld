/*
 * This file is part of the libopencm3 project.

 * Copyright 2012 Michael Ossmann <mike@ossmann.com>
 * Copyright 2012 Jared Boone <jared@sharebrained.com>
 * Copyright 2012 Benjamin Vernoux <titanmkd@gmail.com>
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

/* Linker script for HackRF Jellybean (LPC4330, 1M SPI flash, 264K SRAM). */

/* Define memory regions. */
MEMORY
{
	/* rom is really the shadow region that points to SPI flash or elsewhere */
	rom (rx)  : ORIGIN = 0x00000000, LENGTH =  1M
	ram_local1 (rwx) : ORIGIN = 0x10000000, LENGTH =  128K
	ram_local2 (rwx) : ORIGIN = 0x10080000, LENGTH =  72K
	/* there are some additional RAM regions */
	ram_ahb1   (rwx) : ORIGIN = 0x20000000, LENGTH =  16K
	/* Removed 32K of AHB SRAM for USB buffer. Straddles two blocks of RAM
	 * to get performance benefit of having two USB buffers addressable
	 * simultaneously (on two different buses of the AHB multilayer matrix)
	 */
	/* ram_ahb1_0 (rwx) : ORIGIN = 0x20004000, LENGTH =  16K */
	/* ram_ahb1_1 (rwx) : ORIGIN = 0x20008000, LENGTH =  16K */
	ram_ahb2   (rwx) : ORIGIN = 0x2000C000, LENGTH =  16K /* Reserved for Cortex M0 code/data */
}

/* Include the common ld script. */
INCLUDE libopencm3_lpc43xx_ram_only_m0.ld
