/*
   bitpack.c
   Written by Ronit Sinha (rsinha01) and Helena Benatar (hbenat01)
   Date: 27 October 2021
   Purpose: Implementation of bitpack, a series of functions used for packing
    and fetching data in signed and unsigned 64-bit integers.
*/
#include "bitpack.h"

#include "assert.h"

#define WORD_WIDTH 64

Except_T Bitpack_Overflow = { "Overflow packing bits" };

/* shift_left
    Purpose: shifts a unsigned int left by a specified amount of bits. Since
        shifting by 64 bits is not well defined, we decided that doing so will
        result in 0.

    Parameters:
        uint64_t n - number to be shifted
        unsigned shift - number to shift ints by

    Returns: uint64_t, newly-shifted value
*/
uint64_t shift_left (uint64_t n, unsigned shift) {
    if (shift == WORD_WIDTH) {
        return 0;
    }

    return n << shift;
}

/* ushift_right
    Purpose: shifts a unsigned int right by a specified amount of bits. Since
        shifting by 64 bits is not well defined, we decided that doing so will
        result in 0.

    Parameters:
        uint64_t n - number to be shifted
        unsigned shift - number to shift ints by

    Returns: uint64_t, newly-shifted value
*/
uint64_t ushift_right (uint64_t n, unsigned shift) {
    if (shift == WORD_WIDTH) {
        return 0;
    }

    return n >> shift;
}

/* sshift_right
    Purpose: shifts a signed int right by a specified amount of bits, making
        sure that the correct bit propagates. Since shifting by 64 bits is not
        well defined, we decided that doing so will result in 0.

    Parameters:
        int64_t n - number to be shifted
        unsigned width - number of bits that n takes up
        unsigned shift - number to shift ints by

    Returns: int64_t, newly-shifted value
*/
int64_t sshift_right (int64_t n, unsigned width, unsigned shift) {
    if (shift == WORD_WIDTH) {
        return 0;
    }

    /* we shift back and forth by WORD_WIDTH - width to make sure that
        the correct bit propagates in the front. When we do this with a 
        negative number this will frontfill the word with 1's */
    return n >> shift << (WORD_WIDTH - width) >> (WORD_WIDTH - width);
}

/* Bitpack_fitsu
    Purpose: Determine if a given unsigned int can fit in a given number of
        bits. We define a width of 0 bits to only fit the number 0.

    Parameters:
        uint64_t n - unsigned int to test to be fit
        unsigned width - number of bits that n must fit in

    Returns: bool - whether or not n fits in width bits.
*/
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
    /* uint64_t can store a maximum of 64-bit integers, so if the provided
        width is >= 64, then n is guaranteed to fit */
    if (width >= WORD_WIDTH) {
        return true;
    }

    /* We define 0 bits to only fit the number 0. */
    if (width == 0) {
        return n == 0;
    } 

    /* same as 2^width - 1 */
    uint64_t capacity = shift_left(1, width) - 1; 

    /* n is always >= 0 since it is unsigned, so we just check upper bound */
    return n <= capacity;
}

/* Bitpack_fitss
    Purpose: Determine if a given signed int can fit in a given number of
        bits. We define a width of 0 bits to only fit the number 0.

    Parameters:
        int64_t n - signed int to test to be fit
        unsigned width - number of bits that n must fit in

    Returns: bool - whether or not n fits in width bits.
*/
bool Bitpack_fitss(int64_t n, unsigned width)
{
    /* uint64_t can store a maximum of 64-bit integers, so if the provided
        width is >= 64, then n is guaranteed to fit */
    if (width >= WORD_WIDTH) {
        return true;
    }

    /* We define 0 bits to only fit the number 0. */
    if (width == 0) {
        return n == 0;
    } 

    int64_t max_positive = (int64_t) shift_left(1, width - 1) - 1;
    int64_t max_negative = (int64_t) -shift_left(1, width - 1);

    return n >= max_negative && n <= max_positive;
}

