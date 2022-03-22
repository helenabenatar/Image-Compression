/*
   codewords.c
   Written by Ronit Sinha (rsinha01) and Helena Benatar (hbenat01)
   Date: 27 October 2021
   Purpose: Functions for packing a discrete cosine transformed image into a
       list 32-bit codewords (one per block), and for unpacking a list of 
       codewords into a discrete cosine transformed image.
*/
#include "codewords.h"
#include <stdio.h>

/* Maximum value of b,c,d from DCT */
#define MAX_BCD (0.3)

/* Used by mapping functions fill_codeword_list and fill_dct */
struct Closure {
    Seq_T list;
    PackingScheme_T pc;
};

/* double_to_int
    Purpose: convert a double to a signed integer, given the double's range.

    Parameters:
        double value - value to convert to int
        unsigned width - how many bits the value should be stored in
        double maxval - max of value's range
        doubel minval - min of value's range

    Returns: int64_t - converted value 
*/
int64_t double_to_int (double value, unsigned width,
    double maxval)
{
    double output = value;

    uint64_t newmax = ((1 << (width - 1)) - 1) /  maxval;

    return (int64_t) (output * newmax);
}

/* double_to_uint
    Purpose: convert a double to a unsigned integer.

    Parameters:
        double value - value to convert to int
        unsigned width - how many bits the value should be stored in

    Returns: uint64_t - converted value 
*/
uint64_t double_to_uint (double value, unsigned width)
{
    uint64_t capacity = (1 << width) - 1;
    return (uint64_t) round(value * capacity);
}

/* uint_to_double
    Purpose: convert an unsigned int into a double ranging from 0 to 1.

    Parameters:
        uint64_t value - value to convert
        unsigned width - how many bits long is value

    Returns: double - converted value
*/
double uint_to_double (uint64_t value, unsigned width)
{
    uint64_t capacity = (1 << width) - 1;
    return (double) value / capacity;    
}

/* int_to double
    Purpose: convert an signed int into a double in a given range.

    Parameters:
        int64_t value - value to convert
        unsigned width - how many bits long is value
        double maxval - range of value will be [-maxval, maxval]

    Returns: double - converted value
*/
double int_to_double (int64_t value, unsigned width, double maxval)
{
    uint64_t capacity = 1 << (width - 1);
    double newmax = (double) capacity / maxval;
    
    return (double) value / newmax;
}

/* pack_codeword
    Purpose: pack a DCT_Block into a 32-bit codeword according to a given
        packing scheme.
    
    Parameters:
        DCT_Block block - discrete cosine transform block to pack
        PackingScheme_T pc - how values of block should be stored

    Returns: uint64_t - packed codeword
*/
uint64_t pack_codeword (DCT_Block block, PackingScheme_T pc)
{
    uint64_t data = 0;

    uint64_t a = double_to_uint(block.a, pc.a_width);

    int64_t b = double_to_int(block.b, pc.b_width, MAX_BCD);
    int64_t c = double_to_int(block.c, pc.c_width, MAX_BCD);
    int64_t d = double_to_int(block.d, pc.d_width, MAX_BCD);

    data = Bitpack_newu(data, pc.a_width, pc.a_lsb, a);

    data = Bitpack_news(data, pc.b_width, pc.b_lsb, b);
    data = Bitpack_news(data, pc.c_width, pc.c_lsb, c);
    data = Bitpack_news(data, pc.d_width, pc.d_lsb, d);

    data = Bitpack_newu(data, pc.pb_width, pc.pb_lsb, block.pb_index);
    data = Bitpack_newu(data, pc.pr_width, pc.pr_lsb, block.pr_index);

    return data;
}

