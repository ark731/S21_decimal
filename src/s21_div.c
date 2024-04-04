#include "s21_dec_auxiliary.h"
#include "s21_decimal.h"

int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  if (!result) return 1;           // NULL check
  s21_decimal_initialize(result);  // initialize memory at pointer
  int err = S21_OK;
  if (!value_2.bits[0] && !value_2.bits[1] && !value_2.bits[2]) {
    err = S21_DIVISION_BY_ZERO;  // check div by 0
  } else {
    s21_chad_decimal v1, v2, res;  // declare

    s21_initialize(&v1);   //
    s21_initialize(&v2);   // construct
    s21_initialize(&res);  //

    s21_decimal_to_chad(value_1, &v1);  // convert to chad
    s21_decimal_to_chad(value_2, &v2);  //

    !err ? err = s21_chad_div(&v1, &v2, &res) : 0;  // calculate

    !err ? err = s21_chad_to_decimal(res, result) : 0;  // convert from chad
  }

  return err;
}