/* Bitpack_getu
    Purpose: get an unsigned value of some length of bits from a 64-bit
        unsigned int

    Parameters:
        uint64_t word - the unsigned int to get from
        unsigned width - number of bits of desired data
        unsigned lsb - the least signficant bit of word where data is stored

    Returns: uint64_t - width-bits unsigned data starting at lsb.

    Errors: throws an error when the given width is larger than 64 or width +
        least significant bit is greater than 64.
*/
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
    assert(width <= WORD_WIDTH);
    assert(width + lsb <= WORD_WIDTH);

    if (width == 0) {
        return 0;
    }

    uint64_t mask = shift_left(ushift_right(~0, WORD_WIDTH - width), lsb);

    uint64_t value = ushift_right(word & mask, lsb);

    return value;
}

/* Bitpack_gets
    Purpose: get a signed value of some length of bits from a 64-bit
        signed int

    Parameters:
        int64_t word - the signed int to get from
        unsigned width - number of bits of desired data
        unsigned lsb - the least signficant bit of word where data is stored

    Returns: int64_t - width-bits signed data starting at lsb.
*/
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
    /* We shift right by 0 just to make sure the correct bit propagates in
        the front (see sshift_right for more info) */
    int64_t value = sshift_right(Bitpack_getu(word, width, lsb), width, 0);

    return value;
}

/* update_bitpack
    Purpose: update value in uint64_t in width bits, starting at lsb. Helper
        function used in Bitpack_newu and Bitpack_news.

    Parameters:
        uint64_t word - word to store data in
        unsigned width - number of bits new data takes up
        usigned lsb - least signficant bit in word where data will be stored
        uint64_t value - data to be stored in word

    Returns: uint64_t - word with value stored in it
*/
uint64_t update_bitpack (uint64_t word, unsigned width, unsigned lsb, 
    uint64_t value)
{
    uint64_t mask = shift_left(ushift_right(~0, WORD_WIDTH - width), lsb);
    uint64_t cleared = word & ~mask;

    uint64_t updated = cleared | shift_left(value, lsb);

    return updated;
}

/* Bitpack_newu
    Purpose: add unsigned value of some number of bits into uint64_t, starting
        at a least significant bit.

    Parameters:
        uint64_t word - word to store data in
        unsigned width - number of bits new data takes up
        usigned lsb - least signficant bit in word where data will be stored
        uint64_t value - data to be stored in word

    Errors: throws an error when the given width is larger than 64 or width +
        least significant bit is greater than 64. Also raises Bitpack_Overflow
        when given value does not fit in the given width.
*/
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
    uint64_t value)
{
    assert(width <= WORD_WIDTH);
    assert(width + lsb <= WORD_WIDTH);

    if (!Bitpack_fitsu(value, width)) {
        RAISE(Bitpack_Overflow);
    }

    return update_bitpack(word, width, lsb, value);
}

/* Bitpack_news
    Purpose: add signed value of some number of bits into uint64_t, starting
        at a least significant bit.

    Parameters:
        uint64_t word - word to store data in
        unsigned width - number of bits new data takes up
        usigned lsb - least signficant bit in word where data will be stored
        int64_t value - data to be stored in word

    Errors: throws an error when the given width is larger than 64 or width +
        least significant bit is greater than 64. Also raises Bitpack_Overflow
        when given value does not fit in the given width.
*/
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,
    int64_t value)
{
    assert(width <= WORD_WIDTH);
    assert(width + lsb <= WORD_WIDTH);

    /* If value is negative, then it has a bunch of leading 1's, so to fix it
        we shift it back and forth as an unsigned. This prevents the leading
        1's from overriding bits in word that are outside of value's width */
    uint64_t trimmed_value = ushift_right(
        shift_left(value, WORD_WIDTH - width), 
        WORD_WIDTH - width);

    if (!Bitpack_fitss(value, width)) {
        RAISE(Bitpack_Overflow);
    }

    return update_bitpack(word, width, lsb, (uint64_t) trimmed_value);
}