#ifndef STDLIB_C_H
#define STDLIB_C_H

#ifdef __cplusplus
#   define EXTERN extern "C"
#else
#   define EXTERN extern
#endif

#include "kumtypes.h"

EXTERN void __kumir_init_stdlib();

EXTERN void __kumir_init_scalar(__kumir_scalar * scalar);

EXTERN __kumir_scalar __kumir_create_int(__kumir_int value);
EXTERN __kumir_scalar __kumir_create_real(__kumir_real value);
EXTERN __kumir_scalar __kumir_create_bool(__kumir_bool value);
EXTERN __kumir_scalar __kumir_create_char(const char * utf8);
EXTERN __kumir_scalar __kumir_create_string(const char * utf8);
EXTERN __kumir_variant __kumir_copy_variant(const __kumir_variant rvalue, __kumir_scalar_type type);
EXTERN __kumir_scalar __kumir_copy_scalar(const __kumir_scalar rvalue);
EXTERN void __kumir_check_value_defined(const __kumir_scalar value);
EXTERN void __kumir_free_scalar(__kumir_scalar scalar);

EXTERN void __kumir_output_stdout_ii(const __kumir_scalar value, const int format1, const int format2);
EXTERN void __kumir_output_stdout_is(const __kumir_scalar value, const int format1, const __kumir_scalar format2);
EXTERN void __kumir_output_stdout_si(const __kumir_scalar value, const __kumir_scalar format1, const int format2);
EXTERN void __kumir_output_stdout_ss(const __kumir_scalar value, const __kumir_scalar format1, const __kumir_scalar format2);

EXTERN void __kumir_output_file_ii(const __kumir_scalar handle, const __kumir_scalar value, const int format1, const int format2);
EXTERN void __kumir_output_file_is(const __kumir_scalar handle, const __kumir_scalar value, const int format1, const __kumir_scalar format2);
EXTERN void __kumir_output_file_si(const __kumir_scalar handle, const __kumir_scalar value, const __kumir_scalar format1, const int format2);
EXTERN void __kumir_output_file_ss(const __kumir_scalar handle, const __kumir_scalar value, const __kumir_scalar format1, const __kumir_scalar format2);

EXTERN void __kumir_assert(const __kumir_scalar assumption);

EXTERN __kumir_scalar __kumir_operator_eq(const __kumir_scalar left, const __kumir_scalar right);
EXTERN __kumir_scalar __kumir_operator_ls(const __kumir_scalar left, const __kumir_scalar right);
EXTERN __kumir_scalar __kumir_operator_gt(const __kumir_scalar left, const __kumir_scalar right);
EXTERN __kumir_scalar __kumir_operator_lq(const __kumir_scalar left, const __kumir_scalar right);
EXTERN __kumir_scalar __kumir_operator_gq(const __kumir_scalar left, const __kumir_scalar right);
EXTERN __kumir_scalar __kumir_operator_neq(const __kumir_scalar left, const __kumir_scalar right);

extern void __kumir_init_array_1(__kumir_array * array,
                                 __kumir_int left_1,
                                 __kumir_int right_1
                                 );
EXTERN void __kumir_init_array_2(__kumir_array * array,
                                 __kumir_int left_1,
                                 __kumir_int right_1,
                                 __kumir_int left_2,
                                 __kumir_int right_2
                                 );
EXTERN void __kumir_init_array_3(__kumir_array * array,
                                 __kumir_int left_1,
                                 __kumir_int right_1,
                                 __kumir_int left_2,
                                 __kumir_int right_2,
                                 __kumir_int left_3,
                                 __kumir_int right_3
                                 );

EXTERN __kumir_scalar __kumir__stdlib__otkryit_na_chtenie(const __kumir_scalar name);
EXTERN __kumir_scalar __kumir__stdlib__otkryit_na_zapis(const __kumir_scalar name);
EXTERN void __kumir__stdlib__zakryit(const __kumir_scalar handle);

EXTERN void test_123();
EXTERN void test_456(__kumir_scalar arg);
EXTERN __kumir_scalar test_890();

#endif // STDLIB_C_H