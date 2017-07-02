/*
 *  TOPPERS/FMP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Flexible MultiProcessor Kernel
 *
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2006-2015 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2007-2010 by Industrial Technology Institute,
 *                              Miyagi Prefectural Government, JAPAN
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
 *  @(#) $Id: target_serial.h 1087 2015-02-03 01:04:34Z ertl-honda $
 */

/*
 *  This file contains some definitions for the serial port
 *  interrupt and some function prototypes.
 */
#ifndef TOPPERS_TARGET_SERIAL_H
#define TOPPERS_TARGET_SERIAL_H

#include "zynq7000.h"

/* Serial port control block */
typedef struct sio_port_control_block SIOPCB;

/*
 *  Callback routine flag to distinguish send from receive
 */
#define SIO_RDY_SND    1U   /* send callback */
#define SIO_RDY_RCV    2U   /* receive callback */

#ifndef TOPPERS_MACRO_ONLY

/*
 *  Open the serial port
 */
extern SIOPCB *sio_opn_por(ID siopid, intptr_t exinf);

/*
 *  Close the serial port
 */
extern void sio_cls_por(SIOPCB *p_siopcb);

/*
 *  Interrupt service routine
 */
extern void sio_isr(intptr_t siopid);

/*
 *  Send a character through the serial port
 */
extern bool_t sio_snd_chr(SIOPCB *siopcb, char c);

/*
 *  Receive a character from the serial port
 */
extern int_t sio_rcv_chr(SIOPCB *siopcb);

/*
 *  Enable callbacks (send or receive) from the serial port
 */
extern void sio_ena_cbr(SIOPCB *siopcb, uint_t cbrtn);

/*
 * Disable callbacks (send or receive) from the serial port
 */
extern void sio_dis_cbr(SIOPCB *siopcb, uint_t cbrtn);

/*
 *  Callback called from the serial port when it is ready to send
 */
extern void sio_irdy_snd(intptr_t exinf);

/*
 *  Callback called from the serial port when it is ready to receive
 */
extern void sio_irdy_rcv(intptr_t exinf);

/*
 *  Low-level polling
 */
extern void xuartps_pol_putc(char c);

/*
 *  Initialize the UART
 */
extern void xuartps_init(void);

#endif /* TOPPERS_MACRO_ONLY */
#endif /* TOPPERS_TARGET_SERIAL_H */
