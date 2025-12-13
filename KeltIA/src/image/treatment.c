#include "../../include/image/treatment.h"
#include "../../include/image/autorotation.h"
#include "../../include/image/grayscale.h"
#include "../../include/image/removenoise.h"
#include "../../include/image/rotation.h"

// Fonction qui applique tous les traitements (pour le réseau de neurones)
MagickWand *apply_all_treatments(MagickWand *wand, int auto_rotation)
{
    // Sauvegarder temporairement l'image
    char temp_path[] = "/tmp/temp_input.png";
    if (MagickWriteImage(wand, temp_path) == MagickFalse)
    {
        err(1, "Cannot write temp image");
    }

    // Appliquer grayscale et binarisation
    MagickWand *treated = binarize_image_wand(temp_path);
    // Appliquer rotation
    if (auto_rotation) { treated = auto_rotate_image(treated); }
    else
    {
        // Rotation manuelle, angle par défaut 0
        treated = rotate_image(treated, 0.0);
    }

    // Appliquer suppression du bruit
    treated = remove_noise(treated);
    // Supprimer le fichier temp
    remove(temp_path);

    return treated;
}

// Fonction qui applique les traitements choisis (pour l'UI utilisateur)
MagickWand *apply_selected_treatments(
    MagickWand *wand,
    int apply_grayscale,
    int apply_black_and_white,
    int apply_noise,
    int auto_rotation,
    double manual_angle
)
{
    MagickWand *treated = CloneMagickWand(wand);

    // Appliquer grayscale si choisi
    if (apply_grayscale)
    {
        // Sauvegarder temporairement
        char temp_path[] = "/tmp/temp_gray.png";
        if (MagickWriteImage(treated, temp_path) == MagickFalse)
        {
            err(1, "Cannot write temp image for grayscale");
        }
        // Remplacer par la version en grayscale seulement
        DestroyMagickWand(treated);
        treated = grayscale_image_wand(temp_path);
        remove(temp_path);
    }

    // Appliquer binarisation (noir et blanc) si choisi
    if (apply_black_and_white)
    {
        // Sauvegarder temporairement
        char temp_path[] = "/tmp/temp_bw.png";
        if (MagickWriteImage(treated, temp_path) == MagickFalse)
        {
            err(1, "Cannot write temp image for black and white");
        }
        // Remplacer par la version binarisée
        DestroyMagickWand(treated);
        treated = binarize_image_wand(temp_path);
        remove(temp_path);
    }

    // Appliquer rotation
    if (auto_rotation) { treated = auto_rotate_image(treated); }
    else
    {
        treated = rotate_image(treated, manual_angle);
    }

    // Appliquer suppression du bruit si choisi
    if (apply_noise) { treated = remove_noise(treated); }

    return treated;
}

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
)
{
    MagickWandGenesis();

    // Charger l'image originale
    MagickWand *original = NewMagickWand();
    if (MagickReadImage(original, input_path) == MagickFalse)
    {
        err(1, "Cannot load image %s", input_path);
    }

    // Créer copie 1 et appliquer tous les traitements (pour le NN)
    MagickWand *copy1 = CloneMagickWand(original);
    copy1 = apply_all_treatments(copy1, auto_rotation);

    // Sauvegarder l'image pour le NN
    char *nn_path = "/tmp/nn_processed.png";
    if (MagickWriteImage(copy1, nn_path) == MagickFalse)
    {
        err(1, "Cannot save NN image");
    }
    DestroyMagickWand(copy1);

    // Créer copie 2 et appliquer les traitements choisis (pour l'UI)
    MagickWand *copy2 = CloneMagickWand(original);
    copy2 = apply_selected_treatments(
        copy2, apply_grayscale, apply_black_and_white, apply_noise,
        auto_rotation, manual_angle
    );
    // Sauvegarder l'image pour l'UI
    char *ui_path = "/tmp/ui_processed.png";
    if (MagickWriteImage(copy2, ui_path) == MagickFalse)
    {
        err(1, "Cannot save UI image");
    }
    DestroyMagickWand(copy2);

    // Nettoyer
    DestroyMagickWand(original);
    MagickWandTerminus();

    // Retourner les chemins
    ProcessedImages *result = malloc(sizeof(ProcessedImages));
    result->nn_image_path = strdup(nn_path);
    result->ui_image_path = strdup(ui_path);

    return result;
}
