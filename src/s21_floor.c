#include "s21_dec_auxiliary.h"
#include "s21_decimal.h"

int s21_floor(s21_decimal value, s21_decimal *result) {
  if (!result) return S21_CALCULATION_ERROR;  // check for NULL

  int err;
  s21_chad_decimal tmp;
  s21_initialize(&tmp);              // set temporary buffer
  s21_decimal_to_chad(value, &tmp);  // convert to buffer

  err = s21_chad_round(&tmp, 0, 'f');                 // round to lower
  !err ? err = s21_chad_to_decimal(tmp, result) : 0;  // convert back

  return err;
}