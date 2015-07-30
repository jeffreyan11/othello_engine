#include "board.h"
#include <iostream>

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

/**
 * @brief Make a 8x8 othello board and initialize it to the standard setup.
 */
Board::Board() {
    taken = 0x0000001818000000;
    black = 0x0000000810000000;
}

/**
 * @brief A constructor allowing specification of taken, black.
*/
Board::Board(bitbrd t, bitbrd b) {
    taken = t;
    black = b;
}

/**
 * @brief Destructor for the board.
 */
Board::~Board() {
}

/**
 * @brief Returns a copy of this board.
 */
Board *Board::copy() {
    Board *newBoard = new Board(taken, black);
    return newBoard;
}
 
/**
 * @brief Returns true if the game is finished; false otherwise. The game is
 * finished if neither side has a legal move.
 */
bool Board::isDone() {
    return !(hasMoves(CBLACK) || hasMoves(CWHITE));
}

/**
 * @brief Returns true if there are legal moves for the given side.
 */
bool Board::hasMoves(int side) {
    return numLegalMoves(side);
}

/**
 * @brief Returns true if a move is legal for the given side; false otherwise.
 * For debugging only, not used anywhere in this program.
 */
bool Board::checkMove(Move *m, Side side) {
    // Passing is only legal if you have no moves.
    if (m == NULL) return !hasMoves((side == BLACK) ? CBLACK : CWHITE);

    bitbrd legal = getLegal(side);

    return legal & MOVEMASK[m->getX() + 8 * m->getY()];
}

/**
 * @brief Overloaded function for internal use with getLegalMoves().
*/
bool Board::checkMove(int index, int side) {
    bitbrd legal = getLegal(side);
    return legal & MOVEMASK[index];
}

/**
 * @brief Modifies the board to reflect the specified move.
 * 
 * This algorithm modifies the bitboards by lookup with precalculated tables in
 * each of the eight directions. A switch with the board region is used to only
 * consider certain directions for efficiency.
 */
void Board::doMove(int index, int side) {
    // A NULL move means pass.
    if (index == MOVE_NULL) {
        return;
    }

    // Ignore if move is invalid.
    //if (!checkMove(index, side)) return;

    bitbrd changeMask = 0;
    bitbrd pos = (side == CWHITE) ? ~black : ~(taken^black);
    bitbrd self = (side == CBLACK) ? black : taken^black;

    switch(BOARD_REGIONS[index]) {
    case 2:
        changeMask = southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    case 4:
        changeMask = northFill(index, self, pos);
        changeMask |= southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    case 6:
        changeMask = northFill(index, self, pos);
        changeMask |= southFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        break;
    case 8:
        changeMask = northFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        break;
    case 1:
        changeMask = southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    case 3:
        changeMask = southFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        break;
    case 7:
        changeMask = northFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        break;
    case 9:
        changeMask = northFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        break;
    case 5:
        changeMask = northFill(index, self, pos);
        changeMask |= southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    }

    changeMask |= MOVEMASK[index];

    // update taken, black
    taken |= changeMask;
    if(side == CBLACK)
        black |= changeMask;
    else
        black &= ~changeMask;
}

/**
 * @brief This function returns the mask of changed bits.
 */
bitbrd Board::getDoMove(int index, int side) {
    bitbrd changeMask = 0;
    bitbrd pos = (side == CWHITE) ? ~black : ~(taken^black);
    bitbrd self = (side == CBLACK) ? black : taken^black;

    switch(BOARD_REGIONS[index]) {
    case 2:
        changeMask = southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    case 4:
        changeMask = northFill(index, self, pos);
        changeMask |= southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    case 6:
        changeMask = northFill(index, self, pos);
        changeMask |= southFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        break;
    case 8:
        changeMask = northFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        break;
    case 1:
        changeMask = southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    case 3:
        changeMask = southFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        break;
    case 7:
        changeMask = northFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        break;
    case 9:
        changeMask = northFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        break;
    case 5:
        changeMask = northFill(index, self, pos);
        changeMask |= southFill(index, self, pos);
        changeMask |= eastFill(index, self, pos);
        changeMask |= westFill(index, self, pos);
        changeMask |= neFill(index, self, pos);
        changeMask |= nwFill(index, self, pos);
        changeMask |= swFill(index, self, pos);
        changeMask |= seFill(index, self, pos);
        break;
    }

    return changeMask;
}

