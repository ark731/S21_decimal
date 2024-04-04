#include <stdlib.h>

#include "s21_dec_auxiliary.h"
#include "s21_decimal.h"

int s21_from_int_to_decimal(int s21_src, s21_decimal *dst) {
  int result = S21_OK;
  if (dst == NULL) {  // NULL check
    result = S21_CONVERSION_ERROR;
  } else {
    s21_decimal_initialize(dst);  // initialize memory at dst

    if (s21_src & 0x80000000U) {  // set sign bit
      dst->bits[3] |= 0x80000000U;
    }
    dst->bits[0] = (unsigned)labs((long)s21_src);  // write int to bits[0]
  }
  return result;
}
