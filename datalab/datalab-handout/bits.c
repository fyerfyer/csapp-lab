/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  return ~((~(x & (~y))) & (~((~x) & y)));
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return (1<<31);
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  return !(x ^ (~tmin()));
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  int comp = 0xAA;
  int cmp1 = !((x & comp) ^ comp); // judge if the and between 8 bits of x and comp equals to comp
  int cmp2 = !(((x>>8) & comp) ^ comp);
  int cmp3 = !(((x>>16) & comp) ^ comp);
  int cmp4 = !(((x>>24) & comp) ^ comp);
  return (cmp1 & cmp2 & cmp3 & cmp4);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  int comp = 0x30>>4;
  int cmp1 = !((comp & (x>>4)) ^ comp); // judge if it's 0x3...
  int is_9_or_8 = !((x ^ 0x9) & (x ^ 0x8)); // deal with the last four digits
  int forth_digit_not_1 = !((x>>3) & 1);
  return cmp1 & (is_9_or_8 | forth_digit_not_1);
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  int mask_for_1 = !x - 1;
  int mask_for_0 = !!x - 1;
  return (mask_for_1 & y) | (mask_for_0 & z);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  return !!((y + negate(x)) + 1);
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  return ((x >> 31) & 1) ^ ((negate(x) >> 31) & 1);
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  int self = x;
  int sign = (x >> 31);
  x = (x ^ sign) - sign;
  int neg_self = x;

  int bits = 1;

  // a simply binary search
  bits += (!!(x >> 16)) << 4;
  x >>= ((!!(x >> 16)) << 4);

  bits += (!!(x >> 8)) << 3;
  x >>= ((!!(x >> 8)) << 3);

  bits += (!!(x >> 4)) << 2;
  x >>= ((!!(x >> 4)) << 2);

  bits += (!!(x >> 2)) << 1;
  x >>= ((!!(x >> 2)) << 1);

  bits += !!(x >> 1);

  // then we need to deal with 3 cases:
  // case 1: 0
  // case 2: positive number: we must leave an empty bit for the positive number, or it will be negative.
  // case 3: negative number: this is a little difficult, I divide it into two cases:

  // case a: both self and neg_self have same suffix: such as -1 and 0x80000000. 
  // these pairs of numbers have the following structure: the x former bits of self and neg_self are 000... and 111..., and the rest are all the same. 
  // for this case, we don't need to supply an addition bit, since the bit after 000... is enough to represent the number.
  // to identify this structure, we do the following steps:
  // 1. do neg_self - self, then we get a string of number with structure: 1111...(all 1)0000...(all 0), we name it check_binary
  // 2. the check_binary has this property: x & (x + 1) equals 0.
  
  // case b: self and neg_self have different suffix: such as -5, its suffix(leave along the string of 1111...) is 1011 for self and 0101 for neg_self.
  // this is the opposite of case b, we deal it the same.

  // But why we need do seperate these two cases? This is because if the number pair don't have the "suffix-equal" property, when we negate it, it will lose one bit:
  // take -5 as an example: the self and neg_self are 1011 and 0101, if we use the "bits" result of 0101, we find that we only need 3 bits. 
  // But if we use the twos-complement, the 101 will represent -3. So we need to take the front '0' bit into consideration.
  // However, if the suffix is the same, like -1(1111...11) and its neg (0000...01), we can simply take '1' to represent it, for both from self to neg_self and vice versa are the same, the number won't change 

  int check_binary = ~(self - neg_self);
  int not_neg = !(self ^ neg_self);
  int is_zero = !!self;
  int check = !!(check_binary & (check_binary + 1));

  return bits + ((check | not_neg) & self);
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  unsigned sign = uf >> 31;
  unsigned exponent = (uf >> 23) & 0xFF;
  unsigned mantissa = uf & 0x7FFFFF;

  // NaN and infinity
  if (exponent == 0xFF && mantissa != 0) return uf;
  if (exponent == 0xFF && mantissa == 0) return uf;

  exponent += 1;

  // overflow
  if (exponent == 0xFF) return (sign << 31) | (exponent << 23);

  return (sign << 31) | (exponent << 23) | mantissa;
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  unsigned sign = uf >> 31;
  unsigned exponent = (uf >> 23) & 0xFF;
  unsigned mantissa = uf & 0x7FFFFF;

  // NaN and infinity
  if (exponent == 0xFF) return 0x80000000u;

  int E = exponent - 127;
  if (E < 0) return 0;
  if (E > 31) return 0x80000000u;

  int num;

  if (E >= 23) num = mantissa << (E - 23);
  else num = mantissa >> (23 - E);

  if (sign > 0) return num;
  else return negate(num); 
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  if (x > 127) return 0x7F800000;
  if (x >= -126) return (x + 127) << 23;
  if (x >= -150) return 1 << (x + 150);
  return 0;
}