void Board::makeMove(int index, bitbrd changeMask, int side) {
    taken |= MOVEMASK[index];
    black ^= changeMask | ((side == CBLACK) * MOVEMASK[index]);
}

void Board::undoMove(int index, bitbrd changeMask, int side) {
    taken ^= MOVEMASK[index];
    black ^= changeMask | ((side == CBLACK) * MOVEMASK[index]);
}

/**
 * @brief Returns a list of all legal moves.
*/
MoveList Board::getLegalMoves(int side) {
    MoveList result;
    bitbrd temp = getLegal(side);
    while(temp) {
        result.add(bitScanForward(temp));
        temp &= temp-1;
    }
    return result;
}

/**
 * @brief Returns a list of all legal moves, with priorities for sorting.
*/
MoveList Board::getLegalMovesOrdered(int side, MoveList &priority, int &hashed) {
    MoveList result;
    bitbrd temp = getLegal(side);

    if(hashed != -1) {
        result.add(hashed);
        priority.add(100000);
        temp ^= MOVEMASK[hashed];
    }

    while(temp) {
        result.add(bitScanForward(temp));
        if(!(NEIGHBORS[result.last()] & ~taken))
            priority.add( 100 + 10*SQ_VAL[result.last()] );
        else priority.add( 10*SQ_VAL[result.last()] );
        temp &= temp-1;
    }

    return result;
}

/**
 * @brief Returns a list of all legal moves, given 4 or less empty squares.
*/
int Board::getLegalMoves4(int side, int &m1, int &m2, int &m3) {
    int m4 = MOVE_NULL;
    bitbrd temp = getLegal(side);
    int n = 0;

    if(temp) {
        m1 = bitScanForward(temp);
        temp &= temp-1; n++;
      if(temp) {
          m2 = bitScanForward(temp);
          temp &= temp-1; n++;
        if(temp) {
            m3 = bitScanForward(temp);
            temp &= temp-1; n++;
          if(temp) {
              m4 = bitScanForward(temp);
              n++;
          }
        }
      }
    }

    // parity sorting
    if(n == 2) {
        if( (NEIGHBORS[m1] & ~taken) && !(NEIGHBORS[m2] & ~taken) ) {
            int temp = m1;
            m1 = m2;
            m2 = temp;
        }
    }
    else if(n == 3) {
        if( (NEIGHBORS[m1] & ~taken) ) {
            if( !(NEIGHBORS[m2] & ~taken) ) {
                int temp = m1;
                m1 = m2;
                m2 = temp;
            }
            else if ( !(NEIGHBORS[m3] & ~taken) ) {
                int temp = m1;
                m1 = m3;
                m3 = temp;
            }
        }
    }
    else if(n == 4) {
        if( (NEIGHBORS[m1] & ~taken) ) {
            if( !(NEIGHBORS[m2] & ~taken) ) {
                int temp = m1;
                m1 = m2;
                m2 = m4;
                m4 = temp;
            }
            else if ( !(NEIGHBORS[m3] & ~taken) ) {
                int temp = m1;
                m1 = m3;
                m3 = temp;
                temp = m2;
                m2 = m4;
                m4 = temp;
            }
            else if ( !(NEIGHBORS[m4] & ~taken) ) {
                int temp = m1;
                m1 = m4;
                m4 = temp;
            }
        }
        else if( (NEIGHBORS[m2] & ~taken) ) {
            if ( !(NEIGHBORS[m3] & ~taken) ) {
                int temp = m2;
                m2 = m3;
                m3 = temp;
            }
            else if ( !(NEIGHBORS[m4] & ~taken) ) {
                int temp = m2;
                m2 = m4;
                m4 = temp;
            }
        }
    }

    return m4;
}

