#ifndef __BOARD_H__
#define __BOARD_H__

#include "common.h"
using namespace std;

#define KOGGE_STONE true

/**
 * @brief For converting a move number 0-63 to a bitmask.
*/
const bitbrd MOVEMASK[64] = {
0x0000000000000001, 0x0000000000000002, 0x0000000000000004, 0x0000000000000008,
0x0000000000000010, 0x0000000000000020, 0x0000000000000040, 0x0000000000000080,
0x0000000000000100, 0x0000000000000200, 0x0000000000000400, 0x0000000000000800,
0x0000000000001000, 0x0000000000002000, 0x0000000000004000, 0x0000000000008000,
0x0000000000010000, 0x0000000000020000, 0x0000000000040000, 0x0000000000080000,
0x0000000000100000, 0x0000000000200000, 0x0000000000400000, 0x0000000000800000,
0x0000000001000000, 0x0000000002000000, 0x0000000004000000, 0x0000000008000000,
0x0000000010000000, 0x0000000020000000, 0x0000000040000000, 0x0000000080000000,
0x0000000100000000, 0x0000000200000000, 0x0000000400000000, 0x0000000800000000,
0x0000001000000000, 0x0000002000000000, 0x0000004000000000, 0x0000008000000000,
0x0000010000000000, 0x0000020000000000, 0x0000040000000000, 0x0000080000000000,
0x0000100000000000, 0x0000200000000000, 0x0000400000000000, 0x0000800000000000,
0x0001000000000000, 0x0002000000000000, 0x0004000000000000, 0x0008000000000000,
0x0010000000000000, 0x0020000000000000, 0x0040000000000000, 0x0080000000000000,
0x0100000000000000, 0x0200000000000000, 0x0400000000000000, 0x0800000000000000,
0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000,
};

