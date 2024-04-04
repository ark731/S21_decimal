#include "s21_dec_auxiliary.h"
#include "s21_decimal.h"

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  int res = S21_OK;
  if (result != NULL) {
    s21_decimal_initialize(result);  // initialize memory at incoming pointer

    s21_chad_decimal array1, array2, add_result;
    s21_initialize(&array1);
    s21_initialize(&array2);
    s21_initialize(&add_result);

    s21_decimal_to_chad(value_1, &array1);  // convert addends into chad
    s21_decimal_to_chad(value_2, &array2);

    res = s21_chad_add(&array1, &array2, &add_result);  // run addition

    if (res == S21_OK) {
      res = s21_chad_to_decimal(add_result, result);  // convert result back
    }
  }
  return res;
}