/**
 * @brief Returns a list of all legal moves, given 3 or less empty squares.
*/
int Board::getLegalMoves3(int side, int &m1, int &m2) {
    int result = MOVE_NULL;
    bitbrd temp = getLegal(side);
    int n = 0;

    if(temp) {
        m1 = bitScanForward(temp);
        temp &= temp-1; n++;
      if(temp) {
          m2 = bitScanForward(temp);
          temp &= temp-1; n++;
        if(temp) {
            result = bitScanForward(temp);
            n++;
        }
      }
    }

    // parity sorting
    if(n == 2) {
        if( (NEIGHBORS[m1] & ~taken) && !(NEIGHBORS[m2] & ~taken) ) {
            int temp = m1;
            m1 = m2;
            m2 = temp;
        }
    }
    else if(n == 3) {
        if( (NEIGHBORS[m1] & ~taken) ) {
            if( !(NEIGHBORS[m2] & ~taken) ) {
                int temp = m1;
                m1 = m2;
                m2 = temp;
            }
            else if ( !(NEIGHBORS[result] & ~taken) ) {
                int temp = m1;
                m1 = result;
                result = temp;
            }
        }
    }

    return result;
}

/**
 * @brief Returns a bitmask with a 1 set in every square that is a legal move
 * for the given side.
 * 
 * This method operates by checking in all eight directions, first for the line
 * of pieces of the opposite color, then for the anchor once the line ends.
*/
bitbrd Board::getLegal(int side) {
    bitbrd result = 0;
    bitbrd self = (side == CBLACK) ? (black) : (taken ^ black);
    bitbrd opp = (side == CBLACK) ? (taken ^ black) : (black);

#if KOGGE_STONE
    bitbrd other = opp & 0x00FFFFFFFFFFFF00;
    // north and south
    bitbrd templ = other & (self << 8);
    bitbrd tempr = other & (self >> 8);
    templ |= other & (templ << 8);
    tempr |= other & (tempr >> 8);
    bitbrd maskl = other & (other << 8);
    bitbrd maskr = other & (other >> 8);
    templ |= maskl & (templ << 16);
    tempr |= maskr & (tempr >> 16);
    templ |= maskl & (templ << 16);
    tempr |= maskr & (tempr >> 16);
    result |= (templ << 8) | (tempr >> 8);

    other = opp & 0x7E7E7E7E7E7E7E7E;
    // east and west
    templ = other & (self << 1);
    tempr = other & (self >> 1);
    templ |= other & (templ << 1);
    tempr |= other & (tempr >> 1);
    maskl = other & (other << 1);
    maskr = other & (other >> 1);
    templ |= maskl & (templ << 2);
    tempr |= maskr & (tempr >> 2);
    templ |= maskl & (templ << 2);
    tempr |= maskr & (tempr >> 2);
    result |= (templ << 1) | (tempr >> 1);

    // ne and sw
    templ = other & (self << 7);
    tempr = other & (self >> 7);
    templ |= other & (templ << 7);
    tempr |= other & (tempr >> 7);
    maskl = other & (other << 7);
    maskr = other & (other >> 7);
    templ |= maskl & (templ << 14);
    tempr |= maskr & (tempr >> 14);
    templ |= maskl & (templ << 14);
    tempr |= maskr & (tempr >> 14);
    result |= (templ << 7) | (tempr >> 7);

    // nw and se
    templ = other & (self << 9);
    tempr = other & (self >> 9);
    templ |= other & (templ << 9);
    tempr |= other & (tempr >> 9);
    maskl = other & (other << 9);
    maskr = other & (other >> 9);
    templ |= maskl & (templ << 18);
    tempr |= maskr & (tempr >> 18);
    templ |= maskl & (templ << 18);
    tempr |= maskr & (tempr >> 18);
    result |= (templ << 9) | (tempr >> 9);

    return result & ~taken;

#else
    bitbrd other = opp & 0x00FFFFFFFFFFFF00;
    // north and south
    bitbrd tempM = (((self << 8) | (self >> 8)) & other);
    tempM |= (((tempM << 8) | (tempM >> 8)) & other);
    tempM |= (((tempM << 8) | (tempM >> 8)) & other);
    tempM |= (((tempM << 8) | (tempM >> 8)) & other);
    tempM |= (((tempM << 8) | (tempM >> 8)) & other);
    tempM |= (((tempM << 8) | (tempM >> 8)) & other);
    result = ((tempM << 8) | (tempM >> 8));

    other = opp & 0x7E7E7E7E7E7E7E7E;
    // east and west
    tempM = (((self << 1) | (self >> 1)) & other);
    tempM |= (((tempM << 1) | (tempM >> 1)) & other);
    tempM |= (((tempM << 1) | (tempM >> 1)) & other);
    tempM |= (((tempM << 1) | (tempM >> 1)) & other);
    tempM |= (((tempM << 1) | (tempM >> 1)) & other);
    tempM |= (((tempM << 1) | (tempM >> 1)) & other);
    result |= ((tempM << 1) | (tempM >> 1));

    // ne and sw
    tempM = (((self << 7) | (self >> 7)) & other);
    tempM |= (((tempM << 7) | (tempM >> 7)) & other);
    tempM |= (((tempM << 7) | (tempM >> 7)) & other);
    tempM |= (((tempM << 7) | (tempM >> 7)) & other);
    tempM |= (((tempM << 7) | (tempM >> 7)) & other);
    tempM |= (((tempM << 7) | (tempM >> 7)) & other);
    result |= ((tempM << 7) | (tempM >> 7));

    // nw and se
    tempM = (((self << 9) | (self >> 9)) & other);
    tempM |= (((tempM << 9) | (tempM >> 9)) & other);
    tempM |= (((tempM << 9) | (tempM >> 9)) & other);
    tempM |= (((tempM << 9) | (tempM >> 9)) & other);
    tempM |= (((tempM << 9) | (tempM >> 9)) & other);
    tempM |= (((tempM << 9) | (tempM >> 9)) & other);
    result |= ((tempM << 9) | (tempM >> 9));

    return result & ~taken;
#endif
}