/* unpack_codeword
    Purpose: unpack a 32-bit codeword into a DCT_Block according to a given
        packing scheme.
    
    Parameters:
        uint64_t codeword - codeword to unpack
        PackingScheme_T pc - how the values are stored in the codeword 

    Returns: DCT_Block - unpacked discrete cosine transform
*/
DCT_Block unpack_codeword (uint64_t codeword, PackingScheme_T pc)
{
    uint64_t a_int = Bitpack_getu(codeword, pc.a_width, pc.a_lsb);

    int64_t b_int = Bitpack_gets(codeword, pc.b_width, pc.b_lsb);
    int64_t c_int = Bitpack_gets(codeword, pc.c_width, pc.c_lsb);
    int64_t d_int = Bitpack_gets(codeword, pc.d_width, pc.d_lsb);

    uint64_t pb_index = Bitpack_getu(codeword, pc.pb_width, pc.pb_lsb);
    uint64_t pr_index = Bitpack_getu(codeword, pc.pr_width, pc.pr_lsb);

    double a = uint_to_double (a_int, pc.a_width);
    double b = int_to_double (b_int, pc.b_width, MAX_BCD);
    double c = int_to_double (c_int, pc.c_width, MAX_BCD);
    double d = int_to_double (d_int, pc.d_width, MAX_BCD);


    DCT_Block block = {
        .a = a, .b = b, .c = c, .d = d,
        .pb_index = (unsigned) pb_index, .pr_index = (unsigned) pr_index
    };

    return block;
}

/* fill_codeword_list
    Purpose: Fill a Hanson sequence with codewords created from DCT_Blocks.
        Apply function called by small_map_default in generate_codewords.

    Parameters: see A2Methods_smallapplyfun for more info

    Errors: Throws an error if it cannot allocate memory
*/
void fill_codeword_list (void *elem, void *cl)
{
    struct Closure *clo = (struct Closure *) cl;

    Seq_T list = (Seq_T) clo->list;

    DCT_Block block = *(DCT_Block *) elem;

    uint64_t *codeword = malloc(sizeof(*codeword));
    assert(codeword != NULL);

    *codeword = pack_codeword(block, clo->pc);

    Seq_addhi(list, codeword);
}

/* generate_codewords
    Purpose: Given a 2D array of DCT_Blocks, pack each block into a codeword
        and return a list of these codewords.

    Parameters:
        A2Methods_UArray2 array2 - 2D array of DCT_Blocks
        A2Methods_T methods - methods that can operate on array2
        PackingScheme_T pc - how values of each DCT_Block should be packed into
            a 32-bit word

    Returns: Seq_T - list of codewords
*/
Seq_T generate_codewords (A2Methods_UArray2 array2, A2Methods_T methods,
    PackingScheme_T pc)
{
    Seq_T codewords = Seq_new(0);

    struct Closure cl = {.list = codewords, .pc = pc};

    methods->small_map_default(array2, fill_codeword_list, &cl);

    return codewords;
}

/* fill_dct
    Purpose: unpack a codeword and set element in a 2D array to the unpacked
        DCT_Block. Called by small_map_default in generate_dct.

    Parameters: See A2Methods_smallapplyfun for more info.
*/
void fill_dct (void *elem, void *cl)
{
    struct Closure *clo = (struct Closure *) cl;    

    Seq_T list = (Seq_T) clo->list;

    uint64_t *codeword = (uint64_t *) Seq_remlo(list);

    DCT_Block block = unpack_codeword(*codeword, clo->pc);

    /* Since we've removed it, we'll free it here too */
    free(codeword);

    *(DCT_Block *) elem = block;
}

/* generate_dct
    Purpose: Given a sequence of codewords, unpack each one into a DCT_Block
        and return a 2D array of these blocks.

    Parameters:
        Seq_T codewords - list of codewords
        A2Methods_T methods - methods to create and operate on a 2D array
        PackingScheme_T pc - how values are stored in each codeword

    Returns: A2Methods_UArray2 - 2D discrete cosine transform array
*/
A2Methods_UArray2 generate_dct (Seq_T codewords, A2Methods_T methods,
    unsigned width, unsigned height, PackingScheme_T pc)
{
    A2Methods_UArray2 dct = methods->new(width, height, 
        sizeof(struct DCT_Block));

    struct Closure cl = {.list = codewords, .pc = pc};

    methods->small_map_default(dct, fill_dct, &cl);

    return dct;
}

/* free_codeword_seq
    Purpose: free each element in a codeword sequence, as well as the sequence
        itself.

    Parameters: Seq_T *list - pointer to Hanson sequence of codewords
*/
void free_codeword_seq (Seq_T *list)
{
    int i;
    for (i = 0; i < Seq_length(*list); i++) {
        free(Seq_get(*list, i));
    }

    Seq_free(list);
}