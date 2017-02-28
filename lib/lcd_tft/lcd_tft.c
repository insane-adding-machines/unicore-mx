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

#include "lcd_tft_private.h"

struct lcd_tft *lcd_tft_init(const struct lcd_tft_backend *backend,
					const struct lcd_tft_config *config)
{
	return backend->init(backend, config);
}

void lcd_tft_enable(struct lcd_tft *lt, bool enable)
{
	if (lt->backend->enable) {
		lt->backend->enable(lt, enable);
	}
}

void lcd_tft_layer_set(struct lcd_tft *lt, unsigned index,
			const struct lcd_tft_layer *layer)
{
	if (lt->backend->layer_set) {
		lt->backend->layer_set(lt, index, layer);
	}
}

void lcd_tft_layer_enable(struct lcd_tft *lt, unsigned index,
				bool enable)
{
	if (lt->backend->layer_enable) {
		lt->backend->layer_enable(lt, index, enable);
	}
}
