/*
 *  TOPPERS/FMP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Flexible MultiProcessor Kernel
 *
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2006-2017 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2007-2010 by Industrial Technology Institute,
 *              Miyagi Prefectural Government, JAPAN
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
 *  @(#) $Id: target_serial.c 1218 2017-04-25 07:05:23Z ertl-honda $  
 */

/*
 *  This file contains calls to the Xilinx UART driver (xuartps.c)
 *  in order to implement the FMP serial input/output (SIO) interface.
 */
#include <kernel.h>
#include <t_syslog.h>
#include "target_serial.h"
#include "target_syssvc.h"

/*
 * Configuration constants (delete comment out if necessary)
 */
//#define XUARTPS_POLLING_MODE /* use polling instead of interrupts */

/*
 * UART Registers
 */
#define XUARTPS_CR_REG     (0x00)   /* Control Register */
#define     XUARTPS_CR_STOPBRK          0x00000100   /* Stop TX break */
#define     XUARTPS_CR_TX_EN            0x00000010   /* TX enabled */
#define     XUARTPS_CR_RX_EN            0x00000004   /* RX enabled */
#define     XUARTPS_CR_TX_DIS           0x00000020   /* TX disabled. */
#define     XUARTPS_CR_RX_DIS           0x00000008   /* RX disabled. */
#define     XUARTPS_CR_TXRST            0x00000002   /* TX logic reset */
#define     XUARTPS_CR_RXRST            0x00000001   /* RX logic reset */

#define XUARTPS_MR_REG     (0x04)   /* Mode Register */
#define     XUARTPS_MR_STOPMODE_1_BIT   0x00000000   /* 1 stop bit */
#define     XUARTPS_MR_PARITY_NONE      0x00000020   /* No parity mode */
#define     XUARTPS_MR_CHARLEN_8_BIT    0x00000000   /* 8 bits data */
#define     XUARTPS_MR_CLKSEL           0x00000001   /* Input clock select */
#define     XUARTPS_MR_CCLK             0x00000400   /* Input clock select */

#define XUARTPS_IER_REG    (0x08)   /* Interrupt Enable */
#define XUARTPS_IDR_REG    (0x0C)   /* Interrupt Disable */
#define XUARTPS_ISR_REG    (0x14)   /* Interrupt Status */
#define    XUARTPS_IXR_TXEMPTY          0x00000008   /* TX FIFO empty int */
#define    XUARTPS_IXR_RXTRIG           0x00000001   /* RX FIFO trigger int */

#define XUARTPS_BAUDGEN_REG (0x18)  /* Baud Rate Generator */
#define XUARTPS_BAUDDIV_REG (0x34)  /* Baud Rate Divider */

#define XUARTPS_RXTOUT_REG  (0x1C)  /* RX Timeout [7:0] */

#define XUARTPS_SR_REG      (0x2C)  /* Channel Status */
#define         XUARTPS_SR_TXFULL       0x00000010   /* TX FIFO full */
#define         XUARTPS_SR_RXEMPTY      0x00000002   /* RX FIFO empty */

#define XUARTPS_FIFO_REG    (0x30)  /* FIFO */
#define XUARTPS_RXWM_REG    (0x20)  /* RX FIFO Trigger */

#define UARTREG(x,y) *((volatile uint32_t *)(x->uart_base_address + y))
#define REG(x) *((volatile uint32_t *)(x))

/*
 * Module variables
 */
struct sio_port_control_block {
	uint32_t uart_base_address;
	uint32_t irq_number;
	intptr_t exinf;
};

SIOPCB siopcb_table[TNUM_SIOP] = {
	{
		.uart_base_address = (uint32_t)UART0_BASE,
		.irq_number        = (uint32_t)INHNO_SIO0
	  },
#if TNUM_PORT >= 2
	{
		.uart_base_address = (uint32_t)UART1_BASE,
		.irq_number        = (uint32_t)INHNO_SIO1
	  }
#endif /* TNUM_PORT >= 2 */
};

#define INDEX_SIOP(siopid)  ((uint_t)((siopid) - 1))
#define get_siopcb(siopid)  (&(siopcb_table[INDEX_SIOP(siopid)]))

#ifdef G_SYSLOG
#define my_siopid        1
#else
#define my_siopid        (x_prc_index() + 1)
#endif

/*
 * AUXILIAR FUNCTIONS
 */

/*
 *  Is there a received character?
 */
Inline bool_t
xuartps_getready(SIOPCB *p_siopcb)
{
	return (UARTREG(p_siopcb, XUARTPS_SR_REG) & XUARTPS_SR_RXEMPTY) == 0;
}

/*
 *  Can I send a new character?
 */