int Board::numLegalMoves(int side) {
    return countSetBits(getLegal(side));
}

int Board::potentialMobility(int side) {
    bitbrd other = (side == CBLACK) ? (taken ^ black) : (black);
    bitbrd empty = ~taken;

    // Potential mobility = empty squares adjacent to opponent's pieces
    bitbrd result = (other >> 8) | (other << 8);
    result &= empty;

    empty &= 0x7E7E7E7E7E7E7E7E;
    result |= (other << 1) | (other >> 1);
    result |= (other >> 7) | (other << 7);
    result |= (other >> 9) | (other << 9);
    result &= empty;

    return countSetBits(result);
}

int Board::getStability(int side) {
    bitbrd result = 0;
    bitbrd self = (side == CBLACK) ? (black) : (taken ^ black);

    bitbrd border = self & 0xFF818181818181FF;

    // north and south
    // calculate full lines
    bitbrd full = taken & (((taken << 8) & (taken >> 8)) | border);
    full &= (((full << 8) & (full >> 8)) | border);
    full &= (((full << 8) & (full >> 8)) | border);
    full &= (((full << 8) & (full >> 8)) | border);
    full &= (((full << 8) & (full >> 8)) | border);
    full = (full << 8) & (full >> 8);

    // propagate edge pieces, anywhere there are own pieces or full lines,
    // to find single direction stability
    bitbrd tempM = border;
    tempM |= (((border << 8) | (border >> 8)) & full);
    tempM |= (((tempM << 8) | (tempM >> 8)) & full);
    tempM |= (((tempM << 8) | (tempM >> 8)) & full);
    tempM |= (((tempM << 8) | (tempM >> 8)) & full);
    tempM |= (((tempM << 8) | (tempM >> 8)) & full);
    tempM |= (((tempM << 8) | (tempM >> 8)) & full);
    full = tempM;

    tempM = border;
    tempM |= (((border << 8) | (border >> 8)) & self);
    tempM |= (((tempM << 8) | (tempM >> 8)) & self);
    tempM |= (((tempM << 8) | (tempM >> 8)) & self);
    tempM |= (((tempM << 8) | (tempM >> 8)) & self);
    tempM |= (((tempM << 8) | (tempM >> 8)) & self);
    tempM |= (((tempM << 8) | (tempM >> 8)) & self);
    result = (full | tempM);

    // east and west
    full = taken & (((taken << 1) & (taken >> 1)) | border);
    full &= (((full << 1) & (full >> 1)) | border);
    full &= (((full << 1) & (full >> 1)) | border);
    full &= (((full << 1) & (full >> 1)) | border);
    full &= (((full << 1) & (full >> 1)) | border);
    full = (full << 1) & (full >> 1);

    tempM = border;
    tempM |= (((border << 1) | (border >> 1)) & full);
    tempM |= (((tempM << 1) | (tempM >> 1)) & full);
    tempM |= (((tempM << 1) | (tempM >> 1)) & full);
    tempM |= (((tempM << 1) | (tempM >> 1)) & full);
    tempM |= (((tempM << 1) | (tempM >> 1)) & full);
    tempM |= (((tempM << 1) | (tempM >> 1)) & full);
    full = tempM;

    tempM = border;
    tempM |= (((border << 1) | (border >> 1)) & self);
    tempM |= (((tempM << 1) | (tempM >> 1)) & self);
    tempM |= (((tempM << 1) | (tempM >> 1)) & self);
    tempM |= (((tempM << 1) | (tempM >> 1)) & self);
    tempM |= (((tempM << 1) | (tempM >> 1)) & self);
    tempM |= (((tempM << 1) | (tempM >> 1)) & self);
    result &= (full | tempM);

    // ne and sw
    full = taken & (((taken << 7) & (taken >> 7)) | border);
    full &= (((full << 7) & (full >> 7)) | border);
    full &= (((full << 7) & (full >> 7)) | border);
    full &= (((full << 7) & (full >> 7)) | border);
    full &= (((full << 7) & (full >> 7)) | border);
    full = (full << 7) & (full >> 7);

    tempM = border;
    tempM |= (((border << 7) | (border >> 7)) & full);
    tempM |= (((tempM << 7) | (tempM >> 7)) & full);
    tempM |= (((tempM << 7) | (tempM >> 7)) & full);
    tempM |= (((tempM << 7) | (tempM >> 7)) & full);
    tempM |= (((tempM << 7) | (tempM >> 7)) & full);
    tempM |= (((tempM << 7) | (tempM >> 7)) & full);
    full = tempM;

    tempM = border;
    tempM |= (((border << 7) | (border >> 7)) & self);
    tempM |= (((tempM << 7) | (tempM >> 7)) & self);
    tempM |= (((tempM << 7) | (tempM >> 7)) & self);
    tempM |= (((tempM << 7) | (tempM >> 7)) & self);
    tempM |= (((tempM << 7) | (tempM >> 7)) & self);
    tempM |= (((tempM << 7) | (tempM >> 7)) & self);
    result &= (full | tempM);

    // nw and se
    full = taken & (((taken << 9) & (taken >> 9)) | border);
    full &= (((full << 9) & (full >> 9)) | border);
    full &= (((full << 9) & (full >> 9)) | border);
    full &= (((full << 9) & (full >> 9)) | border);
    full &= (((full << 9) & (full >> 9)) | border);
    full = (full << 9) & (full >> 9);

    tempM = border;
    tempM = (((border << 9) | (border >> 9)) & full);
    tempM |= (((tempM << 9) | (tempM >> 9)) & full);
    tempM |= (((tempM << 9) | (tempM >> 9)) & full);
    tempM |= (((tempM << 9) | (tempM >> 9)) & full);
    tempM |= (((tempM << 9) | (tempM >> 9)) & full);
    tempM |= (((tempM << 9) | (tempM >> 9)) & full);
    full = tempM;

    tempM = border;
    tempM = (((border << 9) | (border >> 9)) & self);
    tempM |= (((tempM << 9) | (tempM >> 9)) & self);
    tempM |= (((tempM << 9) | (tempM >> 9)) & self);
    tempM |= (((tempM << 9) | (tempM >> 9)) & self);
    tempM |= (((tempM << 9) | (tempM >> 9)) & self);
    tempM |= (((tempM << 9) | (tempM >> 9)) & self);
    result &= (full | tempM);

    return countSetBits(result & self);
}
/*
int Board::getStability(int side) {
    bitbrd result = 0;
    bitbrd self = (side == CBLACK) ? (black) : (taken ^ black);
    //bitbrd opp = (side == CBLACK) ? (taken ^ black) : (black);

    //bitbrd other = opp & 0x00FFFFFFFFFFFF00;
    bitbrd border = self & 0xFF818181818181FF;

    // north and south
    // calculate full lines
    bitbrd full = taken & (((taken << 8) & (taken >> 8)) | border);
    full &= (((full << 8) & (full >> 8)) | border);
    full &= (((full << 8) & (full >> 8)) | border);
    full &= (((full << 8) & (full >> 8)) | border);
    full &= (((full << 8) & (full >> 8)) | border);
    full = (full << 8) & (full >> 8);

    // propagate edge pieces, anywhere there are own pieces or full lines,
    // to find single direction stability
    full |= self;
    bitbrd tempM = border;
    tempM |= (((border << 8) | (border >> 8)) & full);
    tempM |= (((tempM << 8) | (tempM >> 8)) & full);
    tempM |= (((tempM << 8) | (tempM >> 8)) & full);
    tempM |= (((tempM << 8) | (tempM >> 8)) & full);
    tempM |= (((tempM << 8) | (tempM >> 8)) & full);
    tempM |= (((tempM << 8) | (tempM >> 8)) & full);
    result = tempM;
    //result = ((tempM << 8) | (tempM >> 8));

    //other = opp & 0x7E7E7E7E7E7E7E7E;
    // east and west
    full = taken & (((taken << 1) & (taken >> 1)) | border);
    full &= (((full << 1) & (full >> 1)) | border);
    full &= (((full << 1) & (full >> 1)) | border);
    full &= (((full << 1) & (full >> 1)) | border);
    full &= (((full << 1) & (full >> 1)) | border);
    full = (full << 1) & (full >> 1);
    full |= self;

    tempM = border;
    tempM |= (((border << 1) | (border >> 1)) & full);
    tempM |= (((tempM << 1) | (tempM >> 1)) & full);
    tempM |= (((tempM << 1) | (tempM >> 1)) & full);
    tempM |= (((tempM << 1) | (tempM >> 1)) & full);
    tempM |= (((tempM << 1) | (tempM >> 1)) & full);
    tempM |= (((tempM << 1) | (tempM >> 1)) & full);
    result &= tempM;
    //result |= ((tempM << 1) | (tempM >> 1));

    // ne and sw
    full = taken & (((taken << 7) & (taken >> 7)) | border);
    full &= (((full << 7) & (full >> 7)) | border);
    full &= (((full << 7) & (full >> 7)) | border);
    full &= (((full << 7) & (full >> 7)) | border);
    full &= (((full << 7) & (full >> 7)) | border);
    full = (full << 7) & (full >> 7);
    full |= self;

    tempM = border;
    tempM |= (((border << 7) | (border >> 7)) & full);
    tempM |= (((tempM << 7) | (tempM >> 7)) & full);
    tempM |= (((tempM << 7) | (tempM >> 7)) & full);
    tempM |= (((tempM << 7) | (tempM >> 7)) & full);
    tempM |= (((tempM << 7) | (tempM >> 7)) & full);
    tempM |= (((tempM << 7) | (tempM >> 7)) & full);
    result &= tempM;
    //result |= ((tempM << 7) | (tempM >> 7));

    // nw and se
    full = taken & (((taken << 9) & (taken >> 9)) | border);
    full &= (((full << 9) & (full >> 9)) | border);
    full &= (((full << 9) & (full >> 9)) | border);
    full &= (((full << 9) & (full >> 9)) | border);
    full &= (((full << 9) & (full >> 9)) | border);
    full = (full << 9) & (full >> 9);
    full |= self;

    tempM = border;
    tempM = (((border << 9) | (border >> 9)) & full);
    tempM |= (((tempM << 9) | (tempM >> 9)) & full);
    tempM |= (((tempM << 9) | (tempM >> 9)) & full);
    tempM |= (((tempM << 9) | (tempM >> 9)) & full);
    tempM |= (((tempM << 9) | (tempM >> 9)) & full);
    tempM |= (((tempM << 9) | (tempM >> 9)) & full);
    result &= tempM;
    //result |= ((tempM << 9) | (tempM >> 9));

    return countSetBits(result & self);
}
*/

