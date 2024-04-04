
#include "s21_dec_auxiliary.h"
#include "s21_decimal.h"

int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  if (!result) return S21_CALCULATION_ERROR;
  s21_decimal_initialize(result);
  int res = S21_CALCULATION_ERROR;
  s21_chad_decimal tmp1, tmp2;
  s21_initialize(&tmp1);                // set temporary buffer
  s21_initialize(&tmp2);                // set temporary buffer
  s21_decimal_to_chad(value_1, &tmp1);  // convert value1 to buffer
  s21_decimal_to_chad(value_2, &tmp2);  // convert value2 to buffer

  s21_chad_decimal temp_result;
  s21_initialize(&temp_result);
  res = s21_chad_sub(&tmp1, &tmp2, &temp_result);
  if (res == S21_OK) {
    res = s21_chad_to_decimal(temp_result, result);
  }

  return res;
}