Inline bool_t
xuartps_putready(SIOPCB *p_siopcb)
{
	return (UARTREG(p_siopcb, XUARTPS_SR_REG) & XUARTPS_SR_TXFULL) == 0;
}

/*
 *  Get character from buffer
 */
Inline char
xuartps_getchar(SIOPCB *p_siopcb)
{
	return (char)UARTREG(p_siopcb, XUARTPS_FIFO_REG);
}

/*
 * Write character to buffer
 */
Inline void
xuartps_putchar(SIOPCB *p_siopcb, char c)
{
	UARTREG(p_siopcb, XUARTPS_FIFO_REG) = (uint32_t)c;
}


#ifndef XUARTPS_POLLING_MODE
/*
 *  Enable send interrupts
 */
Inline void
xuartps_enable_send(SIOPCB *p_siopcb)
{
	UARTREG(p_siopcb, XUARTPS_IER_REG) = XUARTPS_IXR_TXEMPTY;
}

/*
 *  Diable send interrupts
 */
Inline void
xuartps_disable_send(SIOPCB *p_siopcb)
{
	UARTREG(p_siopcb, XUARTPS_IDR_REG) = XUARTPS_IXR_TXEMPTY;
}

/*
 *  Enable receive interrupts
 */
Inline void
xuartps_enable_rcv(SIOPCB *p_siopcb)
{
	UARTREG(p_siopcb, XUARTPS_IER_REG) = XUARTPS_IXR_RXTRIG;
}

/*
 *  Disable receive interrupts
 */
Inline void
xuartps_disable_rcv(SIOPCB *p_siopcb)
{
	UARTREG(p_siopcb, XUARTPS_IDR_REG) = XUARTPS_IXR_RXTRIG;
}
#endif /* XUARTPS_POLLING_MODE */

/*
 * EARLY USAGE FUNCTIONS
 */

/*
 *  Low-level polling
 */
void
xuartps_pol_putc(char c)
{
	SIOPCB *p_siopcb;

	/* Get the uart base address */
	p_siopcb = get_siopcb(my_siopid);

	while (!xuartps_putready(p_siopcb));
	xuartps_putchar(p_siopcb, c);
}

/*
 *  Initialize the UART
 */
#define SLCR_UNLOCK   0xF8000008
#define SLCR_LOCK     0xF8000004
#define APER_CLK_CTRL 0xF800012C
#define UART_CLK_CTRL 0xF8000154
#define MIO_PIN_10    0xF8000728
#define MIO_PIN_11    0xF800072C

void
xuartps_init()
{
	SIOPCB *p_siopcb;

	/* Get the uart base address */
	p_siopcb = get_siopcb(my_siopid);

#ifdef UART0_MIO10_11
	if (p_siopcb->uart_base_address == ZYNQ_UART0_BASE) {
		/*
		 * Map the UART0 to MIO 10-11
		 */
		REG(SLCR_UNLOCK)   = 0xDF0D;
		REG(APER_CLK_CTRL) = REG(APER_CLK_CTRL) | 0x00100000;
		REG(UART_CLK_CTRL) = REG(UART_CLK_CTRL) | 0x1;

		/*
		 * TRI_ENABLE=1
		 * PULLUP=1
		 * IO_Type=011(LVCMOS33=3.3V)
		 * MIO_PIN_10 => L3_SEL=111(UART 0 RxD, Input)
		 * MIO_PIN_11 => L3_SEL=111(UART 0 TxD, Output)
		 */
		REG(MIO_PIN_10) = 0x6E1;
		REG(MIO_PIN_11) = 0x16E0;
		REG(SLCR_LOCK)  = 0x767B;
	}
#endif /* UART0_MIO10_11 */

	/* Disable all interrupts */
	UARTREG(p_siopcb, XUARTPS_IDR_REG) = 0x00001FFF;

	/* Clear pending interrupts */
	UARTREG(p_siopcb, XUARTPS_ISR_REG) = UARTREG(p_siopcb,
												 XUARTPS_ISR_REG);

	/* Reset the UART and disable TX/RX*/
	UARTREG(p_siopcb, XUARTPS_CR_REG) = XUARTPS_CR_TX_DIS |
	  XUARTPS_CR_RX_DIS |
		XUARTPS_CR_TXRST  |
		  XUARTPS_CR_RXRST;

	/* Set 115200bps baud rate */
	UARTREG(p_siopcb, XUARTPS_BAUDGEN_REG) = UART_BAUD_115K;
	UARTREG(p_siopcb, XUARTPS_BAUDDIV_REG) = UART_BAUDDIV_115K;

	// Set 8n1 (8 bits - No parity - 1 Stop bit)
	UARTREG(p_siopcb, XUARTPS_MR_REG) = XUARTPS_MR_CHARLEN_8_BIT |
	  XUARTPS_MR_PARITY_NONE   |
		XUARTPS_MR_STOPMODE_1_BIT;

	// Set Rx trigger (1 byte)
	UARTREG(p_siopcb, XUARTPS_RXWM_REG) = 0x01;

	// Set timeout (Nx4bits-time)
	UARTREG(p_siopcb, XUARTPS_RXTOUT_REG) = 10;

	// Enable transmission/reception
	UARTREG(p_siopcb, XUARTPS_CR_REG) = XUARTPS_CR_TX_EN |
	  XUARTPS_CR_RX_EN |
		XUARTPS_CR_STOPBRK;
}

