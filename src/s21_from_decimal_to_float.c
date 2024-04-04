
#include "s21_dec_auxiliary.h"
#include "s21_decimal.h"

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  int err = S21_OK;
  if (dst == NULL) {  // NULL check
    err = S21_CONVERSION_ERROR;
  } else {
    *dst = 0;
    s21_chad_decimal tmp;
    s21_initialize(&tmp);

    s21_decimal_to_chad(src, &tmp);  // convert to temporary type

    s21_chad_round_to_float(&tmp);  // round to float precision

    s21_chad_to_float(dst, &tmp);  // convert from temporary type
  }
  return err;
}
