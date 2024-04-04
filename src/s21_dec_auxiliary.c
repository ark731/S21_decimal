#include "s21_dec_auxiliary.h"

#include <limits.h>
#include <math.h>
#include <string.h>

#include "s21_decimal.h"

/////////////////////////////////////////////////////////////////
/* ----------------- TEMPORARY TYPE HANDLING ----------------- */
/////////////////////////////////////////////////////////////////
/* Initialize decimal structure. */
void s21_decimal_initialize(s21_decimal *dec) {
  for (int i = 0; i < 4; ++i) (*dec).bits[i] = 0;
}

/* Initialize chad type number structure. */
void s21_initialize(s21_chad_decimal *chad_decimal) {
  memset((*chad_decimal).digit, 0, 60);
  (*chad_decimal).sign = (*chad_decimal).digit + 59;
  (*chad_decimal).integer = (*chad_decimal).digit + 30;
}

/* Copy chad type number. */
void s21_chad_copy(s21_chad_decimal *dst, s21_chad_decimal *src) {
  memcpy((*dst).digit, (*src).digit, 60);
  (*dst).sign = (*dst).digit + 59;
  (*dst).integer = (*dst).digit + 30;
}

/* Write scale and sign into decimal type number. */
void s21_set_scale(s21_decimal *dec, s21_chad_decimal *chad, int scale) {
  (*dec).bits[3] = -scale << 16;                      // set scale
  *(*chad).sign ? (*dec).bits[3] |= 0x80000000U : 0;  // set sign
  *(*chad).sign = 0;
}

/* Conversion from chad type number to decimal. Returns:
 * 0 - OK
 * 1 - Overflow
 * 2 - Underflow  */
int s21_chad_to_decimal(s21_chad_decimal chad, s21_decimal *dec) {
  if (!dec) return S21_CONVERSION_ERROR;  // NULL check
  chad.sign = chad.digit + 59;            // reinitialize pointers
  chad.integer = chad.digit + 30;         //
  s21_decimal_initialize(dec);            // initialize result
  int err = S21_OK;
  int dec_begin, dec_end;  // find significant digits
  s21_number_position(&chad, &dec_begin, &dec_end);
  if (dec_begin >= dec_end) {      // check for 0
    if (dec_end > 0) dec_end = 0;  // zeroes in integral part are significant
    /* Set slice 29 digits long. */
    if (dec_begin - dec_end > 28) dec_end = dec_begin - 28;
    /* Check overflow and if there is, try making slice 28 digits long. */
    if (s21_check_overflow(chad.integer + dec_end)) {    // if not possible,
      dec_end ? ++dec_end : (err = *chad.sign ? 2 : 1);  // signal overflow
    }
    if (!err) {
      /* Shift significant numbers to integral part. */
      s21_buffer_shift(chad.digit, -dec_end);
      s21_decrease_precision(&chad);        // for School21 autotests
      err = s21_chad_round(&chad, 0, 'b');  // rounding may cause false overflow
      if (err && dec_end) {  // correct it by decreasing precision
        s21_buffer_shift(chad.digit, -1);
        ++dec_end;
        err = s21_chad_round(&chad, 0, 'b');  // and round again
      }
    }
    if (!err) {
      s21_chad_to_decimal_conv(&chad, dec);  // run conversion arithmetics
      s21_set_scale(dec, &chad, dec_end);    // write scale into result
    }
  }
  return err;
}

/* Runs arithmetic for conversion from chad type number to decimal. */
void s21_chad_to_decimal_conv(s21_chad_decimal *chad, s21_decimal *dec) {
  s21_chad_decimal bits0, bits1, bits2, pow2;
  s21_initialize(&bits0);
  s21_initialize(&bits1);
  s21_initialize(&bits2);
  s21_init_pow2_64(&pow2);            // get 2^64 value as chad type number
  s21_chad_div(chad, &pow2, &bits2);  // divide input by 2^64, get upper digits
  s21_chad_to_uint32((*dec).bits + 2, &bits2);  // directly write into bits[2]
  s21_chad_round(&bits2, 0, 't');
  s21_chad_mul_abs(&bits2, &pow2, &bits0);  // restore upper digits
  s21_chad_sub_abs(chad, &bits0, chad);     // and substract them from number
  s21_init_pow2_32(&pow2);            // get 2^32 value as chad type number
  s21_chad_div(chad, &pow2, &bits1);  // divide rest of input by 2^32
  s21_chad_to_uint32((*dec).bits + 1, &bits1);  // write into bits[1]
  s21_chad_round(&bits1, 0, 't');
  s21_initialize(&bits2);                   // nullify temporary variable
  s21_chad_mul_abs(&bits1, &pow2, &bits2);  // restore middle digits
  s21_chad_sub_abs(chad, &bits2, chad);     // substract them from number
  s21_chad_to_uint32((*dec).bits, chad);  // only lower digits left, write them
}

