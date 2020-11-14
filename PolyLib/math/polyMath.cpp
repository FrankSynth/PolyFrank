#include "polyMath.hpp"

float fast_sin_f32(float x) {
    float sinVal, fract; /* Temporary variables for input, output */
    uint16_t index;      /* Index variable */
    float a, b;          /* Two nearest output values */
    int32_t n;
    float findex;

    /* Special case for small negative inputs */
    if ((x < 0.0f) && (x >= -1.9e-7f)) {
        return x;
    }

    /* input x is in radians */
    /* Scale the input to [0 1] range from [0 2*PI] , divide input by 2*pi */
    // in = x * 0.159154943092f;

    /* Calculation of floor value of input */
    n = (int32_t)x;

    /* Make negative values towards -infinity */
    if (x < 0.0f) {
        n--;
    }

    /* Map input value to [0 1] */
    x = x - (float)n;

    /* Calculation of index of the table */
    findex = (float)FAST_MATH_TABLE_SIZE * x;

    index = ((uint16_t)findex) & 0x1ff;

    /* fractional value calculation */
    fract = findex - (float)index;

    /* Read two nearest values of input value from the sin table */
    a = sinTable_f32[index];
    b = sinTable_f32[index + 1];

    /* Linear interpolation process */
    // sinVal = (1.0f - fract) * a + fract * b;
    sinVal = fast_lerp_f32(a, b, fract);

    /* Return the output value */
    return (sinVal);
}

inline void fast_copy_f32(float *pSrc, float *pDst, __uint32_t blockSize) {
    uint32_t blkCnt; /* loop counter */

    /* Run the below code for Cortex-M4 and Cortex-M3 */
    float in1, in2, in3, in4;

    /*loop Unrolling */
    blkCnt = blockSize >> 2U;

    /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
     ** a second loop below computes the remaining 1 to 3 samples. */
    while (blkCnt > 0U) {
        /* C = A */
        /* Copy and then store the results in the destination buffer */
        in1 = *pSrc++;
        in2 = *pSrc++;
        in3 = *pSrc++;
        in4 = *pSrc++;

        *pDst++ = in1;
        *pDst++ = in2;
        *pDst++ = in3;
        *pDst++ = in4;

        /* Decrement the loop counter */
        blkCnt--;
    }

    /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
     ** No loop unrolling is used. */
    blkCnt = blockSize % 0x4U;

    while (blkCnt > 0U) {
        /* C = A */
        /* Copy and then store the results in the destination buffer */
        *pDst++ = *pSrc++;

        /* Decrement the loop counter */
        blkCnt--;
    }
}

void precomputeLin2LogTable() {
    for (uint16_t i = 0; i < FAST_LIN2LOG_TABLE_SIZE + 1; i++) {
    }
}