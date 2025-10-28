#pragma once
#include <MagickWand/MagickWand.h>

<<<<<<< HEAD
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
=======
static double compute_average_gray(MagickWand *wand);
MagickBooleanType binarize_image(const char *input_path,
                                 const char *output_path);
>>>>>>> a912520 (1)
