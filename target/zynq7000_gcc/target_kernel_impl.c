/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2007-2018 by Embedded and Real-Time Systems Laboratory
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
 *  $Id: target_kernel_impl.c 969 2018-05-02 09:32:30Z ertl-honda $
 */

/*
 *		カーネルのターゲット依存部（Zynq7000用）
 */

#include "kernel_impl.h"
#include <sil.h>
#include "arm.h"
#include "pl310.h"

/*
 *  カーネル動作時のメモリマップと関連する定義
 *
 *  0x00000000 - 0x3FFFFFFF : DDR（1GB）実装DDRはターゲット毎に異なる
 *  0x40000000 - 0xBFFFFFFF : PL
 *  0xE0000000 - 0xFFFFFFFF : Peripheral + etc
 */

/*
 *  MMUへの設定属性（第1レベルディスクリプタ）
 */
#define MMU_ATTR_RAM	(ARM_MMU_DSCR1_SHARED|ARM_MMU_DSCR1_TEX001 \
							|ARMV6_MMU_DSCR1_AP011|ARM_MMU_DSCR1_CB11)
#define MMU_ATTR_IODEV	(ARM_MMU_DSCR1_SHARED|ARM_MMU_DSCR1_TEX000 \
							|ARMV6_MMU_DSCR1_AP011|ARM_MMU_DSCR1_CB01 \
							|ARMV6_MMU_DSCR1_NOEXEC)

/*
 *  DDR領域の属性（先頭番地とサイズはzynq7000.hで定義)
 */
#define DDR_ATTR		MMU_ATTR_RAM

/*
 *  PL領域の先頭番地とサイズ
 */
#define PL_ADDR		0x40000000
#define PL_SIZE		0x80000000	/* 2GB */
#define PL_ATTR		MMU_ATTR_IODEV

/*
 *  PERI領域の先頭番地とサイズ
 */
#define PERI_ADDR		0xE0000000
#define PERI_SIZE		0x20000000	/* 512MB */
#define PERI_ATTR		MMU_ATTR_IODEV


/*
 *  MMUの設定情報（メモリエリアの情報）
 */
ARM_MMU_CONFIG arm_memory_area[] = {
	{ DDR_ADDR, DDR_ADDR, DDR_SIZE, DDR_ATTR },
	{ PL_ADDR, PL_ADDR, PL_SIZE, PL_ATTR },
	{ PERI_ADDR, PERI_ADDR, PERI_SIZE, PERI_ATTR }

};

/*
 *  MMUの設定情報の数（メモリエリアの数）
 */
const uint_t arm_tnum_memory_area
					= sizeof(arm_memory_area) / sizeof(ARM_MMU_CONFIG);

/*
 *  システムログの低レベル出力のための初期化
 */

#ifndef TOPPERS_OMIT_TECS
/*
 *  セルタイプtPutLogZynq7000内に実装されている関数を直接呼び出す．
 */
extern void	tPutLogZynq7000_initialize(void);

#else /* TOPPERS_OMIT_TECS */

extern void	sio_initialize(void);
extern void	target_fput_initialize(void);

#endif /* TOPPERS_OMIT_TECS */

/*
 *  OS起動時の初期化
 */
void
hardware_init_hook(void)
{
	arm_disable_dcache();
	pl310_disable();
	arm_disable_icache();
}

/*
 *  ターゲット依存の初期化
 */
void
target_initialize(void)
{
	extern void	*vector_table;		/* ベクタテーブル */

	/*
	 *  チップ依存の初期化
	 */
	chip_initialize();

	/*
	 *  ベクタテーブルの設定
	 */
	CP15_WRITE_VBAR((uint32_t) &vector_table);

	/*
	 *  L2キャッシュコントローラ（PL310）の初期化
	 */
	pl310_initialize(0x0U, ~0x0U);

	/*
	 *  UARTを初期化
	 */
#ifndef TOPPERS_OMIT_TECS
	tPutLogZynq7000_initialize();
#else /* TOPPERS_OMIT_TECS */
	sio_initialize();
	target_fput_initialize();
#endif /* TOPPERS_OMIT_TECS */
}

/*
 *  ターゲット依存の終了処理
 */
void
target_exit(void)
{
	/*
	 *  チップ依存の終了処理
	 */
	chip_terminate();

	/*
	 *  Endless loop
	 */
	while (true) ;
}
