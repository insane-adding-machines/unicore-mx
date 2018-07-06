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

#ifndef UNICOREMX_LCD_TFT_H
#define UNICOREMX_LCD_TFT_H

#include <unicore-mx/cm3/common.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

BEGIN_DECLS

typedef struct lcd_tft lcd_tft;
typedef struct lcd_tft_backend lcd_tft_backend;

extern const lcd_tft_backend lcd_tft_stm32_ltdc;

/**
 * @note Single transparent color supported
 */
#define LCD_TFT_STM32_LTDC (&lcd_tft_stm32_ltdc)

struct lcd_tft_config {
	struct {
		unsigned vsync, hsync;
		unsigned vbp, hbp;
		unsigned height, width;
		unsigned vfp, hfp;
	} timing;

	enum {
		LCD_TFT_HSYNC_ACTIVE_LOW = 0 << 0,
		LCD_TFT_HSYNC_ACTIVE_HIGH = 1 << 0,
		LCD_TFT_VSYNC_ACTIVE_LOW = 0 << 1,
		LCD_TFT_VSYNC_ACTIVE_HIGH = 1 << 1,
		LCD_TFT_DE_ACTIVE_LOW = 0 << 2,
		LCD_TFT_DE_ACTIVE_HIGH = 1 << 2,
		LCD_TFT_CLK_ACTIVE_LOW = 0 << 3,
		LCD_TFT_CLK_ACTIVE_HIGH = 1 << 3,

		/* https://en.wikipedia.org/wiki/Dither */
		LCD_TFT_DITHER_ENABLE = 1 << 4,

		LCD_TFT_HSYNC_ACTIVE_MASK = 1 << 0,
		LCD_TFT_VSYNC_ACTIVE_MASK = 1 << 1,
		LCD_TFT_DE_ACTIVE_MASK = 1 << 2,
		LCD_TFT_CLK_ACTIVE_MASK = 1 << 3,
		LCD_TFT_DITHER_MASK = 1 << 4
	} features;

	/* Number of output lines */
	enum {
		LCD_TFT_OUTPUT_RGB565, /* 16bit */
		LCD_TFT_OUTPUT_RGB666, /* 18bit */
		LCD_TFT_OUTPUT_RGB888  /* 24bit */
	} output;

	/** Background color - all pixel in layer are transparent (Format: RGB888) */
	uint32_t background;
};

enum lcd_tft_pixel_format {
	LCD_TFT_ARGB8888,
	LCD_TFT_RGB888,
	LCD_TFT_RGB565,
	LCD_TFT_ARGB1555,
	LCD_TFT_ARGB4444,
	LCD_TFT_L8,
	LCD_TFT_AL44,
	LCD_TFT_AL88
};

struct lcd_tft_layer {
	struct {
		unsigned x, y, width, height;

		/**
		 * Note: Pointer should be aligned to
		 * 32bit if 1 pixel contain 4byte data
		 * 16bit if 1 pixel contain 2byte data
		 */
		enum lcd_tft_pixel_format format;
		const void *data;
	} framebuffer;

	/**
	 * Hardware palette or Color Look-Up Table (CLUT).
	 * Only valid for format = L8, AL44, AL88
	 * Pass @a data = NULL and @a count = 0 to disable.
	 * @note @a data format is RGB888
	 */
	struct {
		const uint32_t *data;
		size_t count;
	} palette;

	/**
	 * These colors are seen as transparent if found in frame buffer.
	 * Pass @a data = NULL and @a count = 0 to disable.
	 * @note @a data format is RGB888
	 */
	struct {
		const uint32_t *data;
		size_t count;
	} transparent;
};

/**
 * Initalize LCD TFT
 * @param backend LCD TFT backend
 * @param config Configuration
 * @return lcd_tft LCD TFT object
 */
lcd_tft *lcd_tft_init(const lcd_tft_backend *backend,
					const struct lcd_tft_config *config);

/**
 * Enable the LCD TFT
 * @param backend LCD TFT backend
 * @param enable Enable
 */
void lcd_tft_enable(lcd_tft *lt, bool enable);

/**
 * Setup a layer for LCD TFT
 * @param ldc_tft LCD TFT object
 * @param index Layer index (0...N)
 * @param layer Layer data
 */
void lcd_tft_layer_set(lcd_tft *lt, unsigned index,
			const struct lcd_tft_layer *layer);

/**
 * Enable the LCD TFT layer
 * @param lt LCD TFT object
 * @param index Layer index (0...N)
 * @param enable Enable
 */
void lcd_tft_layer_enable(lcd_tft *lt, unsigned index,
				bool enable);

END_DECLS

#endif
