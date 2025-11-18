#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include "../../include/upscale/upscale.h"

void upscale(int **M, int h, int w, int result[CONST_SIZE][CONST_SIZE])
{
    /*
    Params :
        - M : Matrix of pixels (0 = black and 1 = white).
        - h : Number of rows of M
        - w : Number of columns of M
        - result : Matrix of size CONST_SIZE*CONST_SIZE after M has been upscaled.
    */
    if (h > CONST_SIZE || w > CONST_SIZE)
        errx(1, "Error : M is bigger than the expected upscaled matrix !");

    double scale_y = (double)h / CONST_SIZE;
    double scale_x = (double)w / CONST_SIZE;

    for (int Y = 0; Y < CONST_SIZE; Y++)
    {
        for (int X = 0; X < CONST_SIZE; X++)
        {
            int y = (int)(Y * scale_y);
            int x = (int)(X * scale_x);

            if (y >= h)
                y = h - 1;
            if (x >= w)
                x = w - 1;

            result[Y][X] = M[y][x];
        }
    }
}