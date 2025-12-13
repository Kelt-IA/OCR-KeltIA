#pragma once
#include <MagickWand/MagickWand.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure pour retourner les deux chemins d'images
typedef struct
{
    char *nn_image_path;
    char *ui_image_path;
} ProcessedImages;

// Fonction principale : traite l'image et retourne les chemins vers les deux
// images traitées
ProcessedImages *process_image(
    const char *input_path,
    int apply_grayscale,
    int apply_black_and_white,
    int apply_noise,
    int auto_rotation,
    double manual_angle
);

MagickWand *apply_all_treatments(MagickWand *wand, int auto_rotation);
MagickWand *apply_selected_treatments(
    MagickWand *wand,
    int apply_grayscale,
    int apply_black_and_white,
    int apply_noise,
    int auto_rotation,
    double manual_angle
);
