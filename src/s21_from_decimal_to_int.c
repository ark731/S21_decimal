#include "s21_dec_auxiliary.h"
#include "s21_decimal.h"

int s21_from_decimal_to_int(s21_decimal decimal_num, int *integer_num) {
  int result = S21_OK;
  if (integer_num == NULL) {  // NULL check
    result = S21_CONVERSION_ERROR;
  } else {
    *integer_num = 0;
    if (s21_check_int_overflow(&decimal_num)) {  // check int32 overflow
      result = 1;
    } else {
      s21_chad_decimal tmp;
      s21_initialize(&tmp);

      s21_decimal_to_chad(decimal_num, &tmp);  // convert to temporary

      s21_chad_to_int(integer_num, &tmp);  // convert from temporary
    }
  }
  return result;
}
