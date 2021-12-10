#include <stdio.h>
#include "tfl.h"
#include <math.h> // used for debug


double eval_tfl16(tfl16_t value, int print);
void run_all_tests();
void test_tfl_sign();
void test_tfl_exponent();
void test_tfl_significand();
void test_eval_tfl16();
void test_tfl_equals();
void test_tfl_greaterthan();
void test_tfl_normalize();
void test_tfl_add();
void test_tfl_mul();

#define tfl16_one (0b0000011000000000)
#define tfl16_one_neg (0b1000011000000000)
#define tfl16_zero_neg (1 << 15)

int main() {
    run_all_tests();
    printf("\n");
}

void run_all_tests(){
    test_eval_tfl16();
    test_tfl_sign();
    test_tfl_exponent();
    test_tfl_significand();
    test_tfl_equals();
    test_tfl_greaterthan();
    test_tfl_normalize();
    test_tfl_add();
    test_tfl_mul();
}

double eval_tfl16(tfl16_t value, int print){
    // function used to print tfl16 for debugging. Use print=1 to print output
    int sign;

    tfl16_t sign_value = tfl_sign(value);
    if (sign_value == 0) sign = 0.0;
    else if (sign_value == tfl16_one) sign = 1.0;
    else sign = -1.0;

    int8_t exponent = tfl_exponent(value);

    uint16_t significand = tfl_significand(value);

    double x = sign * pow(2, exponent) * tfl_significand(value) / 1024;

    if (print) {
        for(int i=15; i>=0; i--){
            printf("%d", (value & 1 << i) >> i);
        }
        printf(" -> %d * 2^(%d) * %u / 1024 = %f\n", sign, exponent, significand, x);
    }

    return x;
}

tfl16_t tfl_sign(tfl16_t value){
    if (!tfl_significand(value)){ //significand is 0
        return 0;
    }
    else if (value & (1 << 15)){
        return tfl16_one_neg;
    }
    return tfl16_one;
}

int8_t tfl_exponent(tfl16_t value){
    int8_t exponent = (value & 0b0111110000000000) >> 10;
    if (exponent & 0b00010000){
        return (exponent | 0b11100000); //if exponent has 1 on MSB, return 0b111eeeee to ensure correct mapping
    }
    return exponent;
}

uint16_t tfl_significand(tfl16_t value){
    return (uint16_t)(value & 0b0000001111111111);
}

uint8_t tfl_equals(tfl16_t a, tfl16_t b){
    uint16_t significand_a = tfl_significand(a);
    uint16_t significand_b = tfl_significand(b);

    // when both significands are zero
    if (!significand_a && !significand_b){
        return 1;
    }

    int8_t exponent_a = tfl_exponent(a);
    int8_t exponent_b = tfl_exponent(b);

    // shifts significands and for each shift add to exponent
    // will not get out-of-range since -16 - 9 = -25 > –128
    if (significand_a){ //only begin left shifting if non-empty
        while (!(significand_a & (1 << 9))){ //while MSB of significant is 0
            significand_a = significand_a << 1;
            exponent_a--;
        }
    }
    if (significand_b){
        while (!(significand_b & (1 << 9))){
            significand_b = significand_b << 1;
            exponent_b--;
        }
    }

    return (tfl_sign(a) == tfl_sign(b)) && (exponent_a == exponent_b) && (significand_a == significand_b);
}

uint8_t greaterthan(uint16_t significand_a, int8_t exponent_a, uint16_t significand_b, int8_t exponent_b){
    //compare exponents and significants of 2 numbers
    if (exponent_a == exponent_b){
        if (significand_a == significand_b){
            return 0;
        } else {
            return significand_a > significand_b;
        }
    } else {
        return exponent_a > exponent_b;
    }
}

