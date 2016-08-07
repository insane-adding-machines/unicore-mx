/*
 * Copyright (C) 2016 Daniele Lacamera <root at danielinux.net>
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


#define PAGE_SIZE 		(1024)

#include <unicore-mx/lm3s/flash.h>

#define FMC_FLASH_WRITE_KEY 	((0xA442) << 16)
#define FMC_COMT 		(1 << 3)
#define FMC_MERASE 		(1 << 2)
#define FMC_ERASE 		(1 << 1)
#define FMC_WRITE 		(1)

void flash_erase_page(void *page_address)
{
    FLASH_FMA = page_address;
    FLASH_FMC = (FMC_FLASH_WRITE_KEY | FMC_ERASE);
}

void flash_program_word(void *address, uint32_t *data)
{
	FLASH_FMA = address;
	FLASH_FMD = *data;
	FLASH_FMC = FMC_FLASH_WRITE_KEY | FMC_WRITE;
}

int flash_write_finished(void)
{
	return ((FLASH_FMC & FMC_WRITE) == 0);
}
