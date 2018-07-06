/*
 * Copyright (C) 2017 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
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

#ifndef UNICOREMX_LCD_TFT_PRIVATE_H
#define UNICOREMX_LCD_TFT_PRIVATE_H

#include <unicore-mx/cm3/common.h>
#include <unicore-mx/lcd_tft/lcd_tft.h>
#include <stdbool.h>

BEGIN_DECLS

struct lcd_tft {
	const struct lcd_tft_backend *backend;
};

struct lcd_tft_backend {
	struct lcd_tft *(*init)(const struct lcd_tft_backend *backend,
					const struct lcd_tft_config *config);
	void (*enable)(struct lcd_tft *lt, bool enable);
	void (*layer_set)(struct lcd_tft *lt, unsigned index,
					const struct lcd_tft_layer *layer);
	void (*layer_enable)(struct lcd_tft *lt, unsigned index, bool enable);
};

END_DECLS

#endif
