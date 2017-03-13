#include <iostream>
#include "common.h"

void swap(MoveList &moves, MoveList &scores, int i, int j);
int partition(MoveList &moves, MoveList &scores, int left, int right,
    int pindex);

// For bitscan
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

int countSetBits(bitbrd i) {
    /*
    #if defined(__x86_64__)
        asm ("popcnt %1, %0" : "=r" (i) : "r" (i));
        return (int) i;
    #elif defined(__i386)
        int a = (int) (i & 0xFFFFFFFF);
        int b = (int) (i >> 32);
        asm ("popcntl %1, %0" : "=r" (a) : "r" (a));
        asm ("popcntl %1, %0" : "=r" (b) : "r" (b));
        return a+b;
    #else
    */
        i = i - ((i >> 1) & 0x5555555555555555);
        i = (i & 0x3333333333333333) + ((i >> 2) & 0x3333333333333333);
        i = (((i + (i >> 4)) & 0x0F0F0F0F0F0F0F0F) *
              0x0101010101010101) >> 56;
        return (int) i;
    //#endif
}

// BitScan forward and reverse algorithms from
// https://chessprogramming.wikispaces.com/BitScan
int bitScanForward(bitbrd bb) {
    #if defined(__x86_64__)
        asm ("bsf %1, %0" : "=r" (bb) : "r" (bb));
        return (int) bb;
    #else
        return index64[(int)(((bb ^ (bb-1)) * 0x03f79d71b4cb0a89) >> 58)];
    #endif
}

int bitScanReverse(bitbrd bb) {
    #if defined(__x86_64__)
        asm ("bsr %1, %0" : "=r" (bb) : "r" (bb));
        return (int) bb;
    #elif defined(__i386)
        int b = (int) (bb >> 32);
        if(b) {
            asm ("bsrl %1, %0" : "=r" (b) : "r" (b));
            return b+32;
        }
        else {
            int a = (int) (bb & 0xFFFFFFFF);
            asm ("bsrl %1, %0" : "=r" (a) : "r" (a));
            return a;
        }
    #else
        bb |= bb >> 1;
        bb |= bb >> 2;
        bb |= bb >> 4;
        bb |= bb >> 8;
        bb |= bb >> 16;
        bb |= bb >> 32;
        return index64[(int)((bb * 0x03f79d71b4cb0a89) >> 58)];
    #endif
}

// Reflection algorithms from
// https://chessprogramming.wikispaces.com/Flipping+Mirroring+and+Rotating
bitbrd reflectVertical(bitbrd i) {
    #if defined(__x86_64__)
        asm ("bswap %0" : "=r" (i) : "0" (i));
        return i;
    #else
        const bitbrd k1 = 0x00FF00FF00FF00FF;
        const bitbrd k2 = 0x0000FFFF0000FFFF;
        i = ((i >>  8) & k1) | ((i & k1) <<  8);
        i = ((i >> 16) & k2) | ((i & k2) << 16);
        i = ( i >> 32)       | ( i       << 32);
        return i;
    #endif
}

bitbrd reflectHorizontal(bitbrd x) {
    const bitbrd k1 = 0x5555555555555555;
    const bitbrd k2 = 0x3333333333333333;
    const bitbrd k4 = 0x0f0f0f0f0f0f0f0f;
    x = ((x >> 1) & k1) | ((x & k1) << 1);
    x = ((x >> 2) & k2) | ((x & k2) << 2);
    x = ((x >> 4) & k4) | ((x & k4) << 4);
    return x;
}

bitbrd reflectDiag(bitbrd x) {
    bitbrd t;
    const bitbrd k1 = 0x5500550055005500;
    const bitbrd k2 = 0x3333000033330000;
    const bitbrd k4 = 0x0f0f0f0f00000000;
    t  = k4 & (x ^ (x << 28));
    x ^=       t ^ (t >> 28) ;
    t  = k2 & (x ^ (x << 14));
    x ^=       t ^ (t >> 14) ;
    t  = k1 & (x ^ (x <<  7));
    x ^=       t ^ (t >>  7) ;
    return x;
}

// Given a std::chrono time_point, this function returns the number of seconds
// elapsed since then.
double getTimeElapsed(OthelloTime startTime) {
    auto endTime = OthelloClock::now();
    std::chrono::duration<double> timeSpan =
        std::chrono::duration_cast<std::chrono::duration<double>>(
        endTime-startTime);
    return timeSpan.count() + 0.001;
}

// Pretty prints a move in (x, y) form indexed from 1.
void printMove(int move) {
    std::cerr << (char) ('a' + (move & 7)) << (move >> 3) + 1;
}

// Retrieves the next move with the highest score, starting from index using a
// partial selection sort. This way, the entire list does not have to be sorted
// if an early cutoff occurs.
int nextMove(MoveList &moves, MoveList &scores, unsigned int index) {
    if (index >= moves.size)
        return MOVE_NULL;
    // Find the index of the next best move/score
    int bestIndex = index;
    for (unsigned int i = index + 1; i < moves.size; i++) {
        if (scores.get(i) > scores.get(bestIndex))
            bestIndex = i;
    }
    // swap to the correct position
    moves.swap(bestIndex, index);
    scores.swap(bestIndex, index);
    // return the move
    return moves.get(index);
}

// A standard in-place quicksort that sorts moves according to scores.
void sort(MoveList &moves, MoveList &scores, int left, int right) {
    int pivot = (left + right) / 2;

    if (left < right) {
        pivot = partition(moves, scores, left, right, pivot);
        sort(moves, scores, left, pivot-1);
        sort(moves, scores, pivot+1, right);
    }
}

void swap(MoveList &moves, MoveList &scores, int i, int j) {
    int less1 = moves.get(j);
    moves.set(j, moves.get(i));
    moves.set(i, less1);

    int less2 = scores.get(j);
    scores.set(j, scores.get(i));
    scores.set(i, less2);
}

int partition(MoveList &moves, MoveList &scores, int left, int right,
    int pindex) {

    int index = left;
    int pivot = scores.get(pindex);

    swap(moves, scores, pindex, right);

    for (int i = left; i < right; i++) {
        if (scores.get(i) > pivot) {
            swap(moves, scores, i, index);
            index++;
        }
    }
    swap(moves, scores, index, right);

    return index;
}