/* Conversion from decimal to chad type number. Returns:
 * 0 - OK
 * 1 - Conversion error  */
void s21_decimal_to_chad(s21_decimal dec, s21_chad_decimal *chad) {
  *((*chad).sign) = (dec.bits[3] & 0x80000000U) ? '-' : 0;  // set output sign
  dec.bits[3] &= ~0x80000000U;                              // delete sign
  dec.bits[3] = dec.bits[3] >> 16;                          // prep scale
  s21_chad_decimal tmp1, tmp2, pow2;
  s21_init_pow2_32(&pow2);  // get 2^32 value as chad type number
  for (int i = 0, j; i < 3; ++i) {
    if (!dec.bits[i]) continue;  // skip empty bits
    j = 0;
    s21_initialize(&tmp1);
    while (dec.bits[i] != 0) {                 // break bits[i] into digits
      *(tmp1.integer + j) = dec.bits[i] % 10;  // write them to chad type digits
      dec.bits[i] /= 10;                       // digit by digit
      j++;
    }
    if (i) {
      if (i == 2) {               // for upper digits
        s21_init_pow2_64(&pow2);  // get 2^64 value as chad type number
      }
      s21_initialize(&tmp2);                  // for middle or upper digits,
      s21_chad_mul_abs(&tmp1, &pow2, &tmp2);  // multiply them by 2^32 or 2^64
    }
    s21_chad_add_abs(chad, i ? &tmp2 : &tmp1, chad);  // write converted bits[i]
  }                                                   // to result
  s21_buffer_shift((*chad).digit, -dec.bits[3]);  // and shift result by scale
}

/* Convert absolute value of chad type number to unsigned int. */
void s21_chad_to_uint32(unsigned int *dst, s21_chad_decimal *src) {
  for (size_t i = 0; i <= 9; i++) {
    *dst += *((*src).integer + i) * (unsigned)pow(10, i);
  }
}

/* Convert chad type number to int. */
void s21_chad_to_int(int *dst, s21_chad_decimal *src) {
  if (*src->sign) {
    for (size_t i = 0; i <= 9; i++) {
      *dst -= src->integer[i] * pow(10, i);
    }
  } else {
    for (size_t i = 0; i <= 9; i++) {
      *dst += src->integer[i] * pow(10, i);
    }
  }
}

/* Convert temporary to float. */
void s21_chad_to_float(float *dst, s21_chad_decimal *src) {
  if (*src->sign) {
    for (int i = 28; i >= -28; i--) {
      *dst -= (float)src->integer[i] * powf(10, i);
    }
  } else {
    for (int i = 28; i >= -28; i--) {
      *dst += (float)src->integer[i] * powf(10, i);
    }
  }
}

/* Handle digit overflow. Chad type arithmetics causes individual digits
 * overflow/underflow, this function straightens them up. */
int s21_correct_digit_overflow(s21_chad_decimal *chad) {
  int err = S21_OK;
  s21_chad_decimal tmp;
  for (int i = 0; i < 60; i++) {  // copy to temporary storage
    tmp.digit[i] = chad->digit[i];
  }
  for (int i = 0; i < 58; i++) {
    if (tmp.digit[i] > 9) {                   // if digit >= 10
      tmp.digit[i + 1] += tmp.digit[i] / 10;  // increase next digit
      tmp.digit[i] %= 10;                     // and decrease current
    } else if (tmp.digit[i] < 0) {            // if digit < 0
      tmp.digit[i] += 10;                     // add 10
      tmp.digit[i + 1]--;                     // and decrement next digit
    }
  }
  if (tmp.digit[58] > 9 || tmp.digit[58] < 0) {  // Full type overflow,
    err++;                                       // signal it
    if (tmp.digit[59]) {  // Check sign, set overflow signal to underflow
      err++;
    }
  } else {
    for (int i = 0; i < 60; i++) {  // copy from temporary storage
      chad->digit[i] = tmp.digit[i];
    }
  }
  return err;
}