bitbrd Board::toBits(int side) {
    return (side == CBLACK) ? black : (taken ^ black);
}

/*
 * Sets the board state given an 8x8 char array where 'w', 'O' indicates a white
 * piece and 'b', 'X' indicates a black piece. Mainly for testing purposes.
 */
void Board::setBoard(char data[]) {
    taken = 0;
    black = 0;
    for (int i = 0; i < 64; i++) {
        if (data[i] == 'b' || data[i] == 'B' || data[i] == 'X') {
            taken |= MOVEMASK[i];
            black |= MOVEMASK[i];
        } if (data[i] == 'w' || data[i] == 'W' || data[i] == 'O') {
            taken |= MOVEMASK[i];
        }
    }
}

char *Board::toString() {
    char *result = new char[64];
    for (int i = 0; i < 64; i++) {
        if (taken & MOVEMASK[i]) {
            if (black & MOVEMASK[i])
                result[i] = 'b';
            else
                result[i] = 'w';
        }
        else
            result[i] = '-';
    }
    return result;
}

bitbrd Board::getTaken() {
    return taken;
}

int Board::countEmpty() {
    return countSetBits(~taken);
}

/*
 * Current count of given side's stones.
 */
int Board::count(int side) {
    bitbrd i = (side == CBLACK) ? (black) : (black^taken);
    return countSetBits(i);
}

