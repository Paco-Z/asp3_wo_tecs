/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2005-2015 by Embedded and Real-Time Systems Laboratory
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
 *  @(#) $Id: core_support.S 323 2015-05-28 08:48:22Z ertl-ishikawa $
 */

/*
 *  プロセッサ依存モジュール アセンブリ言語部（ARM-M用）
 */

#define TOPPERS_MACRO_ONLY
#define UINT_C(val)		(val)		/* uint_t型の定数を作るマクロ */
#define ULONG_C(val)	(val)		/* ulong_t型の定数を作るマクロ */
#define CAST(type, val)	(val)		/* 型キャストを行うマクロ */

#include "kernel_impl.h"
#include "arm_m.h"
#include "offset.h"
#include "target_asm.inc"

/*
 *  タスクディスパッチャ
 */
	ATEXT
	AALIGN(2)
	ATHUMB(_dispatch)
	AGLOBAL(_dispatch)
ALABEL(_dispatch)
#ifdef TOPPERS_SUPPORT_OVRHDR
    push    {lr}
    bl      ovrtimer_stop
    pop     {lr}
#endif /* TOPPERS_SUPPORT_OVRHDR */
	/*
	 *
	 *  このルーチンは，PendSVによって呼び出される
     *  * Handler mode
     *  * use msp
     *  * lr = EXC_RETURN
     *  * scratch registers -> psp
     *  * CONTROL.FPCA -> EXC_RETURN && CONTROL.FPCA == 0
	 */
    cpsid   f                   /* FAULTMASK = 1 */
    /*
     *  PendSV要求をクリア
     *  _dispatch - cpsid f の間でISRが入り，さらにPendSV要求が
     *  入った場合に，dispatcherが無駄に走ることを防ぐため，
     *  ここでPendSV要求をクリアする
     */
	ldr     r1, =0x08000000           /* r1 = ICSR.PENDSVCLR */
	ldr     r2, =0xE000ED04           /* r2 = *ICSR */
    str     r1, [r2]                  /* *ICSR = PENDSVCLR */

    mrs     r3, psp 
	stmfd   r3!, {r4-r11}         /* レジスタの保存 */
#ifdef __TARGET_FPU_VFPUV4_D16
    /*
     * 呼出し元コンテキストで浮動小数点演算をしたか？
     *  EXC_RETURN[4]をチェック
     */
    tst     lr, #0x10
    /*  EXC_RETURN[4] == 1ならばスキップ */
    bne     _dispatch_1
	vstmdb  r3!, {s16-s31}         /* レジスタの保存 */
    /*fpscrは例外フレームに保存済み*/
ALABEL(_dispatch_1)
#endif /* __TARGET_FPU_VFPUV4_D16 */
	ldr   r1, =p_runtsk           /* p_runtskを読み込む */
	ldr   r1, [r1]
	str   r3, [r1,#TCB_sp]        /* タスクのスタックを保存 */
	str   lr, [r1,#TCB_pc]        
    b     dispatcher

/*
 *  CPU例外エントリ
 *
 *  割込みエントリと処理の内容は同等だが，ログの種類が異なるため，
 *  分けている．
 */
	AALIGN(2)
	ATEXT
	ATHUMB(core_exc_entry)
	AGLOBAL(core_exc_entry)
ALABEL(core_exc_entry)
	/*
	 *  例外/割込みが発生すると，発生時にアクティブなスタックにスクラ
	 *  ッチレジスタ等が保存される．
	 *  この内容に加えて，CPU例外ハンドラへの情報として，basepri の値と，
	 *  EXC_RETURNの情報を加えて保存する．basepriの値は，CPU例外からの
	 *  リターン時に割込み優先度マスクの値を元に戻すためにも用いられる．
	 *
	 *   -----------
	 *  | EXC_RETURN|  
	 *   -----------
	 *  |  basepri  |  
	 *   -----------
	 *  |    R0     |  
	 *   -----------
	 *  |    R1     |
	 *   -----------
	 *  |    R2     |
	 *   -----------
	 *  |    R3     |
	 *   -----------
	 *  |    R12    |
	 *   -----------
	 *  |    LR     |
	 *   -----------
	 *  |    PC     |
	 *   -----------
	 *  |   xPSR    |
	 *   -----------
	 *
	 */

	/*
	 *  カーネル管理外の例外かチェック
	 *  カーネル内のクリティカルセクションの実行中，全割込みロック状態，
	 *  CPUロック状態，カーネル管理外の割込みハンドラ実行中のいずれかで
	 *  発生したCPU例外を，カーネル管理外のCPU例外と呼ぶ
	 *  全割込みロック状態はFAULTMASKが'1'の場合
	 *  CPUロック状態はbasepriがIIPM_LOCKかで判断する．
	 */
	mrs   r2, FAULTMASK           /* 全割込みロック状態ならカーネル管理外例外処理へ */
	cbnz  r2, core_nonkernel_exc_entry

	mrs   r2, basepri             /* baepriの値を取得 */
	cmp   r2, #IIPM_LOCK          /* CPUロック状態ならカーネル管理外例外処理へ */
	beq   core_nonkernel_exc_entry


	/*
	 * スタックを変更する必要があるかチェック
	 * EXC_RETURN（割込み時にLRに設定される値）をチェックして，例外発生時に
	 * アクティブなスタックを特定することで多重割込みか判定する．
	 */
	tst   lr, #EXC_RETURN_PSP    /* 割込み元がMSPなら多重割込み */
	beq   core_exc_entry_1       /* 多重割込みならcore_exc_entry_1へ */
	mrs   r0, psp                /* 一段目の割込みの場合はPSP上に */
	stmfd r0!,{r2}               /* 割込み発生時の割込み優先度マスクを積む */
	stmfd r0!,{lr}               /* EXC_RETURN を積む             */
	msr   psp, r0                /* CPU例外ハンドラへの引数となる */
	push  {lr}                   /* MSP上にもEXC_RETURN を積む    */ 
#ifdef TOPPERS_SUPPORT_OVRHDR
    push    {r0}
    bl      ovrtimer_stop
    pop     {r0}
#endif /* TOPPERS_SUPPORT_OVRHDR */
	b     core_exc_entry_2
ALABEL(core_exc_entry_1)         /* 多重割込みの場合 */
	push  {r2}                   /* 割込み発生時の割込み優先度マスクを積む */
	push  {lr}                   /* EXC_RETURN を積む             */ 
	mov   r0, sp                 /* CPU例外ハンドラへの引数となる */

	/*
	 *  共通処理
	 */
ALABEL(core_exc_entry_2)
	mrs   r3, ipsr               /* ハンドラアドレスを取得 */
ALABEL(core_exc_entry_3)
	ldr   r1, =_kernel_exc_tbl
	ldr   r2, [r1, r3, lsl #2]

#ifdef LOG_EXC_ENTER
	push  {r0,r2,r3}
	mov   r0, r3                 /* 例外番号をパラメータに  */
	bl    log_exc_enter          /* log_exc_enterを呼び出す */
	pop   {r0,r2,r3}
	push  {r3}                   /* 例外番号をスタックへ    */
#endif /* LOG_EXC_ENTER */

	/*
	 *  CPU例外ハンドラの呼び出し
	 */
	blx   r2

#ifdef LOG_EXC_ENTER
	pop   {r0}                   /* 例外番号を引数に        */
	bl    log_exc_leave          /* log_exc_leaveを呼び出す */
#endif /* LOG_EXC_ENTER */

	b     ret_exc

/*
 *  カーネル管理外のCPU例外の出入口処理
 */
ALABEL(core_nonkernel_exc_entry)
	tst   lr, #EXC_RETURN_PSP    /* 割込み元がMSPなら多重割込み */
	beq   core_nonkernel_exc_entry_1  /* 多重割込みなら */
	mrs   r0, psp                /* 一段目の割込みの場合はPSP上に */
	stmfd r0!,{r2}               /* 割込み発生時の割込み優先度マスクを積む */
	stmfd r0!,{lr}               /* EXC_RETURN を積む             */
	msr   psp, r0                /* CPU例外ハンドラへの引数となる */
	push  {lr}                   /* MSP上にもEXC_RETURN を積む    */ 
	b     core_nonkernel_exc_entry_2
ALABEL(core_nonkernel_exc_entry_1) /* 多重割込みの場合 */
	push  {r2}                   /* 割込み発生時の割込み優先度マスクを積む */
	push  {lr}                   /* EXC_RETURN を積む             */ 
	mov   r0, sp                 /* CPU例外ハンドラへの引数となる */

ALABEL(core_nonkernel_exc_entry_2)
	mrs   r3, ipsr               /* CPU例外ハンドラのアドレスを取得 */
	ldr   r1, =_kernel_exc_tbl
	ldr   r2, [r1, r3, lsl #2]

	/*
	 *  CPU例外ハンドラの呼び出し
	 */
	blx   r2

	/*
	 *  割込みロック状態とする．
	 */
	cpsid f

	/*
	 *  戻り先のコンテキストの判定
	 * 
	 *  割込みハンドラ実行にLRにセットされるEXC_RETURNをチェックして，戻り
	 *  先でMSPが使われていれば，割込み先が非タスクコンテキストと判定する．
	 */
	pop   {r3}                     /* lrをスタックから取得         */
	tst   r3, #EXC_RETURN_PSP      /* 戻り先がPSPなら              */
	bne   core_nonkernel_ret_exc_1
	pop   {r1}                     /* 元の割込み優先度マスク(basepri) */
	b     core_nonkernel_ret_exc_2 /* の値をMSPから取得 */

ALABEL(core_nonkernel_ret_exc_1)
	/*
	 *  PSP上からEXC_RETURNを削除
	 */
	mrs   r2, psp
	add   r2, r2, #4
	/*
	 *  元の割込み優先度マスク(basepri)の値をPSPから取得
	 */
	ldmfd r2!, {r1} 
	msr   psp, r2

ALABEL(core_nonkernel_ret_exc_2)
	msr   basepri, r1             /* 割込み優先度マスクを割込み前に状態へ */
	bx    r3                      /* リターン */

/*
 *  割込みエントリ
 */
	ATHUMB(core_int_entry)
	AGLOBAL(core_int_entry)
ALABEL(core_int_entry)
	/*
	 *  割込み発生時の割込み優先度マスクをスタックに保存するため取得
	 */
	mrs   r2, basepri             /* baepriの値を取得 */

	/*
	 * 多重割込みかチェック
	 * EXC_RETURN（割込み時にLRに設定される値）をチェックして，例外発生時に
	 * アクティブなスタックを特定することで多重割込みか判定する．
	 */
	tst   lr, #EXC_RETURN_PSP    /* 割込み元がMSPなら多重割込み */
	beq   core_int_entry_1       /* 多重割込みならcore_int_entry_1へ */
	mrs   r0, psp                /* 一段目の割込みの場合はPSP上に */
	stmfd r0!,{r2}               /* 割込み発生時の割込み優先度マスクを積む */
	stmfd r0!,{lr}               /* EXC_RETURN を積む             */
	msr   psp, r0                /* CPU例外ハンドラへの引数となる */
	push  {lr}                   /* MSP上にもEXC_RETURN を積む    */ 
#ifdef TOPPERS_SUPPORT_OVRHDR
    push    {r0}
    bl      ovrtimer_stop
    pop     {r0}
#endif /* TOPPERS_SUPPORT_OVRHDR */
	b     core_int_entry_2
ALABEL(core_int_entry_1)         /* 多重割込みの場合 */
	push  {r2}                   /* 割込み発生時の割込み優先度マスクを積む */
	push  {lr}                   /* EXC_RETURN を積む             */ 
	mov   r0, sp                 /* 未定義の割込みが発生した場合の情報とする */

	/*
	 *  共通処理
	 */
ALABEL(core_int_entry_2)
	mrs   r3, ipsr               /* ハンドラアドレスを取得 */
	ldr   r1, =_kernel_exc_tbl
	ldr   r2, [r1, r3, lsl #2]

	/*
	 *  basepriの設定
	 *  NVIC優先度マスクが自動的に設定されるため優先度マスクの点では必要な
	 *  いが，x_get_ipm()がbasepriを参照するため，basepriも更新する．
	 */
	ldr   r1, =_kernel_int_iipm_tbl
	ldr   lr, [r1, r3, lsl #2]
	msr   basepri, lr
    isb

#ifdef LOG_INH_ENTER
	push  {r0,r2,r3}
	mov   r0, r3                 /* 例外番号をパラメータに  */
	bl    log_inh_enter          /* log_exc_enterを呼び出す */
	pop   {r0,r2,r3}
	push  {r3}                   /* 例外番号をスタックへ    */
#endif /* LOG_EXC_ENTER */

	/*
	 *  割込みハンドラの呼び出し
	 */
	blx   r2

#ifdef LOG_INH_LEAVE
	pop   {r0}                   /* 例外番号を引数に        */
	bl    log_exc_leave          /* log_exc_leaveを呼び出す */
#endif /* LOG_INH_LEAVE */

/*
 *  割込み/例外出口
 *
 *  ret_exc/ret_intは，CPU例外/割込みハンドラから戻った直後に実行する
 *  ルーチンである．
 */
ALABEL(ret_exc)
ALABEL(ret_int)
	/*
	 *  割込みロック状態とする．この時点では，CPUロック状態にはならない
	 * （basepriとlock_flagとsaved_iipmは更新しない）．
	 *
	 *  割込みロック状態とするのは，戻り先のコンテキストのチェックと，
	 *  戻り先が非タスクコンテキストであった場合のリターンをアトミック
	 *  に行うためである．bsepriをCPUロックの値にすることでもアトミッ
	 *  クなチェックと復帰は可能であるが，割込みからリターンしても，
	 *  basepri の設定内容は元に戻らないため，使用することができない． 
	 *  一方，FAULTMASKは，割込みからのリターン処理によって，'0'にクリ
	 *  アされる．
	 */
	cpsid f

	/*
	 *  戻り先のコンテキストの判定
	 * 
	 *  割込みハンドラ実行にLRにセットされるEXC_RETURNをチェックして，戻り
	 *  先でMSPが使われていれば，割込み先が非タスクコンテキストと判定する．
	 */
	pop   {r3}                     /* lrをスタックから取得         */
	tst   r3, #EXC_RETURN_PSP      /* 戻り先がPSPなら ret_int_1 へ */
	bne   ret_int_1
	pop   {r1}                     /* 元の割込み優先度マスク(basepri)をr1へ */
	b     ret_int_2                

	/*
	 *  一段目の割込みの出口処理
	 */
ALABEL(ret_int_1)
	/*
	 *  PSP上から，EXC_RETURN(r0)と元の割込み優先度マスク(basepri)(r1)
	 *  を取得
	 */
	mrs   r2, psp
	ldmfd r2!, {r0,r1} 
	msr   psp, r2

    /*
     *  割込みハンドラによりディスパッチ要求があった場合には，
     *  request_dispatchによって，PendSV要求が発行されている．
     *  その場合，このまま割込みハンドラからリターンした後，
     *  PendSVの割込みが入り，そこでディスパッチャが実行される
     *  よって，ここではディスパッチするかどうかのチェック処理は
     *  不要である
     */

#ifdef TOPPERS_SUPPORT_OVRHDR
    push    {r1,r3}
    bl      ovrtimer_start
    pop     {r1,r3}
#endif /* TOPPERS_SUPPORT_OVRHDR */
ALABEL(ret_int_2)
	/*
	 *  ここには割込みロック状態（FAULTMASKがセット）された状態で来る．
	 *  Handlerモードからのリターンにより自動的に割込みロック解除状態になる．
	 *  割込み優先度マスクは割込み前に状態に戻す．
	 */ 
	msr   basepri, r1             /* 割込み優先度マスクを割込み前の状態へ */
	bx    r3                      /* リターン */

/*
 *  ディスパッチャの動作開始
 */
	ATHUMB(start_dispatch)
	AGLOBAL(start_dispatch)
ALABEL(start_dispatch)
	/*
	 *  このルーチンは，カーネル起動時に，すべての割込みを禁止した状態
	 * （割込みロック状態と同等）で呼び出される．また，割込みモード（非
	 *  タスクコンテキストと同等）で呼び出されることを想定している．
	 *
	 *  core_initializeで，lock_flagをtrueに，saved_iipmをIIPM_ENAALLに
	 *  初期化しているため，カーネル管理外の割込みを許可することで，
	 *  CPUロック状態・（モデル上の）割込み優先度マスク全解除状態になる．
	 *  また，task_initializeでdisdspをfalseに初期化しているため，ディ
	 *  スパッチ許可状態になっている．
	 */
	ldr   r0,=istkpt              /* MSPを初期化   */
	ldr   r1,[r0]                 /* start_dispatch呼び出し時に呼び出し用に */
	msr   msp, r1                 /* 使用しているため初期化する             */
	/* 
     * スタックをIDが1のタスクのスタック領域に切り換える
     */
	mov   r0, #CONTROL_PSP        /* PSPを有効に  */
	msr   control, r0
	isb                           /* control の操作後に必要 */
    ldr   r1, =tinib_table
    ldr   sp, [r1, #TINIB_stk_bottom]
    /* 
     * ID1のタスクがすでにactivate_contextされている場合，
     * スタックの先頭にspを初期化してしまうと，svcにより
     * activate_contextで準備した内容が消されてしまうため，
     * その分を飛ばしたアドレスを，カーネル動作開始時の
     * スタック先頭アドレスとする
     */
    add   sp, sp, #(4*16)         
	ldr   r1, =IIPM_LOCK          /* カーネル管理内の割込みを禁止 */
	msr   basepri, r1 
    isb
	cpsie f                       /* カーネル管理外の割込みを許可 */
    /*
     *  _exit_and_dispatchでCPUロック解除&FAULTMASK=1をしているにも
     *  かかわらず，ここでCPUロック&FAULTMASK=0としているのは，
     *  ディスパッチャをsvc発行時の状態で呼んでおり，かつ，
     *  FAULTMASK状態でsvcを発行するとdouble faultでリセットがかかる
     *  ためである
     */

	ATHUMB(exit_and_dispatch)
	AGLOBAL(exit_and_dispatch)
ALABEL(exit_and_dispatch)
    /*
     *  r3=pspはsvcの発行後でもよいが，スタック使用量が多くなるため，
     *  svc前のpspをr3に保存しておく
     */
    svc    #NO_DISPATCHER

/*
 *  現在のコンテキストを捨ててディスパッチ
 */
	ATHUMB(_exit_and_dispatch)
	AGLOBAL(_exit_and_dispatch)
ALABEL(_exit_and_dispatch)
	cpsid f                       /* FAULTMASK = 1 */
	mov   r0, #0
	ldr   r1, =lock_flag          /* CPUロック解除状態へ */
	str   r0, [r1]
	ldr   r1, =IIPM_ENAALL        /* 割込み優先度マスクを全解除状態に設定 */
	msr   basepri, r1             
    mrs     r3, psp 
	/* ディスパッチャ本体（dispatcher）へ */ 

/*
 *  ディスパッチャ本体
 */
	ATHUMB(dispatcher)
	AGLOBAL(dispatcher)
ALABEL(dispatcher)
	/*
	 *  このルーチンは，タスクコンテキスト・CPUロック状態・ディスパッチ
	 *  許可状態・（モデル上の）割込み優先度マスク全解除状態で呼び出さ
	 *  れる．
	 *
	 *  すなわち，Handlerモード・lock_flagがtrue・disdspがfalse・dspflg
	 *  がtrue・saved_iipmがIIPM_ENAALLとなっている．実行再開番地へもこ
	 *  の状態のまま分岐する．
     *  また，MSPは，istkpt（初期値）となっている
     *  
     *  from dispatch: 
     *    svc命令によりHandlerモードとなり，pspに戻り先番地dispatch_rを
     *    積んだ状態（TCB.spにpspを，TCB.pcにEXC_RETURN_PSPを保存済み）
     *  from start_dispatch/exit_and_dispatch: 
     *    svc命令によりHandlerモードとなった状態
     *    （以前に実行状態であったタスクorID0のタスクのスタックに戻り先
     *      などが積まれるが，次回にタスクを起動する際に破棄される）
     *  from ret_int: 
     *    すでにHandlerモードであるため，単なるジャンプ命令でここにくる
     *    （TCB.spにpspを，TCB.pcにret_int_rを保存済み）

     *  to dispatch_r: 
     *    TCB.pcからEXC_RETURN_PSPをロードし，bx命令によりリターンする
     *    ことで，Threadモードとなり，TCB.spのpspに積まれたdispatch_r
     *    に戻る（TCB.pc/spはdispatchでセット済み）
     *  to start_r: 
     *    TCB.pcからstart_rをロードし，bx命令によりジャンプしたあと，
     *    タスク起動時の引数レジスタ値，pcをTCB.spのpspに積み，bx 
     *    EXC_RETURN_PSP 命令によってThreadモードとなり，タスク起動番地
     *    へジャンプする
     *    （TCB.pc/spはactivate_contextでセット済み）
     *  to ret_int_r: 
     *    TCB.pcからret_int_rをロードし，bx命令によりジャンプしたあと，
     *    割込みハンドラの出口処理により，Threadモードに戻り，割込み元に
     *    戻る（TCB.pc/spはret_intでセット済み）
	 */
#ifdef TOPPERS_SUPPORT_OVRHDR
    bl    ovrtimer_stop
#endif /* TOPPERS_SUPPORT_OVRHDR */
#ifdef LOG_DSP_ENTER
	ldr   r1, =p_runtsk     /* p_runtskをパラメータに */
	ldr   r0, [r1]        
	bl    log_dsp_enter
#endif /* LOG_DSP_ENTER */
ALABEL(dispatcher_0)
	ldr   r0, =p_schedtsk   /* p_schedtskをp_runtskに */
	ldr   r1, [r0]
	ldr   r2, =p_runtsk   
	str   r1, [r2]        
	cbz   r1, idle_loop     /* p_runtskがNULLならidle_loopへ */           
	ldr   lr, [r1,#TCB_pc]  /* 実行再開番地を復帰   */    
	ldr   r0, [r1,#TCB_sp]  /* タスクのスタックを復帰 */
#ifdef __TARGET_FPU_VFPUV4_D16
    /*
     *  戻り先のコンテキストで浮動小数点演算をしたか？
     *  EXC_RETURN[4]をチェック
     */
    tst     lr, #0x10
    /*  EXC_RETURN[4] == 1ならばスキップ */
    bne     dispatcher_2
    /*fpscrは例外フレームから復帰する*/
	vldmia  r0!, {s16-s31}         /* レジスタの復帰 */
ALABEL(dispatcher_2)
#endif /* __TARGET_FPU_VFPUV4_D16 */
    ldmfd   r0!, {r4-r11}       /* レジスタの復帰 */
    msr     psp, r0           /* psp = p_runtsk->sp */

#ifdef LOG_DSP_LEAVE
	mov   r0, r1            /* p_runtskをパラメータに */
	mov   r4, r1            /* r1はスクラッチレジスタなので保存 */
	bl    log_dsp_leave
	mov   r1, r4
#endif /* LOG_DSP_LEAVE */
#ifdef TOPPERS_SUPPORT_OVRHDR
    push    {lr}
    bl      ovrtimer_start
    pop     {lr}
#endif /* TOPPERS_SUPPORT_OVRHDR */
    /*
     *  ARM<->Thumbモード変換を抑制するためにpcに直接戻り番地を
     *  入れる
     *  割込み元に戻る場合においても，EXC_RETURNを直接pcに代入して
     *  （bx命令を使わずに戻っても）問題ない
     */
	bx     lr  /* 実行再開番地を復帰   */    

ALABEL(idle_loop)
	/*
	 * 割込みを許可したらCPUロック解除状態になるよう準備する
	 * CPUロック状態の解除と，非タスクコンテキスト実行状態への
	 * 準備をする
	 */
    /*
     *  pspにダミーフレームを積んでbx lrにより，threadモードかつidleへ
     *  この時点では，r3=pspである
     *  r3には，dispatchでcallee-saved-registerが保存されたあとの
     *  スタックポインタが入っており，その続きからスタックを使用する
     */
	ldr   r0, =_idle_loop         /* PC   */
	ldr   r1, =EPSR_T             /* xPSR(Tビットが'1'である必要がある) */
    /*
     *  アイドルループ中のFPは任意なので設定しない
     */
    ldr   lr, =0xfffffffd
	stmfd r3!, {r0-r1}            /* ダミーフレームをスタック上に積む   */
	sub   r3, #(6*4) /* r0-r3,r12,lrの内容は設定する必要がない */
	msr   psp, r3                  
    bx    lr

ALABEL(_idle_loop)
    /* ここではすでに割込み許可状態 */
#ifdef TOPPERS_CUSTOM_IDLE
	toppers_asm_custom_idle
#else
ALABEL(_idle_loop_1)
    /*
     *  basepriを全割込み許可に設定してからwfiが発行されるまでに
     *  割込みが入った場合，そこでディスパッチが必要な状態になれば
     *  割込みの出口処理で遅延ディスパッチし，ここには戻ってこない．
     *  遅延ディスパッチしなかった場合はここに戻ってきて次の割込みを
     *  wfiで待つため，想定どおりの動作となる
     */
    wfi
    b     _idle_loop_1
    nop
#endif /* TOPPERS_CUSTOM_IDLE */

/*
 *  カーネルの終了処理の呼出し
 *
 *  スタックを非タスクコンテキスト用に切り替え．
 *  
 */
	ATHUMB(call_exit_kernel)
	AGLOBAL(call_exit_kernel)
ALABEL(call_exit_kernel)
	mov   r0, #CONTROL_MSP
	msr   control, r0       /* MSPを有効に  */
	isb                     /* control の操作後に必要 */
	ldr   r0, =exit_kernel  /* カーネルの終了処理を呼ぶ */
	bx    r0

/*
 *  微少時間待ち
 */
	AALIGN(2)
	ATEXT
	ATHUMB(sil_dly_nse)
	AGLOBAL(sil_dly_nse)
ALABEL(sil_dly_nse)
	sub   r0, r0, #SIL_DLY_TIM1
	cmp   r0, #0
	bgt   sil_dly_nse1
	bx    lr
ALABEL(sil_dly_nse1)
	sub   r0, r0, #SIL_DLY_TIM2
	cmp   r0, #0
	bgt   sil_dly_nse1
	bx    lr

