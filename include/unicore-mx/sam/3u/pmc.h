/*
 * Copyright (C) 2012 Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2015 Felix Held <felix-libopencm3@felixheld.de>
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

#ifndef UNICOREMX_PMC_H
#define UNICOREMX_PMC_H

#include <unicore-mx/sam/memorymap.h>
#include <unicore-mx/sam/common/pmc_common_all.h>
#include <unicore-mx/sam/common/pmc_common_3n3u.h>
#include <unicore-mx/sam/common/pmc_common_3a3u3x.h>

/* --- Register contents --------------------------------------------------- */


/* --- PMC Clock Generator Main Oscillator Register (CKGR_MOR) ------------- */

/* Wait Mode Command */
#define CKGR_MOR_WAITMODE		(0x01 << 2)


#endif
