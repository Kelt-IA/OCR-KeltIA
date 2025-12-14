#include "../../include/window/utils.h"

MagickBooleanType is_supported_by_magickwand(char *path)
{
    MagickWand *wand;
    MagickBooleanType status;

    // MagickWandGenesis();

    wand = NewMagickWand();

    status = MagickPingImage(wand, path);

    if (wand) DestroyMagickWand(wand);

    // MagickWandTerminus();
    return status;
}

char *copy_to_temp_file_path(const char *source_path)
{
    FILE *source_fp, *temp_fp;
    char temp_name[L_tmpnam];
    char *result_path = NULL;
    char buffer[BUFSIZ];
    size_t bytes_read;

    if (tmpnam(temp_name) == NULL)
    {
        perror("tmpnam failed");
        return NULL;
    }

    source_fp = fopen(source_path, "rb");
    if (source_fp == NULL)
    {
        perror("Failed to open source file");
        return NULL;
    }

    // Create temp file in binary write mode
    temp_fp = fopen(temp_name, "wb");
    if (temp_fp == NULL)
    {
        perror("Failed to create temp file");
        fclose(source_fp);
        return NULL;
    }

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), source_fp)) > 0)
    {
        if (fwrite(buffer, 1, bytes_read, temp_fp) != bytes_read)
        {
            perror("Write error");
            fclose(source_fp);
            fclose(temp_fp);
            remove(temp_name);
            return NULL;
        }
    }

    if (ferror(source_fp))
    {
        perror("Read error");
        fclose(source_fp);
        fclose(temp_fp);
        remove(temp_name);
        return NULL;
    }

    fclose(source_fp);
    fclose(temp_fp);

    result_path = strdup(temp_name);
    if (result_path == NULL)
    {
        perror("strdup failed");
        remove(temp_name);
        return NULL;
    }

    // printf("File copied to temp: %s\n", result_path);
    return result_path;
}

int copy_file(const char *source, const char *dest)
{
    FILE *src = fopen(source, "rb");
    FILE *dst = fopen(dest, "wb");

    if (!src || !dst)
    {
        if (src) fclose(src);
        if (dst) fclose(dst);
        return -1;
    }

    unsigned char buffer[4096];
    size_t bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0)
    {
        fwrite(buffer, 1, bytes, dst);
    }

    fclose(src);
    fclose(dst);
    return 0;
}
