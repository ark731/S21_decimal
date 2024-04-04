#include "s21_dec_auxiliary.h"
#include "s21_decimal.h"

int s21_negate(s21_decimal value, s21_decimal *result) {
  int res = S21_OK;
  if (result == NULL) {
    res = S21_CALCULATION_ERROR;
  } else {
    for (int i = 0; i <= 3; ++i) {  // copy value to result
      result->bits[i] = value.bits[i];
    }
    result->bits[3] ^= 0x80000000U;  // invert sign bit by its mask
  }
  return res;
}