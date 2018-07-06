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

#include "../lcd_tft_private.h"
#include <unicore-mx/stm32/ltdc.h>
#include <unicore-mx/stm32/rcc.h>

static struct lcd_tft *init(const struct lcd_tft_backend *backend,
				const struct lcd_tft_config *config);
static void enable(struct lcd_tft *lt, bool enable);
static void layer_set(struct lcd_tft *lt, unsigned index,
				const struct lcd_tft_layer *layer);
static void layer_enable(struct lcd_tft *lt, unsigned index,
				bool enable);

const struct lcd_tft_backend lcd_tft_stm32_ltdc = {
	.init = init,
	.enable = enable,
	.layer_set = layer_set,
	.layer_enable = layer_enable
};

static const struct lcd_tft _lt = {
	.backend = &lcd_tft_stm32_ltdc
};

static void timing(const struct lcd_tft_config *config)
{
	uint16_t w, h;
	w = config->timing.hsync - 1;
	h = config->timing.vsync - 1;
	LTDC_SSCR = (w << 16) | (h << 0);

	w += config->timing.hbp;
	h += config->timing.vbp;
	LTDC_BPCR = (w << 16) | (h << 0);

	w += config->timing.width;
	h += config->timing.height;
	LTDC_AWCR = (w << 16) | (h << 0);

	w += config->timing.hfp;
	h += config->timing.vfp;
	LTDC_TWCR = (w << 16) | (h << 0);
}

static void features(const struct lcd_tft_config *config)
{
	uint32_t gcr = 0;

	if (config->features & LCD_TFT_HSYNC_ACTIVE_MASK) {
		gcr |= LTDC_GCR_HSPOL_ACTIVE_HIGH;
	}

	if (config->features & LCD_TFT_VSYNC_ACTIVE_MASK) {
		gcr |= LTDC_GCR_VSPOL_ACTIVE_HIGH;
	}

	if (config->features & LCD_TFT_DE_ACTIVE_MASK) {
		gcr |= LTDC_GCR_DEPOL_ACTIVE_HIGH;
	}

	if (config->features & LCD_TFT_CLK_ACTIVE_MASK) {
		gcr |= LTDC_GCR_PCPOL_ACTIVE_HIGH;
	}

	if (config->features & LCD_TFT_DITHER_MASK) {
		gcr |= LTDC_GCR_DITHER_ENABLE;
	}

	LTDC_GCR = gcr;
}

static struct lcd_tft *init(const struct lcd_tft_backend *backend,
					const struct lcd_tft_config *config)
{
	(void) backend;

	rcc_periph_clock_enable(RCC_LTDC);

	features(config);
	timing(config);

	LTDC_BCCR = config->background;
	LTDC_GCR |= LTDC_GCR_LTDC_ENABLE;

	return (struct lcd_tft *) &_lt;
}

static void enable(struct lcd_tft *lt, bool enable)
{
	(void) lt;

	if (enable) {
		LTDC_GCR |= LTDC_GCR_LTDC_ENABLE;
	} else {
		LTDC_GCR &= ~LTDC_GCR_LTDC_ENABLE;
	}
}

static void layer_window(unsigned num, const struct lcd_tft_layer *layer)
{
	uint32_t hbp = (LTDC_BPCR >> LTDC_BPCR_AHBP_SHIFT) & LTDC_BPCR_AHBP_MASK;
	uint32_t vbp = (LTDC_BPCR >> LTDC_BPCR_AVBP_SHIFT) & LTDC_BPCR_AVBP_MASK;
	uint16_t start, stop;

	start = hbp + layer->framebuffer.x + 1;
	stop = hbp + layer->framebuffer.x + layer->framebuffer.width;
	LTDC_LxWHPCR(num) = (stop  << 16) | (start << 0);

	start = vbp + layer->framebuffer.y + 1;
	stop = vbp + layer->framebuffer.y + layer->framebuffer.height;
	LTDC_LxWVPCR(num) = (stop << 16) | (start << 0);
}

static const uint32_t pixel_format_conv[] = {
	[LCD_TFT_ARGB8888] = LTDC_LxPFCR_ARGB8888,
	[LCD_TFT_RGB888] = LTDC_LxPFCR_RGB888,
	[LCD_TFT_RGB565] = LTDC_LxPFCR_RGB565,
	[LCD_TFT_ARGB1555] = LTDC_LxPFCR_ARGB1555,
	[LCD_TFT_ARGB4444] = LTDC_LxPFCR_ARGB4444,
	[LCD_TFT_L8] = LTDC_LxPFCR_L8,
	[LCD_TFT_AL44] = LTDC_LxPFCR_AL44,
	[LCD_TFT_AL88] = LTDC_LxPFCR_AL88
};

static const unsigned pixel_format_bytes[] = {
	[LCD_TFT_ARGB8888] = 4,
	[LCD_TFT_RGB888] = 3,
	[LCD_TFT_RGB565] = 2,
	[LCD_TFT_ARGB1555] = 2,
	[LCD_TFT_ARGB4444] = 2,
	[LCD_TFT_L8] = 1,
	[LCD_TFT_AL44] = 1,
	[LCD_TFT_AL88] = 2
};

static void layer_data(unsigned num, const struct lcd_tft_layer *layer)
{
	LTDC_LxPFCR(num) = pixel_format_conv[layer->framebuffer.format];
	LTDC_LxCFBAR(num)  = (uint32_t) layer->framebuffer.data;

	unsigned bpp = pixel_format_bytes[layer->framebuffer.format];
	unsigned bytes = bpp * layer->framebuffer.width;
	LTDC_LxCFBLR(num) = (bytes << 16) | ((bytes + 3) << 0);
	LTDC_LxCFBLNR(num) = layer->framebuffer.height;
}

static void layer_clut(unsigned num, const struct lcd_tft_layer *layer)
{
	if (!layer->palette.count) {
		/* Not provided */
		LTDC_LxCR(num) &= ~LTDC_LxCR_CLUT_ENABLE;
		return;
	}

	LTDC_LxCR(num) |= LTDC_LxCR_CLUT_ENABLE;

	for (uint8_t i = 0; i < layer->palette.count; i++) {
		LTDC_LxCLUTWR(num) = (i << 24) | layer->palette.data[i];
	}
}

static void layer_key_color(unsigned num, const struct lcd_tft_layer *layer)
{
	if (!layer->transparent.count) {
		/* Not provided */
		LTDC_LxCR(num) &= ~LTDC_LxCR_COLKEY_ENABLE;
		return;
	}

	/* Warning: Only single transparent color supported */
	LTDC_LxCR(num) |= LTDC_LxCR_COLKEY_ENABLE;
	LTDC_LxCKCR(num) = layer->transparent.data[0];
}

static void layer_set(struct lcd_tft *lt, unsigned index,
				const struct lcd_tft_layer *layer)
{
	(void) lt;

	unsigned num = index + 1;

	layer_window(num, layer);
	layer_data(num, layer);
	layer_clut(num, layer);
	layer_key_color(num, layer);
	LTDC_LxCR(num) |= LTDC_LxCR_LAYER_ENABLE;

	/* Reload immediate */
	LTDC_SRCR = LTDC_SRCR_IMR;
}

static void layer_enable(struct lcd_tft *lt, unsigned index, bool enable)
{
	(void) lt;

	unsigned num = index + 1;

	if (enable) {
		LTDC_LxCR(num) |= LTDC_LxCR_LAYER_ENABLE;
	} else {
		LTDC_LxCR(num) &= ~LTDC_LxCR_LAYER_ENABLE;
	}
}
