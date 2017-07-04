/*
 *  TOPPERS/FMP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Flexible MultiProcessor Kernel
 *
 *  Copyright (C) 2012-2017 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *
 *  上記著作権者は，以下の(1)〜(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 *
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 *
 *  @(#) $Id: zynq.h 1218 2017-04-25 07:05:23Z ertl-honda $
 */

/*
 *  This file contains a few definitions that are specific to
 *  the ZYNQ ZYBO and ZEDboard, such as the memory address, clock values,
 *  or number of interrupts.
 *
 */
#ifndef TOPPERS_ZYNQ_H
#define TOPPERS_ZYNQ_H

#include <sil.h>

#if defined(BOARD_ZYBO)

/*
 * CPU clocks
 */
#define ZYNQ_CPU_6X4X_MHZ     650U
#define ZYNQ_CPU_3X2X_MHZ     325U
#define ZYNQ_CPU_2X_MHZ       222U
#define ZYNQ_CPU_1X_MHZ       111U

#define ZYNQ_CORE_CLK              ZYNQ_CPU_6X4X_MHZ
#define ZYNQ_PERIPHCLK             ZYNQ_CPU_3X2X_MHZ
#define MPCORE_GTM_CLOCK_FREQ_MHZ  ZYNQ_PERIPHCLK

/*
 * Tmer preescaler and load value to achieve 1us per tick.
 * Note that the preescaler value must be representable with 8 bits;
 * and the load value must be a 32bit value.
 *
 * Private timers and watchdogs are clocked with PERIPHCLK which is 1/2 of
 * the CPU frequency. The formula is:
 *
 *    period = (preescaler + 1)(load + 1) / PERIPHCLK
 *      - period    = 1us
 *      - periphclk = 325MHz
 */
#define MPCORE_TMR_PS_VALUE		64  // preescaler for private watchdog
#define MPCORE_WDG_PS_VALUE		64  // preescaler for private timer

#define TCYC_HRTCNT				858993459		/* floor(2^32/5) */

#define MPCORE_WDG_LR_VALUE		4294967294		/* TCYC_HRTCNT * 5 - 1 */
#define MPCORE_WDG_FREQ			5
#define MPCORE_TMR_FREQ			5

/*
 * Definition of the 115200bps baud rate constants
 *        rate = clk / (CD * (BDIV + 1))
 *         clk = MR.CLKSEL? inclk : inclk/8  (e.g. inclk)
 *       inclk = MR.CCLK? uart_clock : apb clock (e.g. uart_clock=50MHz)
 */
#define UART_BAUD_115K          0x7CU
#define UART_BAUDDIV_115K       0x6U

/*
 * Memory base address and size
 */
#define DDR_ADDR 0x00000000
#define DDR_SIZE 0x20000000 /* 512MB */

/*
 *  UART base address definitions (used in target_serial.c)
 *
 */
#ifdef TOPPERS_NOSAFEG
#define     UART0_BASE  ZYNQ_UART1_BASE
#define     UART1_BASE  ZYNQ_UART0_BASE
#else
#error "SafeG is no supported yet!"
#endif

/*
 *  UART interrupt handler definitions (used in target_serial.cfg)
 *     INHNO: interrupt handler number
 *     INTNO: interrupt number
 *     INTPRI: interrupt priority (lower means higher priority)
 *     INTATR: interrupt attributes (0 means not enabled at the beginning)
 *
 */
#ifdef TOPPERS_NOSAFEG
#define    INHNO_SIO0  ZYNQ_UART1_IRQ
#define    INTNO_SIO0  ZYNQ_UART1_IRQ
#define    INTPRI_SIO0    -3
#define    INTATR_SIO0     0U
#define    INHNO_SIO1  ZYNQ_UART0_IRQ
#define    INTNO_SIO1  ZYNQ_UART0_IRQ
#define    INTPRI_SIO1    -2
#define    INTATR_SIO1     0U
#else
#error "SafeG is no supported yet!"
#endif

#elif defined(BOARD_ARDUZYNQ)

/*
 * CPU clocks
 */
#define ZYNQ_CPU_6X4X_MHZ     666U
#define ZYNQ_CPU_3X2X_MHZ     333U
#define ZYNQ_CPU_2X_MHZ       222U
#define ZYNQ_CPU_1X_MHZ       111U

#define ZYNQ_CORE_CLK              ZYNQ_CPU_6X4X_MHZ
#define ZYNQ_PERIPHCLK             ZYNQ_CPU_3X2X_MHZ
#define MPCORE_GTM_CLOCK_FREQ_MHZ  ZYNQ_PERIPHCLK

/*
 * Tmer preescaler and load value to achieve 1us per tick.
 * Note that the preescaler value must be representable with 8 bits;
 * and the load value must be a 32bit value.
 *  
 * Private timers and watchdogs are clocked with PERIPHCLK which is 1/2 of
 * the CPU frequency. The formula is:
 *
 *    period = (preescaler + 1)(load + 1) / PERIPHCLK
 *      - period    = 1us
 *      - periphclk = 325MHz
 */
#define MPCORE_TMR_PS_VALUE		64  // preescaler for private watchdog
#define MPCORE_WDG_PS_VALUE		64  // preescaler for private timer

#define TCYC_HRTCNT				858993459		/* floor(2^32/5) */

#define MPCORE_WDG_LR_VALUE		4294967294		/* TCYC_HRTCNT * 5 - 1 */
#define MPCORE_WDG_FREQ			5
#define MPCORE_TMR_FREQ			5

/*
 * Definition of the 115200bps baud rate constants
 *        rate = clk / (CD * (BDIV + 1))
 *         clk = MR.CLKSEL? inclk : inclk/8  (e.g. inclk)
 *       inclk = MR.CCLK? uart_clock : apb clock (e.g. uart_clock=50MHz)
 */
