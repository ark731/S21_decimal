#ifndef S21_DECIMAL
#define S21_DECIMAL

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef struct {
  unsigned int bits[4];
} s21_decimal;

/////////////////////////////////////////////////////////////////
/* ------------------ ARITHMETIC FUNCTIONS ------------------- */
/* ------------------ FUNCTION RETURN CODES ------------------ */
/* 0 - OK                                                      */
/* 1 - type overflow or INFINITY                               */
/* 2 - type underflow or -INFINITY                             */
/* 3 - division by zero                                        */
/////////////////////////////////////////////////////////////////
/* Addition. */
int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);

/* Substraction. */
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);

/* Multiplication. */
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);

/* Division. */
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);

/////////////////////////////////////////////////////////////////
/* ------------------ COMPARISON FUNCTIONS ------------------- */
/* ------------------ FUNCTION RETURN CODES ------------------ */
/* 0 - FALSE                                                   */
/* 1 - TRUE                                                    */
/////////////////////////////////////////////////////////////////
/* Greater. */
int s21_is_greater(s21_decimal value_1, s21_decimal value_2);

/* Greater or equal. */
int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2);

/* Less. */
int s21_is_less(s21_decimal value_1, s21_decimal value_2);

/* Less or equal. */
int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2);

/* Equal. */
int s21_is_equal(s21_decimal value_1, s21_decimal value_2);

/* Not equal. */
int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2);

/////////////////////////////////////////////////////////////////
/* ------------------ CONVERSION FUNCTIONS ------------------- */
/* ------------------ FUNCTION RETURN CODES ------------------ */
/* 0 - OK                                                      */
/* 1 - Conversion error                                        */
/////////////////////////////////////////////////////////////////
/* int to decimal */
int s21_from_int_to_decimal(int s21_src, s21_decimal *dst);

/* decimal to int */
int s21_from_decimal_to_int(s21_decimal src, int *dst);

/* float to decimal */
int s21_from_float_to_decimal(float src, s21_decimal *dst);

/* decimal to float */
int s21_from_decimal_to_float(s21_decimal src, float *dst);

/////////////////////////////////////////////////////////////////
/* ------------------- AUXILIARY FUNCTIONS ------------------- */
/* ------------------ FUNCTION RETURN CODES ------------------ */
/* 0 - OK                                                      */
/* 1 - Calculation error                                       */
/////////////////////////////////////////////////////////////////
/* Invert sign. */
int s21_negate(s21_decimal value, s21_decimal *result);

/* Rounding down. */
int s21_floor(s21_decimal value, s21_decimal *result);

/* Round to nearest integer. */
int s21_round(s21_decimal value, s21_decimal *result);

/* Discard fractional part. */
int s21_truncate(s21_decimal value, s21_decimal *result);

#endif