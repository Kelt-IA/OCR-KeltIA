#include "../../include/image/autorotation.h"
#include "../../include/image/rotation.h"
#include <MagickWand/MagickWand.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/*
  Detects the optimal rotation angle by analyzing image contours
  Uses edge detection to find the best alignment
  return a rotation angle in degrees (positive: clockwise)
 */

/*
static double detect_rotation_angle_from_edges(MagickWand *wand)
{
   MagickWand *edge_wand = NULL;
   MagickWand *test_wand = NULL;
   double best_angle = 0.0;
   double best_variance = 0.0;
   double angle_step = 1.0;

   edge_wand = CloneMagickWand(wand);
   if (edge_wand == NULL)
   {
       fprintf(stderr, "Error: cannot clone wand for edge detection.\n");
       return 0.0;
   }

   if (MagickEdgeImage(edge_wand, 2.0) == MagickFalse)
   {
       fprintf(stderr, "Warning: edge detection failed.\n");
       edge_wand = DestroyMagickWand(edge_wand);
       return 0.0;
   }

   for (double angle = -45.0; angle <= 45.0; angle += angle_step)
   {
       test_wand = CloneMagickWand(edge_wand);
       if (test_wand == NULL)
           continue;

       PixelWand *background = NewPixelWand();
       PixelSetColor(background, "black");

       MagickRotateImage(test_wand, background, angle);

       double variance = 0.0;
       unsigned long width = MagickGetImageWidth(test_wand);
       unsigned long height = MagickGetImageHeight(test_wand);
       unsigned long pixel_count = width * height;

       PixelWand *pw = NewPixelWand();
       if (pw == NULL)
       {
           test_wand = DestroyMagickWand(test_wand);
           background = DestroyPixelWand(background);
           continue;
       }

       for (unsigned long y = 0; y < height; ++y)
       {
           for (unsigned long x = 0; x < width; ++x)
           {
               if (MagickGetImagePixelColor(test_wand, x, y, pw) == MagickFalse)
                   continue;
               double intensity = PixelGetRed(pw);
               variance += intensity * intensity;
           }
       }

       if (pixel_count > 0)
           variance /= pixel_count;

       if (variance > best_variance)
       {
           best_variance = variance;
           best_angle = angle;
       }

       pw = DestroyPixelWand(pw);
       test_wand = DestroyMagickWand(test_wand);
       background = DestroyPixelWand(background);
   }

   edge_wand = DestroyMagickWand(edge_wand);

   return best_angle;
}
*/
/*
    Detects rotation angle using horizontal projection analysis
    Optimized with downsampling, coarse-to-fine search, and fast pixel export
    return a rotation angle in degrees
 */