#define UART_BAUD_115K          0x7CU
#define UART_BAUDDIV_115K       0x06U

/*
 * Memory base address and size
 */
#define DDR_ADDR 0x00000000
#define DDR_SIZE 0x20000000 /* 512MB */

/*
 * Overun timer prescale value
 * count rate is divided by 2^(N+1)
 * 11 : 1 count  36.9us : max 2,418,300us  
 * 12 : 1 count  73.8us : max 4,836,601us  
 * 13 : 1 count 147.6us : max 9,673,202us
 */
#define OVRTIMER_TIMER_PRESCALE_VAL  13
#define OVRTIMER_TIMER_TICK_NS       148
/*
 *  UART base address definitions (used in target_serial.c)
 *
 */
#ifdef TOPPERS_NOSAFEG
#define     UART0_BASE  ZYNQ_UART0_BASE
#define     UART1_BASE  ZYNQ_UART1_BASE
#else
#error "SafeG is no supported yet!"
#endif

/*
 *  UART interrupt handler definitions (used in target_serial.cfg)
 *     INHNO: interrupt handler number
 *     INTNO: interrupt number
 *     INTPRI: interrupt priority (lower means higher priority)
 *     INTATR: interrupt attributes (0 means not enabled at the beginning)
 *
 */
#ifdef TOPPERS_NOSAFEG
#define    INHNO_SIO0  ZYNQ_UART0_IRQ
#define    INTNO_SIO0  ZYNQ_UART0_IRQ
#define    INTPRI_SIO0    -3
#define    INTATR_SIO0     0U
#define    INHNO_SIO1  ZYNQ_UART1_IRQ
#define    INTNO_SIO1  ZYNQ_UART1_IRQ
#define    INTPRI_SIO1    -2
#define    INTATR_SIO1     0U
#else
#error "SafeG is no supported yet!"
#endif

#else
#error Specifi target board.
#endif 

/*
 *  Common Definition
 */
  
/*
 * Available UARTs
 */
#define ZYNQ_UART0_BASE 0xE0000000
#define ZYNQ_UART1_BASE 0xE0001000
#define ZYNQ_UART0_IRQ  (59)
#define ZYNQ_UART1_IRQ  (82)

/*
 *  MPCore Private Memory Region Base Address (Table 4.7 in ZYNQ manual)
 */
#define MPCORE_PMR_BASE  0xF8F00000

/*
 *  Number of interrupts supported by the GICv1.0 in this board. Note
 *  that these values could be obtained dynamically from the corresponding
 *  GIC register.
 */
#define GIC_TNUM_INTNO		UINT_C(96)

/*
 *  L2(PL310) Base Address
 */
#define PL310_BASE  0xF8F02000

/*
 *  OCM Address
 */
#define OCM_ADDR  0xFFFF0000

/*
 *  Boot address memory
 */
#define BOOT_MEM_ADDR 0xFFFFFFF0

/*
 *  Triple Timer Counters(16bit counter)
 */
#define TTC_CLOCK_CONTROL(id, ch)		((uint32_t *)(0xF8001000 + (1000*id) + 0x00 + (4 * ch)))
#define TTC_COUNTER_CONTROL(id, ch)		((uint32_t *)(0xF8001000 + (1000*id) + 0x0c + (4 * ch)))
#define TTC_COUNTER_VALUE(id, ch)		((uint32_t *)(0xF8001000 + (1000*id) + 0x18 + (4 * ch)))
#define TTC_INTERVAL_COUNTER(id, ch)	((uint32_t *)(0xF8001000 + (1000*id) + 0x24 + (4 * ch)))
#define TTC_MATCH1_COUNTER(id, ch)		((uint32_t *)(0xF8001000 + (1000*id) + 0x30 + (4 * ch)))
#define TTC_MATCH2_COUNTER(id, ch)		((uint32_t *)(0xF8001000 + (1000*id) + 0x3C + (4 * ch)))
#define TTC_MATCH3_COUNTER(id, ch)		((uint32_t *)(0xF8001000 + (1000*id) + 0x48 + (4 * ch)))
#define TTC_INTERRUPT_REGSISTER(id, ch)	((uint32_t *)(0xF8001000 + (1000*id) + 0x54 + (4 * ch)))
#define TTC_INTERRUPT_ENABLE(id, ch)	((uint32_t *)(0xF8001000 + (1000*id) + 0x60 + (4 * ch)))
#define TTC_EVENT_CONTROL_TIMER(id, ch)	((uint32_t *)(0xF8001000 + (1000*id) + 0x6C + (4 * ch)))
#define TTC_EVENT_REGISTER(id, ch)		((uint32_t *)(0xF8001000 + (1000*id) + 0x78 + (4 * ch)))

#define TTC_CLOCK_CONTROL_PS_VAL_OFFSET	0x01
#define TTC_CLOCK_CONTROL_PS_EN			0x01

#define TTC_COUNTER_RST					0x10
#define TTC_COUNTER_DEC					0x04
#define TTC_COUNTER_INT					0x02
#define TTC_COUNTER_DIS					0x01

#define TTC_INTERRUPT_REGSISTER_OVR		0x10

#define TTC_INTERRUPT_OVR				0x10
#define TTC_INTERRUPT_IV				0x01


#define TTC_ID0_CH0_INTNO 42
#define TTC_ID0_CH1_INTNO 43
#define TTC_ID0_CH2_INTNO 43

#define TTC_ID1_CH0_INTNO 69
#define TTC_ID1_CH1_INTNO 70
#define TTC_ID1_CH2_INTNO 71


#endif /* TOPPERS_ZYNQ_H */