/* Multiply mantissa by 10^shift. Done by array shift. */
void s21_buffer_shift(digit *digit, int shift) {
  if (shift > 0) {                       // Multiply
    for (int i = 58; i >= shift; --i) {  // shift to high end
      *(digit + i) = *(digit + i - shift);
    }
    for (int i = 0; i < shift; ++i) {  // set lowest digits to 0
      *(digit + i) = 0;
    }
  } else if (shift < 0) {                // Divide
    for (int i = -shift; i < 59; ++i) {  // shift to low end
      *(digit + i + shift) = *(digit + i);
    }
    for (int i = 59 + shift; i < 59; ++i) {  // set highest digits to 0
      *(digit + i) = 0;
    }
  }
}

/* Finds the index of the biggest and smallest digit of the number. */
void s21_number_position(s21_chad_decimal *array, int *begin, int *end) {
  *begin = -30;
  *end = 28;
  for (int i = 28; i > -30; i--) {  // skip zeroes from high end
    if (array->integer[i] != 0) {
      *begin = i;
      i = -31;  // break cycle
    }
  }
  for (int i = -30; i < 28; i++) {  // skip zeroes from low end
    if (array->integer[i] != 0) {
      *end = i;
      i = 29;  // break cycle
    }
  }
}

/* Some tests comparing output of this lib with other decimal libs were found to
 * fail due to extra precision implemented here. This function decreases
 * precision for compatibility. */
void s21_decrease_precision(s21_chad_decimal *chad) {
  if (S21_DECREASE_PRECISION) {
    int begin, end;
    s21_number_position(chad, &begin, &end);
    if (begin >= 0) {
      memset((*chad).digit, 0, begin + 1);  // truncate to 30 significant digits
    }
  }
}

/////////////////////////////////////////////////////////////////
/* ---------------- TEMPORARY TYPE COMPARISON ---------------- */
/////////////////////////////////////////////////////////////////
/* Compare chad type numbers. */
int s21_chad_compare(s21_chad_decimal *num1, s21_chad_decimal *num2) {
  int res = 0;
  int count_of_0_chars = 0;
  int index = -1;
  while ((num1->sign + index) >=
         num1->digit) {  // check from the end of the string to the start of the
                         // string
    if ((*(num1->sign + index) == 0) &&
        (*(num2->sign + index) == 0)) {  // if num1[i] == 0 and num 2[i] == 0,
                                         // increase the count_of_0_chars
      count_of_0_chars++;
    }
    index--;
  }
  if (count_of_0_chars != 59) {  // if both strings are not equal to 0
    if (*num1->sign != 0) {
      if (*num2->sign != 0) {  // if both with '-', need to compare modules
        res = (-1) * s21_chad_module_compare(num1, num2);
      } else  // if first with '-' and 2nd with '+', first is less than 2nd
        res = -1;
    } else if (*num2->sign != 0) {  // if first with '+' and 2nd with '-', first
                                    // is greater than 2nd
      res = 1;
    } else
      res = s21_chad_module_compare(
          num1,
          num2);  // if first with '+' and 2nd with '+', need to compare modules
  }
  return res;
}

/* Compare absolute values of chad type numbers. */
int s21_chad_module_compare(s21_chad_decimal *num1, s21_chad_decimal *num2) {
  int res = 0;  // set that num1 = num2
  int flag = 1;
  int index = -1;
  while (((num1->sign + index) >= num1->digit) &&
         (flag ==
          1)) {  // check from the end of the string to the start of the string
    if (*(num1->sign + index) >
        *(num2->sign + index)) {  // if num1[i] > num 2[i], num1> num2 and need
                                  // to stop comparing process (flag = 0)
      res = 1;
      flag = 0;
    }
    if (*(num1->sign + index) <
        *(num2->sign + index)) {  // if num1[i] < num 2[i], num1 < num2 and need
                                  // to stop comparing process (flag = 0)
      res = -1;
      flag = 0;
    }
    index--;
  }
  return res;
}

