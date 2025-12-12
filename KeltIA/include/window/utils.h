#pragma once
#include <MagickWand/MagickWand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MagickBooleanType is_supported_by_magickwand(char *path);
char *copy_to_temp_file_path(const char *source_path);
