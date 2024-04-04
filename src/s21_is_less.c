#include "s21_dec_auxiliary.h"
#include "s21_decimal.h"

int s21_is_less(s21_decimal value_1, s21_decimal value_2) {
  int is_less = S21_FALSE;  // set value = false
  s21_chad_decimal tmp1, tmp2;
  s21_initialize(&tmp1);                // set temporary buffer
  s21_initialize(&tmp2);                // set temporary buffer
  s21_decimal_to_chad(value_1, &tmp1);  // convert value1 to buffer
  s21_decimal_to_chad(value_2, &tmp2);

  int result_of_compare = s21_chad_compare(&tmp1, &tmp2);
  if (result_of_compare == -1) {
    is_less = S21_TRUE;  // set value = true
  }

  return is_less;
}