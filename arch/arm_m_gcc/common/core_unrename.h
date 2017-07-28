/* This file is generated from core_rename.def by genrename. */

/* This file is included only when core_rename.h has been included. */
#ifdef TOPPERS_CORE_RENAME_H
#undef TOPPERS_CORE_RENAME_H

/*
 *  core_kernel_impl.c
 */
#undef exc_tbl
#undef vector_table
#undef lock_flag
#undef saved_iipm
#undef default_exc_handler
#undef default_int_handler
#undef config_int
#undef core_initialize
#undef core_terminate
#undef bitpat_cfgint
#undef set_exc_int_priority
#undef enable_exc
#undef disable_exc

/*
 *  core_support.S
 */
#undef core_int_entry
#undef core_exc_entry
#undef ret_int
#undef ret_int_r
#undef ret_exc
#undef svc_handler
#undef svc_vector_tbl
#undef start_r
#undef dispatcher
#undef dispatch
#undef dispatch_r
#undef start_dispatch
#undef exit_and_dispatch
#undef call_exit_kernel


#ifdef TOPPERS_LABEL_ASM

/*
 *  core_kernel_impl.c
 */
#undef _exc_tbl
#undef _vector_table
#undef _lock_flag
#undef _saved_iipm
#undef _default_exc_handler
#undef _default_int_handler
#undef _config_int
#undef _core_initialize
#undef _core_terminate
#undef _bitpat_cfgint
#undef _set_exc_int_priority
#undef _enable_exc
#undef _disable_exc

/*
 *  core_support.S
 */
#undef _core_int_entry
#undef _core_exc_entry
#undef _ret_int
#undef _ret_int_r
#undef _ret_exc
#undef _svc_handler
#undef _svc_vector_tbl
#undef _start_r
#undef _dispatcher
#undef _dispatch
#undef _dispatch_r
#undef _start_dispatch
#undef _exit_and_dispatch
#undef _call_exit_kernel


#endif /* TOPPERS_LABEL_ASM */


#endif /* TOPPERS_CORE_RENAME_H */