/////////////////////////////////////////////////////////////////
/* ---------------- TEMPORARY TYPE ARITHMETIC ---------------- */
/////////////////////////////////////////////////////////////////
/* Rounding at specified digit. */
int s21_chad_round(s21_chad_decimal *chad, int i, int mode) {
  (i < -28) ? i = -28 : (i > 28) ? i = 28 : 0;
  switch (mode) {
    case 't':  // truncate
      for (--i; i > -31; --i) *((*chad).integer + i) = 0;
      break;
    case 'r':  // round
      (*((*chad).integer + i - 1) > 4) ? ++*((*chad).integer + i) : 0;
      for (--i; i > -31; --i) *((*chad).integer + i) = 0;
      break;
    case 'f':  // floor
      if (*(*chad).sign) {
        s21_chad_round_check_digits(chad, i, 1);
      }
      for (--i; i > -31; --i) *((*chad).integer + i) = 0;
      break;
    case 'b':  // banker's
    case 'e':  // round to even
      if ((*((*chad).integer + i - 1) == 5) && !(*((*chad).integer + i) % 2)) {
        s21_chad_round_check_digits(chad, i, 2);
      } else {
        (*((*chad).integer + i - 1) > 4) ? ++*((*chad).integer + i) : 0;
      }
      for (--i; i > -31; --i) *((*chad).integer + i) = 0;
      break;
  }
  int err = s21_correct_digit_overflow(chad);
  !err ? err = s21_check_overflow((*chad).integer) : 0;
  return err;
}

/* Check all digits after current minus offset to see if there is a non-zero
 * digit. For banker's and floor rounding modes. */
void s21_chad_round_check_digits(s21_chad_decimal *chad, int i, int offset) {
  for (int j = i - offset; j > -31; --j) {
    if (*((*chad).integer + j)) {  // if there is a non-zero digit,
      ++*((*chad).integer + i);    // increment digit at rounding point
      j = -32;
    }
  }
}

/* Round number to 7 significant digits to prepare conversion to float. */
void s21_chad_round_to_float(s21_chad_decimal *tmp) {
  int number_begin, number_end;  // find leading significant digit and call
  s21_number_position(tmp, &number_begin, &number_end);  // banker's rounding
  s21_chad_round(tmp, number_begin - 6, 'b');  // at it's position minus 6
}

/* Division. */
int s21_chad_div(s21_chad_decimal *num1, s21_chad_decimal *num2,
                 s21_chad_decimal *res) {
  int err = S21_OK;
  int v1_begin, v2_begin, v1_end, v2_end, result_offset;
  s21_chad_decimal v1, v2;

  s21_chad_copy(&v1, num1);  // numbers will be destroyed in process
  s21_chad_copy(&v2, num2);  // so we copy them first
  s21_number_position(&v1, &v1_begin, &v1_end);  // get information
  s21_number_position(&v2, &v2_begin, &v2_end);  // about numbers

  if (v2_begin < v2_end) {  // check division by 0
    err = S21_DIVISION_BY_ZERO;
  } else if (v1_begin < v1_end) {  // check division of 0
    s21_initialize(res);
  } else {                                // if no zeroes, begin calculations
    result_offset = v1_begin - v2_begin;  // find leading digit of result
    s21_buffer_shift(v2.digit, result_offset);
    s21_chad_div_set_divisor(&v1, &v2, &result_offset);
    if (result_offset > 28) {  // check overflow/undreflow
      err = ((*v1.sign && *v2.sign) || (!*v1.sign && !*v2.sign)) ? 1 : 2;

    } else {          // if no errors, proceed
      *(*res).sign =  // set sign
          ((*v1.sign && *v2.sign) || (!*v1.sign && !*v2.sign)) ? 0 : '-';

      /* Run arithmetics. */
      err = s21_chad_div_calc(&v1, &v2, res, v1_begin, v1_end, v2_begin, v2_end,
                              result_offset);
    }
  }
  !err ? err = s21_correct_digit_overflow(res) : 0;
  return err;
}

/* Arithmetics for chad type number division. */
int s21_chad_div_calc(s21_chad_decimal *v1, s21_chad_decimal *v2,
                      s21_chad_decimal *res, int v1_begin, int v1_end,
                      int v2_begin, int v2_end, int result_offset) {
  int err = 0;
  /* Set precision limits for efficiency. */
  const int endcalc_digit = s21_max(-31, result_offset - 31);

  /* Set divisor in position, run substractions until precision is met */
  while (result_offset - 1 > endcalc_digit && !err) {
    /* Run substractions while dividend > divisor */
    /* Substraction digit by digit. */
    for (int i = v1_begin, j = 0; s21_min(i, result_offset + j) > endcalc_digit;
         --i, --j) {
      *((*v1).integer + i) -= *((*v2).integer + i);
    }
    ++*((*res).integer + result_offset);
    err = s21_correct_digit_overflow(v1);  // finalize substraction
    s21_number_position(v1, &v1_begin, &v1_end);
    s21_number_position(v2, &v2_begin, &v2_end);

    if (v1_begin < v1_end) {  // check if dividend is 0
      result_offset = endcalc_digit - 1;
    } else {  // if not, set divisor for next round of substractions
      s21_buffer_shift((*v2).digit, v1_begin - v2_begin);
      result_offset += v1_begin - v2_begin;  // recalc leading digit of result
      s21_chad_div_set_divisor(v1, v2, &result_offset);
    }
  }
  return err;
}

