/*
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
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
//* this header should provide CMSIS headers detection, to achieve compatibility on ones vs unicore headers

#ifndef UNICOREMX_CMSIS_COMPAT_H
#define UNICOREMX_CMSIS_COMPAT_H



#ifndef __CMSIS_USE
#if defined(__STM32F0xx_CMSIS_DEVICE_VERSION) \
            || defined(__STM32F1xx_CMSIS_DEVICE_VERSION)\
            || defined(__STM32F2xx_CMSIS_DEVICE_VERSION)\
            || defined(__STM32F3xx_CMSIS_DEVICE_VERSION)\
            || defined(__STM32F4xx_CMSIS_DEVICE_VERSION)\
            || defined(__STM32F7xx_CMSIS_DEVICE_VERSION)
#define __CMSIS_USE	1
#else
#define __CMSIS_USE	0
#endif
#endif

#if defined(HAL_MODULE_ENABLED) || defined(HAL_GPIO_MODULE_ENABLED)
#define __HAL_USE	1
#else
#define __HAL_USE	0
#endif


#endif