static double detect_rotation_angle_projection(MagickWand *wand)
{
    MagickWand *small = NULL;
    MagickBooleanType status;
    double best_angle = 0.0;

    const size_t MAX_DIM = 400;

    small = CloneMagickWand(wand);
    if (small == NULL) return 0.0;

    status = MagickSetImageColorspace(small, GRAYColorspace);
    (void)status;

    unsigned long iw = MagickGetImageWidth(small);
    unsigned long ih = MagickGetImageHeight(small);
    double scale = 1.0;
    if (iw > ih)
    {
        if (iw > MAX_DIM) scale = (double)MAX_DIM / (double)iw;
    }
    else
    {
        if (ih > MAX_DIM) scale = (double)MAX_DIM / (double)ih;
    }

    if (scale < 1.0)
    {
        unsigned long new_w = (unsigned long)(iw * scale);
        unsigned long new_h = (unsigned long)(ih * scale);
        MagickResizeImage(small, new_w, new_h, LanczosFilter);
    }

    // unsigned long sw = MagickGetImageWidth(small);
    // unsigned long sh = MagickGetImageHeight(small);

    double coarse_range = 30.0;
    double coarse_step = 2.0;
    double fine_step = 0.25;

    double best_score = -1.0;
    double initial_best_score = -1.0;
    double initial_best_angle = 0.0;
    printf("%f", initial_best_angle);
    for (double angle = -coarse_range; angle <= coarse_range;
         angle += coarse_step)
    {
        MagickWand *rot = CloneMagickWand(small);
        if (rot == NULL) continue;

        PixelWand *bg = NewPixelWand();
        PixelSetColor(bg, "white");
        MagickRotateImage(rot, bg, angle);

        size_t npixels = (size_t)MagickGetImageWidth(rot) *
                         (size_t)MagickGetImageHeight(rot);
        double *buf = (double *)malloc(sizeof(double) * npixels);
        if (buf == NULL)
        {
            rot = DestroyMagickWand(rot);
            bg = DestroyPixelWand(bg);
            continue;
        }

        status = MagickExportImagePixels(
            rot, 0, 0, MagickGetImageWidth(rot), MagickGetImageHeight(rot), "R",
            DoublePixel, buf
        );
        if (status == MagickFalse)
        {
            free(buf);
            rot = DestroyMagickWand(rot);
            bg = DestroyPixelWand(bg);
            continue;
        }

        double prev_line = 0.0;
        double score = 0.0;
        unsigned long rw = MagickGetImageWidth(rot);
        unsigned long rh = MagickGetImageHeight(rot);
        for (unsigned long y = 0; y < rh; ++y)
        {
            double line_sum = 0.0;
            size_t row_start = (size_t)y * (size_t)rw;
            for (unsigned long x = 0; x < rw; ++x)
            {
                double v = buf[row_start + x];
                line_sum += (1.0 - v);
            }
            if (y > 0) { score += fabs(line_sum - prev_line); }
            prev_line = line_sum;
        }

        free(buf);
        rot = DestroyMagickWand(rot);
        bg = DestroyPixelWand(bg);

        if (score > best_score)
        {
            best_score = score;
            best_angle = angle;
        }
    }
    initial_best_score = best_score;
    initial_best_angle = best_angle;

    double score_threshold = 100.0;
    if (initial_best_score < score_threshold)
    {
        fprintf(
            stderr,
            "Warning: weak initial score (%.1f), attempting wider search...\n",
            initial_best_score
        );
        best_angle = 0.0;
        best_score = -1.0;
        for (double angle = -45.0; angle <= 45.0; angle += coarse_step)
        {
            MagickWand *rot = CloneMagickWand(small);
            if (rot == NULL) continue;

            PixelWand *bg = NewPixelWand();
            PixelSetColor(bg, "white");
            MagickRotateImage(rot, bg, angle);

            size_t npixels = (size_t)MagickGetImageWidth(rot) *
                             (size_t)MagickGetImageHeight(rot);
            double *buf = (double *)malloc(sizeof(double) * npixels);
            if (buf == NULL)
            {
                rot = DestroyMagickWand(rot);
                bg = DestroyPixelWand(bg);
                continue;
            }

            status = MagickExportImagePixels(
                rot, 0, 0, MagickGetImageWidth(rot), MagickGetImageHeight(rot),
                "R", DoublePixel, buf
            );
            if (status == MagickFalse)
            {
                free(buf);
                rot = DestroyMagickWand(rot);
                bg = DestroyPixelWand(bg);
                continue;
            }

            double prev_line = 0.0;
            double score = 0.0;
            unsigned long rw = MagickGetImageWidth(rot);
            unsigned long rh = MagickGetImageHeight(rot);
            for (unsigned long y = 0; y < rh; ++y)
            {
                double line_sum = 0.0;
                size_t row_start = (size_t)y * (size_t)rw;
                for (unsigned long x = 0; x < rw; ++x)
                {
                    double v = buf[row_start + x];
                    line_sum += (1.0 - v);
                }
                if (y > 0) score += fabs(line_sum - prev_line);
                prev_line = line_sum;
            }

            free(buf);
            rot = DestroyMagickWand(rot);
            bg = DestroyPixelWand(bg);

            if (score > best_score)
            {
                best_score = score;
                best_angle = angle;
            }
        }
    }

    double refine_range = coarse_step * 2.0;
    double refine_start = best_angle - refine_range;
    double refine_end = best_angle + refine_range;

    for (double angle = refine_start; angle <= refine_end; angle += fine_step)
    {
        MagickWand *rot = CloneMagickWand(small);
        if (rot == NULL) continue;

        PixelWand *bg = NewPixelWand();
        PixelSetColor(bg, "white");
        MagickRotateImage(rot, bg, angle);

        size_t npixels = (size_t)MagickGetImageWidth(rot) *
                         (size_t)MagickGetImageHeight(rot);
        double *buf = (double *)malloc(sizeof(double) * npixels);
        if (buf == NULL)
        {
            rot = DestroyMagickWand(rot);
            bg = DestroyPixelWand(bg);
            continue;
        }

        status = MagickExportImagePixels(
            rot, 0, 0, MagickGetImageWidth(rot), MagickGetImageHeight(rot), "R",
            DoublePixel, buf
        );
        if (status == MagickFalse)
        {
            free(buf);
            rot = DestroyMagickWand(rot);
            bg = DestroyPixelWand(bg);
            continue;
        }

        double prev_line = 0.0;
        double score = 0.0;
        unsigned long rw = MagickGetImageWidth(rot);
        unsigned long rh = MagickGetImageHeight(rot);
        for (unsigned long y = 0; y < rh; ++y)
        {
            double line_sum = 0.0;
            size_t row_start = (size_t)y * (size_t)rw;
            for (unsigned long x = 0; x < rw; ++x)
            {
                double v = buf[row_start + x];
                line_sum += (1.0 - v);
            }
            if (y > 0) score += fabs(line_sum - prev_line);
            prev_line = line_sum;
        }

        free(buf);
        rot = DestroyMagickWand(rot);
        bg = DestroyPixelWand(bg);

        if (score > best_score)
        {
            best_score = score;
            best_angle = angle;
        }
    }

    small = DestroyMagickWand(small);
    return best_angle;
}

/*
    Automatically straightens an image by detecting and correcting rotation
   angle OCR preprocessing without character detection return a MagickWand of
   the straightened image or NULL on error
 */
MagickWand *auto_rotate_image(MagickWand *input_wand)
{
    if (input_wand == NULL)
    {
        fprintf(stderr, "Error: input_wand is NULL.\n");
        return NULL;
    }

    double rotation_angle = detect_rotation_angle_projection(input_wand);

    fprintf(
        stderr, "Auto-rotation: detected angle = %.2f degrees\n", rotation_angle
    );

    MagickWand *rotated_wand = rotate_image(input_wand, rotation_angle);

    return rotated_wand;
}

/**
    Straightens an image with an alternative brute-force strategy
    Useful for complex or heavily misaligned cases
    return a MagickWand of the straightened image or NULL on error
 */
MagickWand *
auto_rotate_image_bruteforce(MagickWand *input_wand, double angle_step)
{
    if (input_wand == NULL)
    {
        fprintf(stderr, "Error: input_wand is NULL.\n");
        return NULL;
    }

    if (angle_step <= 0.0)
    {
        fprintf(stderr, "Error: angle_step must be positive.\n");
        return NULL;
    }

    double rotation_angle = detect_rotation_angle_projection(input_wand);

    fprintf(
        stderr, "Auto-rotation (bruteforce): detected angle = %.2f degrees\n",
        rotation_angle
    );

    MagickWand *rotated_wand = rotate_image(input_wand, rotation_angle);

    return rotated_wand;
}