uint8_t tfl_greaterthan(tfl16_t a, tfl16_t b){
    uint16_t significand_a = tfl_significand(a);
    uint16_t significand_b = tfl_significand(b);

    // when both significands are zero
    if (!significand_a && !significand_b){
        return 0;
    }

    int8_t exponent_a = tfl_exponent(a);
    int8_t exponent_b = tfl_exponent(b);

    // shifts significands and add to exponent
    // will not get out-of-range since -16 - 9 = -25 > –128
    if (significand_a){ //only begin left shifting if non-empty
        while (!(significand_a & (1 << 9))){ //while MSB of significant is 0
            significand_a = significand_a << 1;
            exponent_a--;
        }
    }
    if (significand_b){
        while (!(significand_b & (1 << 9))){
            significand_b = significand_b << 1;
            exponent_b--;
        }
    }

    tfl16_t sign_a = tfl_sign(a);
    tfl16_t sign_b = tfl_sign(b);

    if ((sign_a == tfl16_one) && (sign_b == tfl16_one)){
        return greaterthan(significand_a, exponent_a, significand_b, exponent_b);
    } else if ((sign_a == tfl16_one_neg) && (sign_b == tfl16_one_neg)){
        return greaterthan(significand_b, exponent_b, significand_a, exponent_a);
    } else {
        // if sign_a is +1, it is true, because sign_b must be -1 or 0
        // if sign_a is -1, it is false, because sign_b must be +1 or 0
        // if sign_a is 0, it is true if b is -1
        return (sign_a == tfl16_one) || ((sign_a == 0) && (sign_b == tfl16_one_neg));
    }
}

uint16_t most_significant_bit(uint16_t x){
    // counts to most significant bit 
    int i = 0;
    while (x){
        x = x >> 1;
        i++;
    }
    return i;
}

tfl16_t tfl_normalize(uint8_t sign, int8_t exponent, uint16_t significand){
    uint16_t sign_16, exponent_16;

    // if sign is non-zero: -1, else: +1
    if (sign){
        sign_16 = 1 << 15;
    } else {
        sign_16 = 0;
    }

    // change exponent to 0 if significand is 0
    if (significand == 0){
        return sign_16;
    }

    // if significand is too large
    uint16_t a = (significand & 0b1111110000000000) >> 10;
    int i = 0; // number of shift rights
    if (a){
        // most significand bit of "a" (out-of-range significand)
        i = most_significant_bit(a);
        significand = significand >> i; // the i least significand bits are removed
    }

    // add to exponent if in range -- also works for i=0
    if (exponent + i <= (int8_t) 0b00001111){ //if increment does not cause positive overflow
        exponent += i;
    } else {
        int out_of_range = 1;
        // case when exponent is too large and was significand not:
        if (!a){
            int n_possible_shifts = 10 - most_significant_bit(significand); //possible right shifts
            int n_needed_shifts = exponent - 0b1111; //needed decrements to prevent overflow
            if (n_possible_shifts >= n_needed_shifts){
                significand = significand << n_needed_shifts;
                exponent -= n_needed_shifts; // will not be negative
                out_of_range = 0;
            }
        }
        // if out-of-range: change significand and exponent to max (signify overflow)
        if (out_of_range){
            exponent = 0b01111;
            significand = 0b1111111111;
        }
    }

    // exponent: set min
    if ((exponent & (1 << 7)) >> 7){//if exponent is negative
        // if too small:
        if (exponent < (int8_t) 0b11110000){ // -16 in int8
            // try to right shift significand to compensate
            int n_possible_shifts = most_significant_bit(significand)-1; //can potentially shift untill MSB is LSB
            int n_needed_shifts = -16 - exponent; //will be a positive val
            if (n_possible_shifts >= n_needed_shifts){
                significand = significand >> n_needed_shifts;
                exponent += n_needed_shifts; // will not be positive
            } else {
                // if not possible to shift : return zero with given sign
                return sign_16;
            }
        } //if not too small
        exponent = exponent & 0b00011111; // change to int5 negative
    }

    // shift in place
    exponent_16 = exponent;
    exponent_16 = exponent_16 << 10;
    
    return sign_16 | exponent_16 | significand;
}

tfl16_t tfl_add(tfl16_t a, tfl16_t b){
    uint16_t significand_final;
    uint8_t sign_final;

    uint16_t significand_a = tfl_significand(a);
    uint16_t significand_b = tfl_significand(b);

    int8_t exponent_a = tfl_exponent(a);
    int8_t exponent_b = tfl_exponent(b);

    tfl16_t sign_a = tfl_sign(a);
    tfl16_t sign_b = tfl_sign(b);

    // increment lowest to get same exponent
    while (exponent_a > exponent_b){
        significand_b = significand_b >> 1; //might disappear, if b is a lot smaller than a
        exponent_b++;
    }
    while (exponent_a < exponent_b){
        significand_a = significand_a >> 1;
        exponent_a++;
    }

    // simple addition if same sign
    if (sign_a == sign_b){
        significand_final = significand_a + significand_b;
    
    } else if(sign_a == tfl16_one){
        // convert to int32 to calculate different signs
        int32_t significand_int32 = (int32_t)significand_a - (int32_t)significand_b;
        if (significand_int32 < 0){
            significand_final = (uint16_t)(significand_int32 * (-1));
            sign_a = tfl16_one_neg;
        } else {
            significand_final = (uint16_t)significand_int32;
        }
    } else {
        int32_t significand_int32 = (int32_t)significand_b - (int32_t)significand_a;
        if (significand_int32 < 0){
            significand_final = (uint16_t)(significand_int32 * (-1));
        } else {
            significand_final = (uint16_t)significand_int32;
            sign_a = tfl16_one;
        }
    }
    sign_final = sign_a != tfl16_one;

    return tfl_normalize(sign_final, exponent_a, significand_final);
}