/* Check if necessary and shift divisor right for next round of division. */
void s21_chad_div_set_divisor(s21_chad_decimal *v1, s21_chad_decimal *v2,
                              int *result_offset) {
  if (s21_chad_module_compare(v1, v2) == -1) {
    s21_buffer_shift((*v2).digit, -1);
    --*result_offset;  // correct it by 1 if necessary
  }
}

/* Multiplication. */
int s21_chad_mul(s21_chad_decimal *array1, s21_chad_decimal *array2,
                 s21_chad_decimal *add_result) {
  int error = S21_OK;
  if ((array1->sign[0] != 0 && array2->sign[0] != 0) ||
      (array1->sign[0] == 0 && array2->sign[0] == 0)) {  // set sign
    add_result->sign[0] = 0;
  } else {
    add_result->sign[0] = 1;
  }
  error = s21_chad_mul_abs(array1, array2, add_result);  // call arithmetics

  return error;
}

/* Multiplication of absolute values, result always positive. */
int s21_chad_mul_abs(s21_chad_decimal *array1, s21_chad_decimal *array2,
                     s21_chad_decimal *result) {
  int error = S21_OK;
  /* Defining of the indexes of the beginning and end from both numbers. */
  int number1_begin, number1_end, number2_begin, number2_end, result_begin;

  s21_number_position(array1, &number1_begin, &number1_end);
  s21_number_position(array2, &number2_begin, &number2_end);

  /* Index of the greatest digit of the result. */
  result_begin = number1_begin + number2_begin;
  if (array1->integer[number1_begin] * array2->integer[number2_begin] > 9) {
    result_begin++;
  }
  if (result_begin > 28) {  // check if leading digit of result is out of range
    if ((array1->sign[0] != 0 && array2->sign[0] != 0) ||
        (array1->sign[0] == 0 && array2->sign[0] == 0)) {
      error = S21_OVERFLOW;  // if it is, signal overflow
    } else {
      error = S21_UNDERFLOW;  // or underflow
    }
  } else if (result_begin < -28) {  // if it is in range, run arithmetics
  } else {  // multiply number 1 by each digit of number 2
    for (int i = number1_begin; (i >= number1_end) && error == 0; i--) {
      for (int j = number2_begin; j >= number2_end; j--) {
        if (i + j >= -30) {  // stay in range for low digits
          result->integer[i + j] += array1->integer[i] * array2->integer[j];
        }
      }
      /* After each pass of multiplication by digit, run correction. */
      error = s21_correct_digit_overflow(result);
    }
  }
  return error;
}

/* Substraction. */
int s21_chad_sub(s21_chad_decimal *num1, s21_chad_decimal *num2,
                 s21_chad_decimal *sub_result) {
  int res = S21_CALCULATION_ERROR;  // res = not OK
  if (((*num1->sign != 0) && (*num2->sign != 0)) ||
      ((*num1->sign == 0) &&
       (*num2->sign == 0))) {  // if signs of numbers are equal
    int result_of_module_compare = s21_chad_module_compare(num1, num2);
    if (result_of_module_compare == 1) {
      res = s21_chad_sub_abs(num1, num2, sub_result);  // call substraction
      *sub_result->sign = *num1->sign;
    } else if (result_of_module_compare == -1) {
      res = s21_chad_sub_abs(num2, num1, sub_result);  // call substraction
      switch (*num1->sign) {                           // set sign
        case 0:
          *(sub_result->sign) = '-';
          break;
        default:
          *(sub_result->sign) = 0;
          break;
      }
    } else {
      res = s21_chad_sub_abs(num1, num2, sub_result);  // call substraction
    }
  } else {  // if signs of numbers are not equal
    res = s21_chad_add_abs(num1, num2, sub_result);  // call addition
    *sub_result->sign = *num1->sign;  // always set the sign of the first number
  }
  return res;
}

/* Substraction of absolute values, result always positive. num1 MUST have
 * bigger absolute value than num2. */
