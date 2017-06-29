/* This file is generated from core_rename.def by genrename. */

#ifndef TOPPERS_CORE_RENAME_H
#define TOPPERS_CORE_RENAME_H

/*
 *  core_kernel_impl.c
 */
#define exc_tbl						_kernel_exc_tbl
#define vector_table				_kernel_vector_table
#define lock_flag					_kernel_lock_flag
#define saved_iipm					_kernel_saved_iipm
#define default_exc_handler			_kernel_default_exc_handler
#define default_int_handler			_kernel_default_int_handler
#define config_int					_kernel_config_int
#define core_initialize				_kernel_core_initialize
#define core_terminate				_kernel_core_terminate
#define bitpat_cfgint				_kernel_bitpat_cfgint
#define set_exc_int_priority		_kernel_set_exc_int_priority
#define enable_exc					_kernel_enable_exc
#define disable_exc					_kernel_disable_exc

/*
 *  core_support.S
 */
#define core_int_entry				_kernel_core_int_entry
#define core_exc_entry				_kernel_core_exc_entry
#define ret_int						_kernel_ret_int
#define ret_int_r					_kernel_ret_int_r
#define ret_exc						_kernel_ret_exc
#define svc_handler					_kernel_svc_handler
#define svc_vector_tbl				_kernel_svc_vector_tbl
#define start_r						_kernel_start_r
#define dispatcher					_kernel_dispatcher
#define dispatch					_kernel_dispatch
#define dispatch_r					_kernel_dispatch_r
#define start_dispatch				_kernel_start_dispatch
#define exit_and_dispatch			_kernel_exit_and_dispatch
#define call_exit_kernel			_kernel_call_exit_kernel



#endif /* TOPPERS_CORE_RENAME_H */