/*
 * SIO INTERFACE IMPLEMENTATION
 */

/*
 * Open the serial port
 */
SIOPCB *
sio_opn_por(ID siopid, intptr_t exinf)
{
	ER ercd;
	SIOPCB  *p_siopcb;

	p_siopcb = get_siopcb(siopid);
	p_siopcb->exinf = exinf;

	ercd = ena_int(p_siopcb->irq_number);
	assert(ercd == E_OK);

	return p_siopcb;
}

/*
 * Close the serial port
 */
void
sio_cls_por(SIOPCB *p_siopcb)
{
	ER ercd;

	// Disable transmission/reception
	UARTREG(p_siopcb, XUARTPS_CR_REG) =
	  XUARTPS_CR_STOPBRK | XUARTPS_CR_TX_DIS | XUARTPS_CR_RX_DIS;

	// Disable all interrupts
	UARTREG(p_siopcb, XUARTPS_IDR_REG) = 0x00001FFF;
	ercd = dis_int(p_siopcb->irq_number);
	assert(ercd == E_OK);
}

/*
 *  SIO interrupt service routine (used in target_serial.cfg)
 */
void
sio_isr(intptr_t exinf)
{
#ifndef XUARTPS_POLLING_MODE
	SIOPCB *p_siopcb = get_siopcb(exinf);
	/*
	 * Perform callbacks to the serial module if a transmission
	 * finished or some characters were received
	 */
	if (xuartps_putready(p_siopcb)) {
	  sio_irdy_snd(p_siopcb->exinf);
	}

	if (xuartps_getready(p_siopcb)) {
	  sio_irdy_rcv(p_siopcb->exinf);
	}

	/* clear pending interrupts */
	UARTREG(p_siopcb, XUARTPS_ISR_REG) = UARTREG(p_siopcb,
												 XUARTPS_ISR_REG);
#endif /* XUARTPS_POLLING_MODE */
}

/*
 * Send a character through the serial port
 */
bool_t
sio_snd_chr(SIOPCB *siopcb, char c)
{
#ifndef XUARTPS_POLLING_MODE
	if (!xuartps_putready(siopcb)) return false;
#else /* XUARTPS_POLLING_MODE */
	while(!xuartps_putready(siopcb));
#endif /* XUARTPS_POLLING_MODE */

	xuartps_putchar(siopcb, c);
	return true;
}

/*
 * Receive a character through the serial port
 */
int_t
sio_rcv_chr(SIOPCB *siopcb)
{
#ifndef XUARTPS_POLLING_MODE
	if (!xuartps_getready(siopcb)) return -1;
#else /* XUARTPS_POLLING_MODE */
	while(!xuartps_getready(siopcb));
#endif /* XUARTPS_POLLING_MODE */

	return (int_t)(uint8_t)xuartps_getchar(siopcb);
}

/*
 *  Enable callbacks from the serial port hardware when a character is
 *  received (cbrtn=SIO_RDY_RCV) or sent (cbrtn==SIO_RDY_SND).
 */
void
sio_ena_cbr(SIOPCB *siopcb, uint_t cbrtn)
{
#ifndef XUARTPS_POLLING_MODE
	if (cbrtn == SIO_RDY_SND) {
	  xuartps_enable_send(siopcb);
	}
	else if (cbrtn == SIO_RDY_RCV) {
		xuartps_enable_rcv(siopcb);
	}
#endif /* XUARTPS_POLLING_MODE */
}

/*
 *  Disable callbacks from the serial port hardware.
 */
void
sio_dis_cbr(SIOPCB *siopcb, uint_t cbrtn)
{
#ifndef XUARTPS_POLLING_MODE
	if (cbrtn == SIO_RDY_SND) {
		xuartps_disable_send(siopcb);
	}
	else if (cbrtn == SIO_RDY_RCV) {
		xuartps_disable_rcv(siopcb);
	}
#endif /* XUARTPS_POLLING_MODE */
}