int s21_chad_sub_abs(  //переделать на инт
    s21_chad_decimal *num1, s21_chad_decimal *num2,
    s21_chad_decimal *sub_result) {  // num 1 is always greater than num2
  int index = -1;
  while ((num1->sign + index) >=
         num1->digit) {  // check from the end of the string to the start of the
                         // string
    *(sub_result->sign + index) =
        *(num1->sign + index) -
        *(num2->sign +
          index);  // set into sub_result Substraction of num1 and num2
    index--;
  }
  return s21_correct_digit_overflow(sub_result);
}

/* Addition. */
int s21_chad_add(s21_chad_decimal *array1, s21_chad_decimal *array2,
                 s21_chad_decimal *add_result) {
  int res = S21_OK;
  if ((array1->sign[0] != 0 && array2->sign[0] != 0) ||
      (array1->sign[0] == 0 && array2->sign[0] == 0)) {
    res = s21_chad_add_abs(array1, array2, add_result);
    add_result->sign[0] = array1->sign[0];
  } else {  // if signs of numbers are not equal, prepare for substraction
    int array1_is_bigger = s21_chad_module_compare(array1, array2);
    if (array1_is_bigger == 1) {          // compare absolute values
      int result_sign = array1->sign[0];  // set sign
      res = s21_chad_sub_abs(array1, array2, add_result);  // call sub
      add_result->sign[0] = result_sign;
    } else if (array1_is_bigger == -1) {
      int result_sign = array2->sign[0];                   // set sign
      res = s21_chad_sub_abs(array2, array1, add_result);  // call sub
      add_result->sign[0] = result_sign;
    } else {
      s21_initialize(add_result);  // if equal absolute values, write 0
    }
  }

  return res;
}

/* Addition of absolute values, result always positive. */
int s21_chad_add_abs(s21_chad_decimal *array1, s21_chad_decimal *array2,
                     s21_chad_decimal *result) {
  for (int i = -28; i <= 28; i++) {  // add digit by digit
    result->integer[i] = array1->integer[i] + array2->integer[i];
  }
  return s21_correct_digit_overflow(result);
}

/////////////////////////////////////////////////////////////////
/* ------------------ CONSTANTS AND LIMITS ------------------- */
/////////////////////////////////////////////////////////////////
/* Check overflow / underflow. */
int s21_check_overflow(const digit *integer) {
  int overflow = S21_FALSE;
  const digit max_decimal[29] = {7, 9, 2, 2, 8, 1, 6, 2, 5, 1, 4, 2, 6, 4, 3,
                                 3, 7, 5, 9, 3, 5, 4, 3, 9, 5, 0, 3, 3, 5};
  for (int i = 28; i >= 0; --i) {  // compare number to max decimal
    if (*(integer + i) != *(max_decimal + 28 - i)) {
      overflow = (*(integer + i) > *(max_decimal + 28 - i));
      *(integer + 29) ? overflow *= 2 : 0;
      i = -1;
    }
  }
  return overflow;
}

/* Check if decimal is within int32 type limits. */
int s21_check_int_overflow(s21_decimal *dec) {
  int overflow = S21_FALSE;
  s21_decimal max_int, min_int;
  s21_from_int_to_decimal(INT_MAX, &max_int);  // get max and min int32 values
  s21_from_int_to_decimal(INT_MIN, &min_int);  // as chad type numbers
  if (s21_is_greater(*dec, max_int) || s21_is_less(*dec, min_int)) {
    overflow = S21_TRUE;
  }
  return overflow;
}

/* Write chad value for 2^32. */
void s21_init_pow2_32(s21_chad_decimal *chad_decimal) {
  memset((*chad_decimal).digit, 0, 30);
  (*chad_decimal).sign = (*chad_decimal).digit + 59;
  (*chad_decimal).integer = (*chad_decimal).digit + 30;
  memcpy((*chad_decimal).integer,
         "\6\11\2\7\6\11\4\11\2\4\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 30);
}

/* Write chad value for 2^64. */
void s21_init_pow2_64(s21_chad_decimal *chad_decimal) {
  memset((*chad_decimal).digit, 0, 30);
  (*chad_decimal).sign = (*chad_decimal).digit + 59;
  (*chad_decimal).integer = (*chad_decimal).digit + 30;
  memcpy((*chad_decimal).integer,
         "\6\1\6\1\5\5\11\0\7\3\7\0\4\4\7\6\4\4\10\1\0\0\0\0\0\0\0\0\0\0", 30);
}