bitbrd Board::northFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = NORTHRAY[index];
    bitbrd block = result & pos;
    if(block) {
        int anchor = bitScanReverse(block);
        // use multiplication to reduce branching
        return ((self & MOVEMASK[anchor]) && true) *
                (result ^ NORTHRAYI[anchor]);
    }
    return 0;
}

bitbrd Board::southFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = SOUTHRAY[index];
    bitbrd block = result & pos;
    block &= -block & self;
    if((block >> 8) & ~pos) {
        int anchor = bitScanForward(block);
        return (result ^ SOUTHRAYI[anchor]);
    }
    return 0;
}

bitbrd Board::eastFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = EASTRAY[index];
    bitbrd block = result & pos;
    block &= -block & self;
    if((block >> 1) & ~pos) {
        int anchor = bitScanForward(block);
        return (result ^ EASTRAYI[anchor]);
    }
    return 0;
}

bitbrd Board::westFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = WESTRAY[index];
    bitbrd block = result & pos;
    if(block) {
        int anchor = bitScanReverse(block);
        return ((self & MOVEMASK[anchor]) && true) *
                (result ^ WESTRAYI[anchor]);
    }
    return 0;
}

bitbrd Board::neFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = NERAY[index];
    bitbrd block = result & pos;
    if(block) {
        int anchor = bitScanReverse(block);
        return ((self & MOVEMASK[anchor]) && true) *
                (result ^ NERAYI[anchor]);
    }
    return 0;
}

bitbrd Board::nwFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = NWRAY[index];
    bitbrd block = result & pos;
    if(block) {
        int anchor = bitScanReverse(block);
        return ((self & MOVEMASK[anchor]) && true) *
                (result ^ NWRAYI[anchor]);
    }
    return 0;
}

bitbrd Board::swFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = SWRAY[index];
    bitbrd block = result & pos;
    block &= -block & self;
    if((block >> 7) & ~pos) {
        int anchor = bitScanForward(block);
        return (result ^ SWRAYI[anchor]);
    }
    return 0;
}

bitbrd Board::seFill(int index, bitbrd self, bitbrd pos) {
    bitbrd result = SERAY[index];
    bitbrd block = result & pos;
    block &= -block & self;
    if((block >> 9) & ~pos) {
        int anchor = bitScanForward(block);
        return (result ^ SERAYI[anchor]);
    }
    return 0;
}