tfl16_t tfl_mul(tfl16_t a, tfl16_t b){
    uint16_t signif_a = tfl_significand(a);    
    uint16_t signif_b = tfl_significand(b);
    int8_t exponent_a = tfl_exponent(a);
    int8_t exponent_b = tfl_exponent(b);
    uint16_t MSB_a = most_significant_bit(signif_a);
    uint16_t MSB_b = most_significant_bit(signif_b);
    int8_t i = -10; //initial "bias" in exponent, due to denominator in frac

    //controls overflow over 16 bit 
    if (MSB_a + MSB_b > 16){
        int needed_shifts = (MSB_a + MSB_b)-16; //n0 of bits exceeded 16
        if (MSB_a > MSB_b){
            signif_a = signif_a >> needed_shifts;
            i += needed_shifts;
        }else{
            signif_b = signif_b >> needed_shifts;
            i += needed_shifts;
        }
    }
    uint16_t significand = signif_a * signif_b;
    uint8_t sign = (tfl_sign(a) == tfl16_one) != (tfl_sign(b) == tfl16_one);
    int8_t exponent = exponent_a + exponent_b + i;

    return tfl_normalize(sign, exponent, significand);
}


void test_eval_tfl16(){
    printf("Testing eval_tfl16:");

    // 1 test zero
    int i = 1;
    int j = 0;
    tfl16_t x1 = 0;
    if (eval_tfl16(x1,0) == 0.0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 2 test negative zero
    i++;
    x1 = tfl16_zero_neg;
    if (eval_tfl16(x1,0) == 0.0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 3 test 1
    i++;
    x1 = tfl16_one;
    if (eval_tfl16(x1,0) == 1.0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 4 test -1
    i++;
    x1 = tfl16_one_neg;
    if (eval_tfl16(x1,0) == -1.0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 5 test 1.5
    i++;
    x1 = 0b0000011100000000;
    if (eval_tfl16(x1,0) == 1.5){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 6 test 0.5
    i++;
    x1 = 0b0000100010000000;
    if (eval_tfl16(x1,0) == 0.5){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 7 test 0.333
    i++;
    x1 = 0b0000000101010101;
    double x2 = 1 * pow(2, 0) * 341 / 1024;
    if (eval_tfl16(x1,0) == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 8 test 3.141
    i++;
    x1 = 0b0000101100100100;
    x2 = 1 * pow(2, 2) * 804 / 1024;
    if (eval_tfl16(x1,0) == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 9 test 1000
    i++;
    x1 = 0b0010101111101000;
    if (eval_tfl16(x1,0) == 1000.0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 10 test 0.0075
    i++;
    x1 = 0b0111000001111011;
    x2 = 1 * pow(2, -4) * 123 / 1024;
    if (eval_tfl16(x1,0) == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    printf("\n   %d/%d\n", j,i);
}

void test_tfl_sign(){
    printf("Testing tfl_sign:");

    // 1 test some positve number
    int i = 1;
    int j = 0;
    tfl16_t x1 = 0b0010010010001001;
    if (tfl_sign(x1) == tfl16_one){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 2 test some negative number
    i++;
    tfl16_t x2 = 0b1010011000101011;
    if (tfl_sign(x2) == tfl16_one_neg){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 3 test positve 0
    i++;
    tfl16_t x3 = 0;
    if (tfl_sign(x3) == 0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 4 test negative 0
    i++;
    tfl16_t x4 = tfl16_zero_neg;
    if (tfl_sign(x4) == 0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    printf("\n   %d/%d\n", j,i);
}

void test_tfl_exponent(){
    printf("Testing tfl_exponent:");

    // 1 test some positive number
    int i = 1;
    int j = 0;
    tfl16_t x1 = 0b0100010010010111;
    if (tfl_exponent(x1) == -15){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 2 test some negative number
    i++;
    x1 = 0b1010011000101011;
    if (tfl_exponent(x1) == 9){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 3 test zero
    i++;
    x1 = 0;
    if (tfl_exponent(x1) == 0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 4 test zero exponent
    i++;
    x1 = 0b1000001000101011;
    if (tfl_exponent(x1) == 0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    printf("\n   %d/%d\n", j,i);
}

void test_tfl_significand(){
    printf("Testing tfl_significand:");

    // 1 test some positive number
    int i = 1;
    int j = 0;
    tfl16_t x1 = 0b0011000110110010;
    if (tfl_significand(x1) == 0b0110110010){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 2 test some negative number
    i++;
    tfl16_t x2 = 0b1000011000101011;
    if (tfl_significand(x2) == 0b1000101011){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 3 test zero
    i++;
    tfl16_t x3 = 0;
    if (tfl_significand(x3) == 0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    printf("\n   %d/%d\n", j,i);
}

void test_tfl_equals(){
    printf("Testing tfl_equal:");

    // 1 test some equal numbers
    int i = 1;
    int j = 0;
    tfl16_t x1 = 0b0011000110110010;
    tfl16_t x2 = 0b0011000110110010;
    if (tfl_equals(x1,x2)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 2 test some non equal numbers
    i++;
    x1 = 0b0011000110110010;
    x2 = 0b1011000110110010;
    if (tfl_equals(x1,x2) == 0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 3 test 0 and -0
    i++;
    x1 = 0;
    x2 = tfl16_zero_neg;
    if (tfl_equals(x1,x2)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 4 test two types of ones
    i++;
    x1 = tfl16_one;
    x2 = 0b0000100100000000;
    if (tfl_equals(x1,x2)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 5 test 0 and number
    i++;
    x1 = 0;
    x2 = 0b1011000110110010;
    if (tfl_equals(x1,x2) == 0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 6 test 0 and number with non zero exponent but zero significand
    i++;
    x1 = 0;
    x2 = 0b1011000000000000;
    if (tfl_equals(x1,x2)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 7 test numbers with different expoent but same significand
    i++;
    x1 = 0b0011000110110010;
    x2 = 0b0111000110110010;
    if (tfl_equals(x1,x2) == 0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 8 test numbers with same expoent but different significand
    i++;
    x1 = 0b0011000110110010;
    x2 = 0b0011000110110011;
    if (tfl_equals(x1,x2) == 0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    printf("\n   %d/%d\n", j,i);
}

void test_tfl_greaterthan(){
    printf("Testing tfl_greaterthan:");

    // 1 test two positive numbers, with same exponent, x1 > x2
    int i = 1;
    int j = 0;
    tfl16_t x1 = 0b0011001000000000;
    tfl16_t x2 = 0b0011000100000000;
    if (tfl_greaterthan(x1,x2)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 2 test two positive numbers, with same significand, x1 < x2
    i++;
    x1 = 0b0001001000000000;
    x2 = 0b0010001000000000;
    if (tfl_greaterthan(x1,x2) == 0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 3 test two posive numbers, x1 > x2
    i++;
    x1 = tfl16_one;
    x2 = 0b0000100010000000; // 0.5
    if (tfl_greaterthan(x1,x2)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 4 test two negative numbers, x1 < x2
    i++;
    x1 = 0b1000011000000000; // -1.0
    x2 = 0b1000100010000000; // -0.5
    if (tfl_greaterthan(x1,x2) == 0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 5 test two numbers with different signs, x1 > x2
    i++;
    x1 = tfl16_one;
    x2 = tfl16_one_neg;
    if (tfl_greaterthan(x1,x2)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 6 test two equal numbers
    i++;
    x1 = tfl16_one;
    x2 = 0b0000100100000000;
    if (tfl_greaterthan(x1,x2) == 0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 7 test number with negative exponent, x1 > x2
    i++;
    x1 = 0b0010000000000010;
    x2 = 0b0110000000000010;
    if (tfl_greaterthan(x1,x2)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 8 test largest number, x1 > x2
    i++;
    x1 = 0b0011111111111111;
    x2 = 0b0010000000000010;
    if (tfl_greaterthan(x1,x2)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 9 test 0 with positive number, x1 < x2
    i++;
    x1 = 0;
    x2 = tfl16_one;
    if (tfl_greaterthan(x1,x2) == 0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 10 test 0 with negative number, x1 > x2
    i++;
    x1 = 0;
    x2 = tfl16_one_neg;
    if (tfl_greaterthan(x1,x2)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 11 test +0 and -0
    i++;
    x1 = 0;
    x2 = tfl16_zero_neg;
    if (tfl_greaterthan(x1,x2) == 0){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    printf("\n   %d/%d\n", j,i);
}

void test_tfl_normalize(){
    printf("Testing tfl_normalize:");

    // 1 test some positve number
    int i = 1;
    int j = 0;
    uint8_t sign = 0;
    int8_t exponent = 1;
    uint16_t significand = 0b1000000000;
    tfl16_t x1 = tfl_normalize(sign,exponent,significand);
    tfl16_t x2 = tfl16_one;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 2 test all zeros
    i++;
    sign = 0;
    exponent = 0;
    significand = 0;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 3 test all ones in range (negative number)
    i++;
    sign = 1;
    exponent = -1;
    significand = 0b1111111111;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b1111111111111111;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 4 test a too large sign
    i++;
    sign = 0b11111111;
    exponent = 1;
    significand = 0b1000000000;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = tfl16_one_neg;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 5 test a too large significand
    i++;
    sign = 0;
    exponent = 2;
    significand = 0b111000000000;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0001001110000000; // 14
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 6 test too large significand and negative exponent
    i++;
    sign = 0;
    exponent = -10;
    significand = 0b111000000000;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0110001110000000; 
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 7 test too large significand, where negative exponent becomes positive
    i++;
    sign = 0;
    exponent = -1;
    significand = 0b111000000000;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0000011110000000; 
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 8 test too large significand, where negative exponent becomes 0
    i++;
    sign = 0;
    exponent = -2;
    significand = 0b111000000000;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0000001110000000; 
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 9 test too large significand, where insignificand bits are removed
    i++;
    sign = 0;
    exponent = 2;
    significand = 0b111000000111;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0001001110000001; 
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 10 test too large significand, where exponent is max
    i++;
    sign = 0;
    exponent = 15;
    significand = 0b10000000000;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0011111111111111; 
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 11 test when exponent is too large (not possible to increase significand)
    i++;
    sign = 0;
    exponent = 16;
    significand = 0b1000000000;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0011111111111111;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 12 test when exponent is too large -> increase significand
    i++;
    sign = 0;
    exponent = 17;
    significand = 0b0010000000;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0011111000000000;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 13 same as 12 but larger exponent and 1 significand
    i++;
    sign = 0;
    exponent = 15+9;
    significand = 1;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0011111000000000;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 14 same as 12 but too high exponent and 1 significand
    i++;
    sign = 0;
    exponent = 15+10;
    significand = 1;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0011111111111111;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 15 test too large significand, and too large exponent
    i++;
    sign = 0;
    exponent = 16;
    significand = 0b10000000000;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0011111111111111; 
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 16 test 0 significand
    i++;
    sign = 1;
    exponent = 7;
    significand = 0;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = tfl16_zero_neg; 
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 17 test lowest exponent (-16)
    i++;
    sign = 0;
    exponent = -16;
    significand = 0b1000000000;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0100001000000000;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 18 test too low exponent -> change significand
    i++;
    sign = 0;
    exponent = -17;
    significand = 0b1000000011;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0100000100000001;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 19 test too large significand, which changes too low exponent to now be in range
    i++;
    sign = 0;
    exponent = -17;
    significand = 0b10000000000;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0100001000000000;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 20 test too large significand and too low exponent -> in range
    i++;
    sign = 0;
    exponent = -18;
    significand = 0b10000000000;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0100000100000000;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 21 test too low exponent, where significand is too low to shift
    i++;
    sign = 0;
    exponent = -18;
    significand = 0b10;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 22 same as 21 but with negative sign
    i++;
    sign = 1;
    exponent = -19;
    significand = 0b10;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = tfl16_zero_neg;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 23 same as 18 - test maximum number of shifts (9) for in-range significand
    i++;
    sign = 0;
    exponent = -25;
    significand = 0b1000000000;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0100000000000001;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 24 largest significand with exponent in range for shift
    i++;
    sign = 0;
    exponent = 0;
    significand = 0b1111111111111111;
    x1 = tfl_normalize(sign,exponent,significand);
    x2 = 0b0001101111111111;
    if (x1 == x2){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    printf("\n   %d/%d\n", j,i);
}

void test_tfl_add(){
    printf("Testing tfl_add:");

    // 1 test two positive numbers with same exponent
    int i = 1;
    int j = 0;
    tfl16_t x1 = 0b0011001000000100;
    tfl16_t x2 = 0b0011000100000000;
    tfl16_t x3 = 0b0011001100000100;
    if (tfl_equals(tfl_add(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 2 test two positive numbers, with different exponent
    i++;
    x1 = tfl_normalize(0,1,512);
    x2 = tfl_normalize(0,2,128);
    x3 = tfl_normalize(0,1,512+256);
    if (tfl_equals(tfl_add(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 3 test two positive numbers, where significand becomes too large
    i++;
    x1 = tfl_normalize(0,2,1000);
    x2 = tfl_normalize(0,4,71);
    x3 = tfl_normalize(0,2,1284);
    if (tfl_equals(tfl_add(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 4 test very large number cancels out very small number
    i++;
    x1 = tfl_normalize(1,0xF8,1234); //very small number
    x2 = tfl_normalize(0,0x0F,1234); //very large number
    x3 = tfl_normalize(0,0x0F,1234); //small number should get cancelled out
    if (tfl_equals(tfl_add(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 5 test 2 large numbers
    i++;
    x1 = tfl_normalize(0,15,1023);
    x2 = tfl_normalize(0,15,1023);
    x3 = tfl_normalize(0,15,1023);
    if (tfl_equals(tfl_add(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 6 test numbers with different sign, resulting in positive number
    i++;
    x1 = tfl_normalize(0,3,1023);
    x2 = tfl_normalize(1,3,23);
    x3 = tfl_normalize(0,3,1000);
    if (tfl_equals(tfl_add(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 7 same as 6 but reversed
    i++;
    x1 = tfl_normalize(1,3,23);
    x2 = tfl_normalize(0,3,1023);
    x3 = tfl_normalize(0,3,1000);
    if (tfl_equals(tfl_add(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 8 test numbers with different sign, resulting in negative number
    i++;
    x1 = tfl_normalize(1,3,1023);
    x2 = tfl_normalize(0,3,1022);
    x3 = tfl_normalize(1,3,1);
    if (tfl_equals(tfl_add(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 9 same as 8 but reversed
    i++;
    x1 = tfl_normalize(0,3,1022);
    x2 = tfl_normalize(1,3,1023);
    x3 = tfl_normalize(1,3,1);
    if (tfl_equals(tfl_add(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    printf("\n   %d/%d\n", j,i);
}

void test_tfl_mul(){
    printf("Testing tfl_mul:");

    // 1 test two positive numbers with same exponent
    int i = 1;
    int j = 0;
    tfl16_t x1 = tfl_normalize(0,2,131);
    tfl16_t x2 = tfl_normalize(0,2,3);
    tfl16_t x3 = tfl_normalize(0,4-10,393);
    if (tfl_equals(tfl_mul(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 2 test one negative and one positive number
    i++;
    x1 = tfl_normalize(1,2,131);
    x2 = tfl_normalize(0,2,3);
    x3 = tfl_normalize(1,4-10,393);
    if (tfl_equals(tfl_mul(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }
    
    // 3 test two large numbers, OVERFLOW
    i++;
    x1 = tfl_normalize(1,14,1109);
    x2 = tfl_normalize(0,13,1200);
    x3 = tfl_normalize(1,15,1023);
    if (tfl_equals(tfl_mul(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 4 test two small numbers, UNDERFLOW
    i++;
    x1 = tfl_normalize(1,-15,131);
    x2 = tfl_normalize(0,-10,3);
    x3 = 0;
    if (tfl_equals(tfl_mul(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 5 multiply with zero
    i++;
    x1 = 0;
    x2 = tfl_normalize(1,5,1023);
    x3 = 0;
    if (tfl_equals(tfl_mul(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 6 try max uint16 significand
    i++;
    x1 = tfl_normalize(1,1,13107);
    x2 = tfl_normalize(1,1,5);
    x3 = tfl_normalize(0,2-10,65535);
    if (tfl_equals(tfl_mul(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    // 7 try where result of significand is larger than max uint16, but it is possible to shift
    i++;
    x1 = tfl_normalize(0,1,13107);
    x2 = tfl_normalize(1,1,6);
    x3 = tfl_normalize(1,2-10+1,39321);
    if (tfl_equals(tfl_mul(x1,x2), x3)){ printf("  %d [SUCCESS]",i); j++; }
    else { printf("  %d [FAIL]",i); }

    printf("\n   %d/%d\n", j,i);
}




