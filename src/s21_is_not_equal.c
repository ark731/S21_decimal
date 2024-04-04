#include "s21_dec_auxiliary.h"
#include "s21_decimal.h"

int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2) {
  int is_not_equal = S21_FALSE;  // set value = false
  s21_chad_decimal tmp1, tmp2;
  s21_initialize(&tmp1);                // set temporary buffer
  s21_initialize(&tmp2);                // set temporary buffer
  s21_decimal_to_chad(value_1, &tmp1);  // convert value1 to buffer
  s21_decimal_to_chad(value_2, &tmp2);  // convert value2 to buffer

  int result_of_compare = s21_chad_compare(&tmp1, &tmp2);
  if (result_of_compare != 0) {  // less or greater
    is_not_equal = S21_TRUE;     // set value = true
  }

  return is_not_equal;
}