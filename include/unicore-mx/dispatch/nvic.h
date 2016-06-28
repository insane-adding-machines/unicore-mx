#ifndef UNICOREMX_NVIC_H
#error You should not be including this file directly, but <unicore-mx/cm3/nvic.h>
#endif

#if defined(STM32F0)
#	include <unicore-mx/stm32/f0/nvic.h>
#elif defined(STM32F1)
#	include <unicore-mx/stm32/f1/nvic.h>
#elif defined(STM32F2)
#	include <unicore-mx/stm32/f2/nvic.h>
#elif defined(STM32F3)
#	include <unicore-mx/stm32/f3/nvic.h>
#elif defined(STM32F4)
#	include <unicore-mx/stm32/f4/nvic.h>
#elif defined(STM32F7)
#	include <unicore-mx/stm32/f7/nvic.h>
#elif defined(STM32L0)
#	include <unicore-mx/stm32/l0/nvic.h>
#elif defined(STM32L1)
#	include <unicore-mx/stm32/l1/nvic.h>
#elif defined(STM32L4)
#	include <unicore-mx/stm32/l4/nvic.h>

#elif defined(EFM32TG)
#	include <unicore-mx/efm32/tg/nvic.h>
#elif defined(EFM32G)
#	include <unicore-mx/efm32/g/nvic.h>
#elif defined(EFM32LG)
#	include <unicore-mx/efm32/lg/nvic.h>
#elif defined(EFM32GG)
#	include <unicore-mx/efm32/gg/nvic.h>

#elif defined(LPC13XX)
#	include <unicore-mx/lpc13xx/nvic.h>
#elif defined(LPC17XX)
#	include <unicore-mx/lpc17xx/nvic.h>
#elif defined(LPC43XX_M4)
#	include <unicore-mx/lpc43xx/m4/nvic.h>
#elif defined(LPC43XX_M0)
#	include <unicore-mx/lpc43xx/m0/nvic.h>

#elif defined(SAM3A)
#	include <unicore-mx/sam/3a/nvic.h>
#elif defined(SAM3N)
#	include <unicore-mx/sam/3n/nvic.h>
#elif defined(SAM3S)
#	include <unicore-mx/sam/3s/nvic.h>
#elif defined(SAM3U)
#	include <unicore-mx/sam/3u/nvic.h>
#elif defined(SAM3X)
#	include <unicore-mx/sam/3x/nvic.h>
#elif defined(SAM4L)
#	include <unicore-mx/sam/4l/nvic.h>

#elif defined(LM3S) || defined(LM4F)
/* Yes, we use the same interrupt table for both LM3S and LM4F */
#	include <unicore-mx/lm3s/nvic.h>

#elif defined(VF6XX)
#	include <unicore-mx/vf6xx/nvic.h>

#else
#	warning"no interrupts defined for chipset; NVIC_IRQ_COUNT = 0"

#define NVIC_IRQ_COUNT 0

#endif
