/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2005-2018 by Embedded and Real-Time Systems Laboratory
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
 *  $Id: target_serial.c 963 2018-05-01 00:51:38Z ertl-hiro $
 */

/*
 *		シリアルインタフェースドライバのターゲット依存部（CT11MPCore用）
 */

#include <kernel.h>
#include <t_syslog.h>
#include "target_serial.h"

/*
 *  SIOドライバの初期化
 */
void
sio_initialize(intptr_t exinf)
{
	uart_pl011_initialize();
}

/*
 *  SIOドライバの終了処理
 */
void
sio_terminate(intptr_t exinf)
{
	/* 何もしない */
}

/*
 *  SIOの割込みサービスルーチン
 */
void
sio_isr(intptr_t exinf)
{
	uart_pl011_isr();
}

/*
 *  SIOポートのオープン
 */
SIOPCB *
sio_opn_por(ID siopid, intptr_t exinf)
{
	SIOPCB	*p_siopcb;

	/*
	 *  デバイス依存のオープン処理
	 */
	p_siopcb = uart_pl011_opn_por(siopid, exinf);

	/*
	 *  SIOの割込みマスクを解除する．
	 */
	(void) ena_int(INTNO_SIO);
	return(p_siopcb);
}

/*
 *  SIOポートのクローズ
 */
void
sio_cls_por(SIOPCB *p_siopcb)
{
	/*
	 *  デバイス依存のクローズ処理
	 */
	uart_pl011_cls_por(p_siopcb);

	/*
	 *  SIOの割込みをマスクする．
	 */
	(void) dis_int(INTNO_SIO);
}

/*
 *  SIOポートへの文字送信
 */
bool_t
sio_snd_chr(SIOPCB *siopcb, char c)
{
	return(uart_pl011_snd_chr(siopcb, c));
}

/*
 *  SIOポートからの文字受信
 */
int_t
sio_rcv_chr(SIOPCB *siopcb)
{
	return(uart_pl011_rcv_chr(siopcb));
}

/*
 *  SIOポートからのコールバックの許可
 */
void
sio_ena_cbr(SIOPCB *siopcb, uint_t cbrtn)
{
	uart_pl011_ena_cbr(siopcb, cbrtn);
}

/*
 *  SIOポートからのコールバックの禁止
 */
void
sio_dis_cbr(SIOPCB *siopcb, uint_t cbrtn)
{
	uart_pl011_dis_cbr(siopcb, cbrtn);
}

/*
 *  SIOポートからの送信可能コールバック
 */
void
uart_pl011_irdy_snd(intptr_t exinf)
{
	sio_irdy_snd(exinf);
}

/*
 *  SIOポートからの受信通知コールバック
 */
void
uart_pl011_irdy_rcv(intptr_t exinf)
{
	sio_irdy_rcv(exinf);
}

/*
 *		システムログの低レベル出力（本来は別のファイルにすべき）
 */

/*
 *  低レベル出力用のSIOポート管理ブロック
 */
static SIOPCB	*p_siopcb_target_fput;

/*
 *  SIOポートの初期化
 */
void
target_fput_initialize(void)
{
	p_siopcb_target_fput = uart_pl011_opn_por(SIOPID_FPUT, 0);
}

/*
 *  SIOポートへのポーリング出力
 */
static void
ct11mpcore_uart_fput(char c)
{
	/*
	 *  送信できるまでポーリング
	 */
	while (!(uart_pl011_snd_chr(p_siopcb_target_fput, c))) ;
}

/*
 *  SIOポートへの文字出力
 */
void
target_fput_log(char c)
{
	if (c == '\n') {
		ct11mpcore_uart_fput('\r');
	}
	ct11mpcore_uart_fput(c);
}
