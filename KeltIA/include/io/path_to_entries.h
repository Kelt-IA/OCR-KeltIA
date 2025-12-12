#pragma once

#include "../../include/nn/accuracy_metrics.h"
#include "parse_csv.h"
#include <stdlib.h>

Dataset *csv_to_dataset(
    CSV *csv_data,
    const char *base_path,
    size_t target_width,
    size_t target_height
);

double *read_image_pixels(
    const char *image_path,
    size_t *pixels_count,
    size_t target_width,
    size_t target_height
);
