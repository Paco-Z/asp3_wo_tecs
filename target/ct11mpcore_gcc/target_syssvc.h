/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2005-2016 by Embedded and Real-Time Systems Laboratory
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
 *  $Id: target_syssvc.h 698 2016-03-26 08:48:12Z ertl-hiro $
 */

/*
 *  システムサービスのターゲット依存部（CT11MPCORE用）
 *
 *  システムサービスのターゲット依存部のヘッダファイル．このファイルの
 *  内容は，コンポーネント記述ファイルに記述され，このファイルは無くな
 *  る見込み．
 */

#ifndef TOPPERS_TARGET_SYSSVC_H
#define TOPPERS_TARGET_SYSSVC_H

#include "ct11mpcore.h"

/*
 *  トレースログに関する設定
 */
#ifdef TOPPERS_ENABLE_TRACE
#include "arch/logtrace/trace_config.h"
#endif /* TOPPERS_ENABLE_TRACE */

/*
 *  起動メッセージのターゲットシステム名
 */
#define TARGET_NAME    "ARM CT11MPCore"

/*
 *  システムログの低レベル出力のための文字出力
 *
 *  ターゲット依存の方法で，文字cを表示/出力/保存する．
 */
extern void	target_fput_log(char c);

/*
 *  サポートするシリアルポートの数
 */
#define TNUM_PORT        1        

/*
 *  SIOドライバで使用するUARTに関する指定
 */
#if defined(CORE0)
#define SIO_UART_BASE	EB_UART0_BASE
#define SIO_UART_IRQNO	EB_IRQNO_UART0
#elif defined(CORE1)
#define SIO_UART_BASE	EB_UART1_BASE
#define SIO_UART_IRQNO	EB_IRQNO_UART1
#elif defined(CORE2)
#define SIO_UART_BASE	EB_UART2_BASE
#define SIO_UART_IRQNO	EB_IRQNO_UART2
#elif defined(CORE3)
#define SIO_UART_BASE	EB_UART3_BASE
#define SIO_UART_IRQNO	EB_IRQNO_UART3
#endif

#define UART_IBRD_DEF	EB_UART_IBRD_38400
#define UART_FBRD_DEF	EB_UART_FBRD_38400
#define UART_LCR_H_DEF	UART_LCR_H_WLEN8

#endif /* TOPPERS_TARGET_SYSSVC_H */
