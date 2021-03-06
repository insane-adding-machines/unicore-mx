/*
 * This file is part of the unicore-mx project.
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

#ifndef NRF_COMMON_PERIPH_H
#define NRF_COMMON_PERIPH_H

#include <unicore-mx/cm3/common.h>
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/nrf/memorymap.h>

/* Common Peripheral Interface.
 * The implementation only applies to peripherals on APB
 * bus, which for this part excludes only GPIO.
 */

/* Peripheral IDs
 *
 * For peripherals on the APB bus there is a direct relationship between its ID
 * and its base address. A peripheral with base address 0x40000000 is therefore
 * assigned ID=0, and a peripheral with base address 0x40001000 is assigned
 * ID=1. The peripheral with base address 0x4001F000 is assigned ID=31
 */

#define PERIPH_CLOCK_ID            (0x00)
#define PERIPH_POWER_ID            (0x00)
#define PERIPH_MPU_ID              (0x00)
#define PERIPH_RADIO_ID            (0x01)
#define PERIPH_UART_ID             (0x02)
#define PERIPH_SPI0_ID             (0x03)
#define PERIPH_TWI0_ID             (0x03)
#define PERIPH_I2C0_ID             (0x03)
#define PERIPH_SPI1_ID             (0x04)
#define PERIPH_SPIS1_ID            (0x04)
#define PERIPH_TWI1_ID             (0x04)
#define PERIPH_I2C1_ID             (0x04)
#define PERIPH_GPIOTE_ID           (0x06)
#define PERIPH_ADC_ID              (0x07)
#define PERIPH_TIMER0_ID           (0x08)
#define PERIPH_TIMER1_ID           (0x09)
#define PERIPH_TIMER2_ID           (0x0a)
#define PERIPH_RTC0_ID             (0x0b)
#define PERIPH_TEMP_ID             (0x0c)
#define PERIPH_RNG_ID              (0x0d)
#define PERIPH_ECB_ID              (0x0e)
#define PERIPH_AAR_ID              (0x0f)
#define PERIPH_CCM_ID              (0x0f)
#define PERIPH_WDT_ID              (0x10)
#define PERIPH_RTC1_ID             (0x11)
#define PERIPH_QDEC_ID             (0x12)
#define PERIPH_LPCOMP_ID           (0x13)
#define PERIPH_SWI0_ID             (0x14)
#define PERIPH_SWI1_ID             (0x15)
#define PERIPH_SWI2_ID             (0x16)
#define PERIPH_SWI3_ID             (0x17)
#define PERIPH_SWI4_ID             (0x18)
#define PERIPH_SWI5_ID             (0x19)
#define PERIPH_NVMC_ID             (0x1e)
#define PERIPH_PPI_ID              (0x1f)

#define periph_base_from_id(periph_id)       (ABP_BASE + 0x1000 * (periph_id))
#define periph_id_from_base(base)            (((base) - APB_BASE) >> 12)
#define periph_base_from_reg(reg)            (((uint32_t)&reg) & 0xfffff000)

/*
 * Tasks are used to trigger actions in a peripheral, for example, to start a
 * particular behavior. A peripheral can implement multiple tasks with each
 * task having a separate register in that peripheral's task register group.
 *
 * A task is triggered when firmware writes a '1' to the task register or when
 * the peripheral itself, or another peripheral, toggles the corresponding task
 * signal.
 */

/** Starting address of all the tasks in the peripheral. */
#define PERIPH_TASK_OFFSET         (0x000)

/*
 * Events are used to notify peripherals and the CPU about events that have
 * happened, for example, a state change in a peripheral. A peripheral may
 * generate multiple events with each event having a separate register in that
 * peripheral’s event register group.  An event is generated when the
 * peripheral itself toggles the corresponding event signal, whereupon the
 * event register is updated to reflect that the event has been generated.
 */

/** Starting address of all the events in the peripheral. */
#define PERIPH_EVENT_OFFSET        (0x100)

#define periph_trigger_task(task)  (task) = (1)

/* All peripherals on the APB bus support interrupts. A peripheral only
 * occupies one interrupt, and the interrupt number follows the peripheral ID,
 * for example, the peripheral with ID=4 is connected to interrupt number 4 in
 * the Nested Vector Interrupt Controller (NVIC).
 */

#define periph_enable_irq(base)    nvic_enable_irq(periph_id_from_base(base))
#define periph_disable_irq(base)   nvic_disable_irq(periph_id_from_base(base))

/* Common regisgers. Not all peripherals have these registers, but when they
 * are present, they are at this offset.
 */
#define PERIPH_SHORTS_OFFSET       (0x200)
#define PERIPH_INTEN_OFFSET        (0x300)
#define PERIPH_INTENSET_OFFSET     (0x304)
#define PERIPH_INTENCLR_OFFSET     (0x308)

#define periph_shorts(base)        MMIO32((base) + PERIPH_SHORTS_OFFSET)
#define periph_inten(base)         MMIO32((base) + PERIPH_INTEN_OFFSET)
#define periph_intenset(base)      MMIO32((base) + PERIPH_INTENSET_OFFSET)
#define periph_intenclr(base)      MMIO32((base) + PERIPH_INTENCLR_OFFSET)

#define periph_enable_shorts(base, shorts)    periph_shorts(base) |= (shorts)
#define periph_disable_shorts(base, shorts)   periph_shorts(base) &= (~(shorts))
#define periph_clear_shorts(base)             periph_shorts(base) = (0)

#define periph_enable_interrupts(base, mask)     periph_intenset(base) |= (mask)
#define periph_disable_interrupts(base, mask)    periph_intenclr(base) = (mask)
#define periph_clear_interrupts(base)            periph_intenclr(base) = (0xffffffff)

/* Each event implemented in the peripheral is associated with a specific bit
 * position in the INTEN, INTENSET and INTENCLR registers. The correct bit
 * position can be derived from the event's address. The event on address 0x100
 * is associated with bit 0 in the INTEN register, the event at address 0x104
 * is associated with bit 1, and so on. The event at address 0x17C is
 * identified with bit 31 in the INTEN register. This pattern effectively
 * limits the maximum number of events in a peripheral to 32.
 */
#define periph_event_inten_bit(event)            ((((uint32_t)&event) & 0xff) >> 2)

#define periph_enable_event_interrupt(event)     periph_enable_interrupts(periph_base_from_reg(event), periph_event_inten_bit(event))
#define periph_disable_event_interrupt(event)    periph_disable_interrupts(periph_base_from_reg(event), periph_event_inten_bit(event))
#define periph_clear_event(event)                (event) = (0)

#define periph_wait_event(event) do {\
    while(!event);\
    periph_clear_event(event);\
} while (0)

#endif  /* NRF_PERIPH_H */
