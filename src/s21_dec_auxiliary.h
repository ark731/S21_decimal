#ifndef S21_DECIMAL_AUX
#define S21_DECIMAL_AUX

/* Some tests comparing output of this lib with other decimal libs were found to
 * fail due to extra precision implemented here. This option controls precision.
 * Set to 1 for compatibility with other decimal libs and School21 autotests.
 * Set to 0 for full precision. */
#define S21_DECREASE_PRECISION 1

#include <stdint.h>
#include <string.h>

#include "s21_decimal.h"

#define s21_max(a, b) ((a) > (b) ? (a) : (b))
#define s21_min(a, b) ((a) < (b) ? (a) : (b))

/* Each decimal digit is to be represented with signed byte type. */
typedef int8_t digit;

/* Array of signed bytes large enough to hold full range of decimal type. For
 * convenience and simplicity, pointers to sign and smallest digit of integer
 * part of a number are added. */
typedef struct {
  digit digit[60];
  digit *sign;
  digit *integer;
} s21_chad_decimal;

/* Function return codes. */
#define S21_OK 0
#define S21_OVERFLOW 1
#define S21_UNDERFLOW 2
#define S21_DIVISION_BY_ZERO 3

#define S21_CONVERSION_ERROR 1
#define S21_CALCULATION_ERROR 1

#define S21_FALSE 0
#define S21_TRUE !0

/////////////////////////////////////////////////////////////////
/* ----------------- TEMPORARY TYPE HANDLING ----------------- */
/////////////////////////////////////////////////////////////////
/* Initialize decimal structure. */
void s21_decimal_initialize(s21_decimal *dec);

/* Initialize chad type number structure. */
void s21_initialize(s21_chad_decimal *);

/* Copy chad type number. */
void s21_chad_copy(s21_chad_decimal *dst, s21_chad_decimal *src);

/* Write scale and sign into decimal type number. */
void s21_set_scale(s21_decimal *dec, s21_chad_decimal *chad, int scale);

/* Conversion from chad type number to decimal. Returns:
 * 0 - OK
 * 1 - Overflow
 * 2 - Underflow  */
int s21_chad_to_decimal(s21_chad_decimal, s21_decimal *);

/* Runs arithmetic for conversion from chad type number to decimal. */
void s21_chad_to_decimal_conv(s21_chad_decimal *chad, s21_decimal *dec);

/* Conversion from decimal to chad type number. Returns:
 * 0 - OK
 * 1 - Conversion error  */
void s21_decimal_to_chad(s21_decimal, s21_chad_decimal *);

/* Convert absolute value of chad type number to unsigned int. */
void s21_chad_to_uint32(unsigned int *dst, s21_chad_decimal *src);

/* Convert chad type number to int. */
void s21_chad_to_int(int *dst, s21_chad_decimal *src);

/* Convert chad type number to float. */
void s21_chad_to_float(float *dst, s21_chad_decimal *src);

/* Handle digit overflow. Returns:
 * 0 - OK
 * 1 - Overflow
 * 2 - Underflow  */
int s21_correct_digit_overflow(s21_chad_decimal *);

/* Multiply mantissa by 10^shift. */
void s21_buffer_shift(digit *, int shift);

/* Finds the index of the biggest and smallest digit of the number. */
void s21_number_position(s21_chad_decimal *, int *begin, int *end);

/* Some tests comparing output of this lib with other decimal libs were found to
 * fail due to extra precision implemented here. This function decreases
 * precision for compatibility. */
void s21_decrease_precision(s21_chad_decimal *chad);

/////////////////////////////////////////////////////////////////
/* ---------------- TEMPORARY TYPE COMPARISON ---------------- */
/////////////////////////////////////////////////////////////////
/* Compare chad type numbers.
 * -1 - num1 < num2
 * 0  - num1 = num2
 * 1  - num1 > num2  */
int s21_chad_compare(s21_chad_decimal *num1, s21_chad_decimal *num2);

/* Compare absolute values of chad type numbers.
 * -1 - num1 < num2
 * 0  - num1 = num2
 * 1  - num1 > num2  */
