#ifndef GRAYSCALE_H
#define GRAYSCALE_H

#include <MagickWand/MagickWand.h>

#ifdef __cplusplus
extern "C"
{
#endif

    int grayscale_threshold_file(
        const char *in_path,
        const char *out_path,
        double threshold
    );
    MagickBooleanType
    grayscale_threshold_wand(MagickWand *wand, double threshold);

#ifdef __cplusplus
}
#endif

#endif