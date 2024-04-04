#include "s21_dec_auxiliary.h"
#include "s21_decimal.h"

int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  if (!result) return S21_CALCULATION_ERROR;  // NULL check
  s21_decimal_initialize(result);
  int res = S21_OK;

  s21_chad_decimal array1, array2, add_result;  // create temporary structures
  s21_initialize(&array1);
  s21_initialize(&array2);
  s21_initialize(&add_result);

  s21_decimal_to_chad(value_1, &array1);  // convert to temporary structures
  s21_decimal_to_chad(value_2, &array2);

  res = s21_chad_mul(&array1, &array2, &add_result);  // multiply

  if (res == S21_OK) {
    res = s21_chad_to_decimal(add_result, result);  // convert result to decimal
  }

  return res;
}