int s21_chad_module_compare(s21_chad_decimal *num1, s21_chad_decimal *num2);

/////////////////////////////////////////////////////////////////
/* ---------------- TEMPORARY TYPE ARITHMETIC ---------------- */
/////////////////////////////////////////////////////////////////
/* Rounding at specified digit. Returns:
 * 0 - OK
 * 1 - Overflow/Underflow error
 * Mode:
 * 't' - Truncate
 * 'r' - Round
 * 'f' - Round to lower (floor)
 * 'b' or 'e' - Bankers rounding  */
int s21_chad_round(s21_chad_decimal *, int integer_offset, int mode);

/* Check all digits after current minus offset to see if there is a non-zero
 * digit. For banker's and floor rounding modes. */
void s21_chad_round_check_digits(s21_chad_decimal *chad, int i, int offset);

/* Round number to 7 significant digits to prepare conversion to float. */
void s21_chad_round_to_float(s21_chad_decimal *tmp);

/* Division.
 * 0 - OK
 * 1 - Overflow or INFINITY
 * 2 - Underflow or -INFINITY
 * 3 - division by zero */
int s21_chad_div(s21_chad_decimal *dividend, s21_chad_decimal *divisor,
                 s21_chad_decimal *result);

/* Arithmetics for chad type number division. */
int s21_chad_div_calc(s21_chad_decimal *v1, s21_chad_decimal *v2,
                      s21_chad_decimal *res, int v1_begin, int v1_end,
                      int v2_begin, int v2_end, int result_offset);

/* Check if necessary and shift divisor right for next round of division. */
void s21_chad_div_set_divisor(s21_chad_decimal *v1, s21_chad_decimal *v2,
                              int *result_offset);

/* Multiplication.
 * 0 - OK
 * 1 - Overflow or INFINITY
 * 2 - Underflow or -INFINITY
 * 3 - division by zero */
int s21_chad_mul(s21_chad_decimal *num1, s21_chad_decimal *num2,
                 s21_chad_decimal *result);

/* Multiplication of absolute values, result always positive.
 * 0 - OK
 * 1 - Overflow
 * 2 - Underflow */
int s21_chad_mul_abs(s21_chad_decimal *num1, s21_chad_decimal *num2,
                     s21_chad_decimal *result);

/* Substraction.
 * 0 - OK
 * 1 - Overflow
 * 2 - Underflow */
int s21_chad_sub(s21_chad_decimal *minuend, s21_chad_decimal *subtrahend,
                 s21_chad_decimal *result);

/* Substraction of absolute values, result always positive. num1 MUST have
 * bigger absolute value than num2.
 * 0 - OK
 * 1 - Overflow
 * 2 - Underflow */
int s21_chad_sub_abs(s21_chad_decimal *minuend, s21_chad_decimal *subtrahend,
                     s21_chad_decimal *result);

/* Addition.
 * 0 - OK
 * 1 - Overflow
 * 2 - Underflow */
int s21_chad_add(s21_chad_decimal *num1, s21_chad_decimal *num2,
                 s21_chad_decimal *result);

/* Addition of absolute values, result always positive.
 * 0 - OK
 * 1 - Overflow
 * 2 - Underflow */
int s21_chad_add_abs(s21_chad_decimal *num1, s21_chad_decimal *num2,
                     s21_chad_decimal *result);

/////////////////////////////////////////////////////////////////
/* ------------------ CONSTANTS AND LIMITS ------------------- */
/////////////////////////////////////////////////////////////////
/* Check overflow / underflow. Returns:
 * 0 - OK
 * 1 - Overflow
 * 2 - Underflow  */
int s21_check_overflow(const digit *integer);

/* Check if decimal number is within int32 type limits. Returns:
 * 0 - OK
 * 1 - Overflow / Underflow  */
int s21_check_int_overflow(s21_decimal *dec);

/* Write chad value for 2^32. */
void s21_init_pow2_32(s21_chad_decimal *chad_decimal);

/* Write chad value for 2^64. */
void s21_init_pow2_64(s21_chad_decimal *chad_decimal);

#endif