const int index64[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

const bitbrd NORTHRAY[64] = {
0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 
0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 
0x0000000000000001, 0x0000000000000002, 0x0000000000000004, 0x0000000000000008, 
0x0000000000000010, 0x0000000000000020, 0x0000000000000040, 0x0000000000000080, 
0x0000000000000101, 0x0000000000000202, 0x0000000000000404, 0x0000000000000808, 
0x0000000000001010, 0x0000000000002020, 0x0000000000004040, 0x0000000000008080, 
0x0000000000010101, 0x0000000000020202, 0x0000000000040404, 0x0000000000080808, 
0x0000000000101010, 0x0000000000202020, 0x0000000000404040, 0x0000000000808080, 
0x0000000001010101, 0x0000000002020202, 0x0000000004040404, 0x0000000008080808, 
0x0000000010101010, 0x0000000020202020, 0x0000000040404040, 0x0000000080808080, 
0x0000000101010101, 0x0000000202020202, 0x0000000404040404, 0x0000000808080808, 
0x0000001010101010, 0x0000002020202020, 0x0000004040404040, 0x0000008080808080, 
0x0000010101010101, 0x0000020202020202, 0x0000040404040404, 0x0000080808080808, 
0x0000101010101010, 0x0000202020202020, 0x0000404040404040, 0x0000808080808080, 
0x0001010101010101, 0x0002020202020202, 0x0004040404040404, 0x0008080808080808, 
0x0010101010101010, 0x0020202020202020, 0x0040404040404040, 0x0080808080808080
};

const bitbrd SOUTHRAY[64] = {
0x0101010101010100, 0x0202020202020200, 0x0404040404040400, 0x0808080808080800, 
0x1010101010101000, 0x2020202020202000, 0x4040404040404000, 0x8080808080808000, 
0x0101010101010000, 0x0202020202020000, 0x0404040404040000, 0x0808080808080000, 
0x1010101010100000, 0x2020202020200000, 0x4040404040400000, 0x8080808080800000, 
0x0101010101000000, 0x0202020202000000, 0x0404040404000000, 0x0808080808000000, 
0x1010101010000000, 0x2020202020000000, 0x4040404040000000, 0x8080808080000000, 
0x0101010100000000, 0x0202020200000000, 0x0404040400000000, 0x0808080800000000, 
0x1010101000000000, 0x2020202000000000, 0x4040404000000000, 0x8080808000000000, 
0x0101010000000000, 0x0202020000000000, 0x0404040000000000, 0x0808080000000000, 
0x1010100000000000, 0x2020200000000000, 0x4040400000000000, 0x8080800000000000, 
0x0101000000000000, 0x0202000000000000, 0x0404000000000000, 0x0808000000000000, 
0x1010000000000000, 0x2020000000000000, 0x4040000000000000, 0x8080000000000000, 
0x0100000000000000, 0x0200000000000000, 0x0400000000000000, 0x0800000000000000, 
0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000, 
0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 
0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000
};

const bitbrd EASTRAY[64] = {
0x00000000000000fe, 0x00000000000000fc, 0x00000000000000f8, 0x00000000000000f0, 
0x00000000000000e0, 0x00000000000000c0, 0x0000000000000080, 0x0000000000000000, 
0x000000000000fe00, 0x000000000000fc00, 0x000000000000f800, 0x000000000000f000, 
0x000000000000e000, 0x000000000000c000, 0x0000000000008000, 0x0000000000000000, 
0x0000000000fe0000, 0x0000000000fc0000, 0x0000000000f80000, 0x0000000000f00000, 
0x0000000000e00000, 0x0000000000c00000, 0x0000000000800000, 0x0000000000000000, 
0x00000000fe000000, 0x00000000fc000000, 0x00000000f8000000, 0x00000000f0000000, 
0x00000000e0000000, 0x00000000c0000000, 0x0000000080000000, 0x0000000000000000, 
0x000000fe00000000, 0x000000fc00000000, 0x000000f800000000, 0x000000f000000000, 
0x000000e000000000, 0x000000c000000000, 0x0000008000000000, 0x0000000000000000, 
0x0000fe0000000000, 0x0000fc0000000000, 0x0000f80000000000, 0x0000f00000000000, 
0x0000e00000000000, 0x0000c00000000000, 0x0000800000000000, 0x0000000000000000, 
0x00fe000000000000, 0x00fc000000000000, 0x00f8000000000000, 0x00f0000000000000, 
0x00e0000000000000, 0x00c0000000000000, 0x0080000000000000, 0x0000000000000000, 
0xfe00000000000000, 0xfc00000000000000, 0xf800000000000000, 0xf000000000000000, 
0xe000000000000000, 0xc000000000000000, 0x8000000000000000, 0x0000000000000000
};

const bitbrd SERAY[64] = {
0x8040201008040200, 0x0080402010080400, 0x0000804020100800, 0x0000008040201000, 
0x0000000080402000, 0x0000000000804000, 0x0000000000008000, 0x0000000000000000, 
0x4020100804020000, 0x8040201008040000, 0x0080402010080000, 0x0000804020100000, 
0x0000008040200000, 0x0000000080400000, 0x0000000000800000, 0x0000000000000000, 
0x2010080402000000, 0x4020100804000000, 0x8040201008000000, 0x0080402010000000, 
0x0000804020000000, 0x0000008040000000, 0x0000000080000000, 0x0000000000000000, 
0x1008040200000000, 0x2010080400000000, 0x4020100800000000, 0x8040201000000000, 
0x0080402000000000, 0x0000804000000000, 0x0000008000000000, 0x0000000000000000, 
0x0804020000000000, 0x1008040000000000, 0x2010080000000000, 0x4020100000000000, 
0x8040200000000000, 0x0080400000000000, 0x0000800000000000, 0x0000000000000000, 
0x0402000000000000, 0x0804000000000000, 0x1008000000000000, 0x2010000000000000, 
0x4020000000000000, 0x8040000000000000, 0x0080000000000000, 0x0000000000000000, 
0x0200000000000000, 0x0400000000000000, 0x0800000000000000, 0x1000000000000000, 
0x2000000000000000, 0x4000000000000000, 0x8000000000000000, 0x0000000000000000, 
0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 
0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000
};

const bitbrd NERAY[64] = {
0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 
0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 
0x0000000000000002, 0x0000000000000004, 0x0000000000000008, 0x0000000000000010, 
0x0000000000000020, 0x0000000000000040, 0x0000000000000080, 0x0000000000000000, 
0x0000000000000204, 0x0000000000000408, 0x0000000000000810, 0x0000000000001020, 
0x0000000000002040, 0x0000000000004080, 0x0000000000008000, 0x0000000000000000, 
0x0000000000020408, 0x0000000000040810, 0x0000000000081020, 0x0000000000102040, 
0x0000000000204080, 0x0000000000408000, 0x0000000000800000, 0x0000000000000000, 
0x0000000002040810, 0x0000000004081020, 0x0000000008102040, 0x0000000010204080, 
0x0000000020408000, 0x0000000040800000, 0x0000000080000000, 0x0000000000000000, 
0x0000000204081020, 0x0000000408102040, 0x0000000810204080, 0x0000001020408000, 
0x0000002040800000, 0x0000004080000000, 0x0000008000000000, 0x0000000000000000, 
0x0000020408102040, 0x0000040810204080, 0x0000081020408000, 0x0000102040800000, 
0x0000204080000000, 0x0000408000000000, 0x0000800000000000, 0x0000000000000000, 
0x0002040810204080, 0x0004081020408000, 0x0008102040800000, 0x0010204080000000, 
0x0020408000000000, 0x0040800000000000, 0x0080000000000000, 0x0000000000000000
};

const bitbrd WESTRAY[64] = {
0x0000000000000000, 0x0000000000000001, 0x0000000000000003, 0x0000000000000007, 
0x000000000000000f, 0x000000000000001f, 0x000000000000003f, 0x000000000000007f, 
0x0000000000000000, 0x0000000000000100, 0x0000000000000300, 0x0000000000000700, 
0x0000000000000f00, 0x0000000000001f00, 0x0000000000003f00, 0x0000000000007f00, 
0x0000000000000000, 0x0000000000010000, 0x0000000000030000, 0x0000000000070000, 
0x00000000000f0000, 0x00000000001f0000, 0x00000000003f0000, 0x00000000007f0000, 
0x0000000000000000, 0x0000000001000000, 0x0000000003000000, 0x0000000007000000, 
0x000000000f000000, 0x000000001f000000, 0x000000003f000000, 0x000000007f000000, 
0x0000000000000000, 0x0000000100000000, 0x0000000300000000, 0x0000000700000000, 
0x0000000f00000000, 0x0000001f00000000, 0x0000003f00000000, 0x0000007f00000000, 
0x0000000000000000, 0x0000010000000000, 0x0000030000000000, 0x0000070000000000, 
0x00000f0000000000, 0x00001f0000000000, 0x00003f0000000000, 0x00007f0000000000, 
0x0000000000000000, 0x0001000000000000, 0x0003000000000000, 0x0007000000000000, 
0x000f000000000000, 0x001f000000000000, 0x003f000000000000, 0x007f000000000000, 
0x0000000000000000, 0x0100000000000000, 0x0300000000000000, 0x0700000000000000, 
0x0f00000000000000, 0x1f00000000000000, 0x3f00000000000000, 0x7f00000000000000
};

const bitbrd NWRAY[64] = {
0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 
0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 
0x0000000000000000, 0x0000000000000001, 0x0000000000000002, 0x0000000000000004, 
0x0000000000000008, 0x0000000000000010, 0x0000000000000020, 0x0000000000000040, 
0x0000000000000000, 0x0000000000000100, 0x0000000000000201, 0x0000000000000402, 
0x0000000000000804, 0x0000000000001008, 0x0000000000002010, 0x0000000000004020, 
0x0000000000000000, 0x0000000000010000, 0x0000000000020100, 0x0000000000040201, 
0x0000000000080402, 0x0000000000100804, 0x0000000000201008, 0x0000000000402010, 
0x0000000000000000, 0x0000000001000000, 0x0000000002010000, 0x0000000004020100, 
0x0000000008040201, 0x0000000010080402, 0x0000000020100804, 0x0000000040201008, 
0x0000000000000000, 0x0000000100000000, 0x0000000201000000, 0x0000000402010000, 
0x0000000804020100, 0x0000001008040201, 0x0000002010080402, 0x0000004020100804, 
0x0000000000000000, 0x0000010000000000, 0x0000020100000000, 0x0000040201000000, 
0x0000080402010000, 0x0000100804020100, 0x0000201008040201, 0x0000402010080402, 
0x0000000000000000, 0x0001000000000000, 0x0002010000000000, 0x0004020100000000, 
0x0008040201000000, 0x0010080402010000, 0x0020100804020100, 0x0040201008040201
};

const bitbrd SWRAY[64] = {
0x0000000000000000, 0x0000000000000100, 0x0000000000010200, 0x0000000001020400, 
0x0000000102040800, 0x0000010204081000, 0x0001020408102000, 0x0102040810204000, 
0x0000000000000000, 0x0000000000010000, 0x0000000001020000, 0x0000000102040000, 
0x0000010204080000, 0x0001020408100000, 0x0102040810200000, 0x0204081020400000, 
0x0000000000000000, 0x0000000001000000, 0x0000000102000000, 0x0000010204000000, 
0x0001020408000000, 0x0102040810000000, 0x0204081020000000, 0x0408102040000000, 
0x0000000000000000, 0x0000000100000000, 0x0000010200000000, 0x0001020400000000, 
0x0102040800000000, 0x0204081000000000, 0x0408102000000000, 0x0810204000000000, 
0x0000000000000000, 0x0000010000000000, 0x0001020000000000, 0x0102040000000000, 
0x0204080000000000, 0x0408100000000000, 0x0810200000000000, 0x1020400000000000, 
0x0000000000000000, 0x0001000000000000, 0x0102000000000000, 0x0204000000000000, 
0x0408000000000000, 0x0810000000000000, 0x1020000000000000, 0x2040000000000000, 
0x0000000000000000, 0x0100000000000000, 0x0200000000000000, 0x0400000000000000, 
0x0800000000000000, 0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 
0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 
0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000
};

const bitbrd NORTHRAYI[64] = {
0x0000000000000001, 0x0000000000000002, 0x0000000000000004, 0x0000000000000008, 
0x0000000000000010, 0x0000000000000020, 0x0000000000000040, 0x0000000000000080, 
0x0000000000000101, 0x0000000000000202, 0x0000000000000404, 0x0000000000000808, 
0x0000000000001010, 0x0000000000002020, 0x0000000000004040, 0x0000000000008080, 
0x0000000000010101, 0x0000000000020202, 0x0000000000040404, 0x0000000000080808, 
0x0000000000101010, 0x0000000000202020, 0x0000000000404040, 0x0000000000808080, 
0x0000000001010101, 0x0000000002020202, 0x0000000004040404, 0x0000000008080808, 
0x0000000010101010, 0x0000000020202020, 0x0000000040404040, 0x0000000080808080, 
0x0000000101010101, 0x0000000202020202, 0x0000000404040404, 0x0000000808080808, 
0x0000001010101010, 0x0000002020202020, 0x0000004040404040, 0x0000008080808080, 
0x0000010101010101, 0x0000020202020202, 0x0000040404040404, 0x0000080808080808, 
0x0000101010101010, 0x0000202020202020, 0x0000404040404040, 0x0000808080808080, 
0x0001010101010101, 0x0002020202020202, 0x0004040404040404, 0x0008080808080808, 
0x0010101010101010, 0x0020202020202020, 0x0040404040404040, 0x0080808080808080, 
0x0101010101010101, 0x0202020202020202, 0x0404040404040404, 0x0808080808080808, 
0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080
};

const bitbrd SOUTHRAYI[64] = {
0x0101010101010101, 0x0202020202020202, 0x0404040404040404, 0x0808080808080808, 
0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080, 
0x0101010101010100, 0x0202020202020200, 0x0404040404040400, 0x0808080808080800, 
0x1010101010101000, 0x2020202020202000, 0x4040404040404000, 0x8080808080808000, 
0x0101010101010000, 0x0202020202020000, 0x0404040404040000, 0x0808080808080000, 
0x1010101010100000, 0x2020202020200000, 0x4040404040400000, 0x8080808080800000, 
0x0101010101000000, 0x0202020202000000, 0x0404040404000000, 0x0808080808000000, 
0x1010101010000000, 0x2020202020000000, 0x4040404040000000, 0x8080808080000000, 
0x0101010100000000, 0x0202020200000000, 0x0404040400000000, 0x0808080800000000, 
0x1010101000000000, 0x2020202000000000, 0x4040404000000000, 0x8080808000000000, 
0x0101010000000000, 0x0202020000000000, 0x0404040000000000, 0x0808080000000000, 
0x1010100000000000, 0x2020200000000000, 0x4040400000000000, 0x8080800000000000, 
0x0101000000000000, 0x0202000000000000, 0x0404000000000000, 0x0808000000000000, 
0x1010000000000000, 0x2020000000000000, 0x4040000000000000, 0x8080000000000000, 
0x0100000000000000, 0x0200000000000000, 0x0400000000000000, 0x0800000000000000, 
0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000
};

const bitbrd EASTRAYI[64] = {
0x00000000000000ff, 0x00000000000000fe, 0x00000000000000fc, 0x00000000000000f8, 
0x00000000000000f0, 0x00000000000000e0, 0x00000000000000c0, 0x0000000000000080, 
0x000000000000ff00, 0x000000000000fe00, 0x000000000000fc00, 0x000000000000f800, 
0x000000000000f000, 0x000000000000e000, 0x000000000000c000, 0x0000000000008000, 
0x0000000000ff0000, 0x0000000000fe0000, 0x0000000000fc0000, 0x0000000000f80000, 
0x0000000000f00000, 0x0000000000e00000, 0x0000000000c00000, 0x0000000000800000, 
0x00000000ff000000, 0x00000000fe000000, 0x00000000fc000000, 0x00000000f8000000, 
0x00000000f0000000, 0x00000000e0000000, 0x00000000c0000000, 0x0000000080000000, 
0x000000ff00000000, 0x000000fe00000000, 0x000000fc00000000, 0x000000f800000000, 
0x000000f000000000, 0x000000e000000000, 0x000000c000000000, 0x0000008000000000, 
0x0000ff0000000000, 0x0000fe0000000000, 0x0000fc0000000000, 0x0000f80000000000, 
0x0000f00000000000, 0x0000e00000000000, 0x0000c00000000000, 0x0000800000000000, 
0x00ff000000000000, 0x00fe000000000000, 0x00fc000000000000, 0x00f8000000000000, 
0x00f0000000000000, 0x00e0000000000000, 0x00c0000000000000, 0x0080000000000000, 
0xff00000000000000, 0xfe00000000000000, 0xfc00000000000000, 0xf800000000000000, 
0xf000000000000000, 0xe000000000000000, 0xc000000000000000, 0x8000000000000000
};

const bitbrd SERAYI[64] = {
0x8040201008040201, 0x0080402010080402, 0x0000804020100804, 0x0000008040201008, 
0x0000000080402010, 0x0000000000804020, 0x0000000000008040, 0x0000000000000080, 
0x4020100804020100, 0x8040201008040200, 0x0080402010080400, 0x0000804020100800, 
0x0000008040201000, 0x0000000080402000, 0x0000000000804000, 0x0000000000008000, 
0x2010080402010000, 0x4020100804020000, 0x8040201008040000, 0x0080402010080000, 
0x0000804020100000, 0x0000008040200000, 0x0000000080400000, 0x0000000000800000, 
0x1008040201000000, 0x2010080402000000, 0x4020100804000000, 0x8040201008000000, 
0x0080402010000000, 0x0000804020000000, 0x0000008040000000, 0x0000000080000000, 
0x0804020100000000, 0x1008040200000000, 0x2010080400000000, 0x4020100800000000, 
0x8040201000000000, 0x0080402000000000, 0x0000804000000000, 0x0000008000000000, 
0x0402010000000000, 0x0804020000000000, 0x1008040000000000, 0x2010080000000000, 
0x4020100000000000, 0x8040200000000000, 0x0080400000000000, 0x0000800000000000, 
0x0201000000000000, 0x0402000000000000, 0x0804000000000000, 0x1008000000000000, 
0x2010000000000000, 0x4020000000000000, 0x8040000000000000, 0x0080000000000000, 
0x0100000000000000, 0x0200000000000000, 0x0400000000000000, 0x0800000000000000, 
0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000
};

const bitbrd NERAYI[64] = {
0x0000000000000001, 0x0000000000000002, 0x0000000000000004, 0x0000000000000008, 
0x0000000000000010, 0x0000000000000020, 0x0000000000000040, 0x0000000000000080, 
0x0000000000000102, 0x0000000000000204, 0x0000000000000408, 0x0000000000000810, 
0x0000000000001020, 0x0000000000002040, 0x0000000000004080, 0x0000000000008000, 
0x0000000000010204, 0x0000000000020408, 0x0000000000040810, 0x0000000000081020, 
0x0000000000102040, 0x0000000000204080, 0x0000000000408000, 0x0000000000800000, 
0x0000000001020408, 0x0000000002040810, 0x0000000004081020, 0x0000000008102040, 
0x0000000010204080, 0x0000000020408000, 0x0000000040800000, 0x0000000080000000, 
0x0000000102040810, 0x0000000204081020, 0x0000000408102040, 0x0000000810204080, 
0x0000001020408000, 0x0000002040800000, 0x0000004080000000, 0x0000008000000000, 
0x0000010204081020, 0x0000020408102040, 0x0000040810204080, 0x0000081020408000, 
0x0000102040800000, 0x0000204080000000, 0x0000408000000000, 0x0000800000000000, 
0x0001020408102040, 0x0002040810204080, 0x0004081020408000, 0x0008102040800000, 
0x0010204080000000, 0x0020408000000000, 0x0040800000000000, 0x0080000000000000, 
0x0102040810204080, 0x0204081020408000, 0x0408102040800000, 0x0810204080000000, 
0x1020408000000000, 0x2040800000000000, 0x4080000000000000, 0x8000000000000000
};

const bitbrd WESTRAYI[64] = {
0x0000000000000001, 0x0000000000000003, 0x0000000000000007, 0x000000000000000f, 
0x000000000000001f, 0x000000000000003f, 0x000000000000007f, 0x00000000000000ff, 
0x0000000000000100, 0x0000000000000300, 0x0000000000000700, 0x0000000000000f00, 
0x0000000000001f00, 0x0000000000003f00, 0x0000000000007f00, 0x000000000000ff00, 
0x0000000000010000, 0x0000000000030000, 0x0000000000070000, 0x00000000000f0000, 
0x00000000001f0000, 0x00000000003f0000, 0x00000000007f0000, 0x0000000000ff0000, 
0x0000000001000000, 0x0000000003000000, 0x0000000007000000, 0x000000000f000000, 
0x000000001f000000, 0x000000003f000000, 0x000000007f000000, 0x00000000ff000000, 
0x0000000100000000, 0x0000000300000000, 0x0000000700000000, 0x0000000f00000000, 
0x0000001f00000000, 0x0000003f00000000, 0x0000007f00000000, 0x000000ff00000000, 
0x0000010000000000, 0x0000030000000000, 0x0000070000000000, 0x00000f0000000000, 
0x00001f0000000000, 0x00003f0000000000, 0x00007f0000000000, 0x0000ff0000000000, 
0x0001000000000000, 0x0003000000000000, 0x0007000000000000, 0x000f000000000000, 
0x001f000000000000, 0x003f000000000000, 0x007f000000000000, 0x00ff000000000000, 
0x0100000000000000, 0x0300000000000000, 0x0700000000000000, 0x0f00000000000000, 
0x1f00000000000000, 0x3f00000000000000, 0x7f00000000000000, 0xff00000000000000
};

const bitbrd NWRAYI[64] = {
0x0000000000000001, 0x0000000000000002, 0x0000000000000004, 0x0000000000000008, 
0x0000000000000010, 0x0000000000000020, 0x0000000000000040, 0x0000000000000080, 
0x0000000000000100, 0x0000000000000201, 0x0000000000000402, 0x0000000000000804, 
0x0000000000001008, 0x0000000000002010, 0x0000000000004020, 0x0000000000008040, 
0x0000000000010000, 0x0000000000020100, 0x0000000000040201, 0x0000000000080402, 
0x0000000000100804, 0x0000000000201008, 0x0000000000402010, 0x0000000000804020, 
0x0000000001000000, 0x0000000002010000, 0x0000000004020100, 0x0000000008040201, 
0x0000000010080402, 0x0000000020100804, 0x0000000040201008, 0x0000000080402010, 
0x0000000100000000, 0x0000000201000000, 0x0000000402010000, 0x0000000804020100, 
0x0000001008040201, 0x0000002010080402, 0x0000004020100804, 0x0000008040201008, 
0x0000010000000000, 0x0000020100000000, 0x0000040201000000, 0x0000080402010000, 
0x0000100804020100, 0x0000201008040201, 0x0000402010080402, 0x0000804020100804, 
0x0001000000000000, 0x0002010000000000, 0x0004020100000000, 0x0008040201000000, 
0x0010080402010000, 0x0020100804020100, 0x0040201008040201, 0x0080402010080402, 
0x0100000000000000, 0x0201000000000000, 0x0402010000000000, 0x0804020100000000, 
0x1008040201000000, 0x2010080402010000, 0x4020100804020100, 0x8040201008040201
};

const bitbrd SWRAYI[64] = {
0x0000000000000001, 0x0000000000000102, 0x0000000000010204, 0x0000000001020408, 
0x0000000102040810, 0x0000010204081020, 0x0001020408102040, 0x0102040810204080, 
0x0000000000000100, 0x0000000000010200, 0x0000000001020400, 0x0000000102040800, 
0x0000010204081000, 0x0001020408102000, 0x0102040810204000, 0x0204081020408000, 
0x0000000000010000, 0x0000000001020000, 0x0000000102040000, 0x0000010204080000, 
0x0001020408100000, 0x0102040810200000, 0x0204081020400000, 0x0408102040800000, 
0x0000000001000000, 0x0000000102000000, 0x0000010204000000, 0x0001020408000000, 
0x0102040810000000, 0x0204081020000000, 0x0408102040000000, 0x0810204080000000, 
0x0000000100000000, 0x0000010200000000, 0x0001020400000000, 0x0102040800000000, 
0x0204081000000000, 0x0408102000000000, 0x0810204000000000, 0x1020408000000000, 
0x0000010000000000, 0x0001020000000000, 0x0102040000000000, 0x0204080000000000, 
0x0408100000000000, 0x0810200000000000, 0x1020400000000000, 0x2040800000000000, 
0x0001000000000000, 0x0102000000000000, 0x0204000000000000, 0x0408000000000000, 
0x0810000000000000, 0x1020000000000000, 0x2040000000000000, 0x4080000000000000, 
0x0100000000000000, 0x0200000000000000, 0x0400000000000000, 0x0800000000000000, 
0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000
};

const bitbrd NEIGHBORS[64] = {
0x0000000000000302, 0x0000000000000705, 0x0000000000000e0a, 0x0000000000001c14,
0x0000000000003828, 0x0000000000007050, 0x000000000000e0a0, 0x000000000000c040,
0x0000000000030203, 0x0000000000070507, 0x00000000000e0a0e, 0x00000000001c141c,
0x0000000000382838, 0x0000000000705070, 0x0000000000e0a0e0, 0x0000000000c040c0,
0x0000000003020300, 0x0000000007050700, 0x000000000e0a0e00, 0x000000001c141c00,
0x0000000038283800, 0x0000000070507000, 0x00000000e0a0e000, 0x00000000c040c000,
0x0000000302030000, 0x0000000705070000, 0x0000000e0a0e0000, 0x0000001c141c0000,
0x0000003828380000, 0x0000007050700000, 0x000000e0a0e00000, 0x000000c040c00000,
0x0000030203000000, 0x0000070507000000, 0x00000e0a0e000000, 0x00001c141c000000,
0x0000382838000000, 0x0000705070000000, 0x0000e0a0e0000000, 0x0000c040c0000000,
0x0003020300000000, 0x0007050700000000, 0x000e0a0e00000000, 0x001c141c00000000,
0x0038283800000000, 0x0070507000000000, 0x00e0a0e000000000, 0x00c040c000000000,
0x0302030000000000, 0x0705070000000000, 0x0e0a0e0000000000, 0x1c141c0000000000,
0x3828380000000000, 0x7050700000000000, 0xe0a0e00000000000, 0xc040c00000000000,
0x0203000000000000, 0x0507000000000000, 0x0a0e000000000000, 0x141c000000000000,
0x2838000000000000, 0x5070000000000000, 0xa0e0000000000000, 0x40c0000000000000
};

const int BOARD_REGIONS[64] = {
1, 1, 2, 2, 2, 2, 3, 3,
1, 1, 2, 2, 2, 2, 3, 3,
4, 4, 5, 5, 5, 5, 6, 6,
4, 4, 5, 5, 5, 5, 6, 6,
4, 4, 5, 5, 5, 5, 6, 6,
4, 4, 5, 5, 5, 5, 6, 6,
7, 7, 8, 8, 8, 8, 9, 9,
7, 7, 8, 8, 8, 8, 9, 9
};

const int SQ_VAL[64] = {
/*9, 1, 7, 5, 5, 7, 1, 9,
1, 0, 2, 3, 3, 2, 0, 1,
7, 2, 6, 4, 4, 6, 2, 7,
5, 3, 4, 0, 0, 4, 3, 5,
5, 3, 4, 0, 0, 4, 3, 5,
7, 2, 6, 4, 4, 6, 2, 7,
1, 0, 2, 3, 3, 2, 0, 1,
9, 1, 7, 5, 5, 7, 1, 9*/ // sucky
9, 1, 7, 6, 6, 7, 1, 9,
1, 0, 2, 3, 3, 2, 0, 1,
7, 2, 5, 4, 4, 5, 2, 7,
6, 3, 4, 0, 0, 4, 3, 6,
6, 3, 4, 0, 0, 4, 3, 6,
7, 2, 5, 4, 4, 5, 2, 7,
1, 0, 2, 3, 3, 2, 0, 1,
9, 1, 7, 6, 6, 7, 1, 9 // better
};

class Board {

private:
    bool checkMove(int index, int side);

    int countSetBits(bitbrd i);
    int bitScanForward(bitbrd bb);
    int bitScanReverse(bitbrd bb);

    bitbrd northFill(int index, bitbrd self, bitbrd pos);
    bitbrd southFill(int index, bitbrd self, bitbrd pos);
    bitbrd eastFill(int index, bitbrd self, bitbrd pos);
    bitbrd westFill(int index, bitbrd self, bitbrd pos);
    bitbrd neFill(int index, bitbrd self, bitbrd pos);
    bitbrd nwFill(int index, bitbrd self, bitbrd pos);
    bitbrd swFill(int index, bitbrd self, bitbrd pos);
    bitbrd seFill(int index, bitbrd self, bitbrd pos);
      
public:
    bitbrd taken;
    bitbrd black;

    Board();
    Board(bitbrd b, bitbrd t);
    ~Board();
    Board *copy();

    bool isDone();
    bool hasMoves(int side);
    bool checkMove(Move *m, Side side);
    void doMove(int index, int side);
    bitbrd getDoMove(int index, int side);
    void makeMove(int index, bitbrd changeMask, int side);
    void undoMove(int index, bitbrd changeMask, int side);
    bitbrd getLegal(int side);
    MoveList getLegalMoves(int side);
    MoveList getLegalMovesOrdered(int side, MoveList &priority, int &hashed);
    int getLegalMoves4(int side, int &m1, int &m2, int &m3);
    int getLegalMoves3(int side, int &m1, int &m2);

    int numLegalMoves(int side);
    int potentialMobility(int side);
    int getStability(int side);

    void setBoard(char data[]);
    char *toString();
    bitbrd toBits(int side);
    int count(int side);
    bitbrd getTaken();
    int countEmpty();
};

#endif
