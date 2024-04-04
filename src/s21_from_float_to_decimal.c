#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "s21_dec_auxiliary.h"
#include "s21_decimal.h"

int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  int err = S21_CONVERSION_ERROR;
  if ((dst != NULL) && (src < pow(2, 96)) && (src > -pow(2, 96)) &&
      (!isinf(src)) && (!isnan(src))) {
    s21_decimal_initialize(dst);
    if (fabs(src) >= 1e-28 || src == 0.0) {
      char buf[60];
      sprintf(buf, "%e", src);  // convert src into scientific notation

      int length = strlen(buf);
      int power = -100;
      char buf_new[3];
      strcpy(buf_new, &buf[length - 2]);

      power = atoi(buf_new);         // parse power from scientific notation
      if (buf[length - 3] == '-') {  // parse the sign of power
        power = -power;
      }
      int i = 0;
      s21_chad_decimal float_to_dec_string;
      s21_initialize(&float_to_dec_string);
      if (length == 13) {
        *float_to_dec_string.sign = '-';  // add sign of value into the string
        i = 1;
      }
      for (; i < length - 4; i++) {  // copy 7 digits into the string
        if ((i == length - 11) || (power < -30)) {  // skip '.'
          continue;
        } else {
          *(float_to_dec_string.integer + power) = buf[i] - '0';
          power--;
        }
      }

      err = s21_chad_to_decimal(float_to_dec_string, dst);
    }
  }
  return err;
}
