#include "../../include/image/treatment.h"
#include "../../include/image/autorotation.h"
#include "../../include/image/grayscale.h"
#include "../../include/image/removenoise.h"
#include "../../include/image/rotation.h"

// Function that applies all treatments (for the neural network)
MagickWand *apply_all_treatments(MagickWand *wand, int auto_rotation)
{
    // Temporarily save the image
    char temp_path[] = "/tmp/temp_input.png";
    if (MagickWriteImage(wand, temp_path) == MagickFalse)
    {
        err(1, "Cannot write temp image");
    }

    // Apply grayscale and binarization
    MagickWand *treated = binarize_image_wand(temp_path);
    // Apply rotation
    if (auto_rotation) { treated = auto_rotate_image(treated); }
    else
    {
        // Manual rotation, default angle 0
        treated = rotate_image(treated, 0.0);
    }

    // Apply noise removal
    treated = remove_noise(treated);
    // Delete the temp file
    remove(temp_path);

    return treated;
}

// Function that applies selected treatments (for the user UI)
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

    // Apply grayscale if selected
    if (apply_grayscale)
    {
        // Temporarily save
        char temp_path[] = "/tmp/temp_gray.png";
        if (MagickWriteImage(treated, temp_path) == MagickFalse)
        {
            err(1, "Cannot write temp image for grayscale");
        }
        // Replace with grayscale version only
        DestroyMagickWand(treated);
        treated = grayscale_image_wand(temp_path);
        remove(temp_path);
    }

    // Apply binarization (black and white) if selected
    if (apply_black_and_white)
    {
        // Temporarily save
        char temp_path[] = "/tmp/temp_bw.png";
        if (MagickWriteImage(treated, temp_path) == MagickFalse)
        {
            err(1, "Cannot write temp image for black and white");
        }
        // Replace with binarized version
        DestroyMagickWand(treated);
        treated = binarize_image_wand(temp_path);
        remove(temp_path);
    }

    // Apply rotation
    if (auto_rotation) { treated = auto_rotate_image(treated); }
    else { treated = rotate_image(treated, manual_angle); }

    // Apply noise removal if selected
    if (apply_noise) { treated = remove_noise(treated); }

    return treated;
}

// Main function: processes the image and returns the paths to the two
// processed images
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

    // Load the original image
    MagickWand *original = NewMagickWand();
    if (MagickReadImage(original, input_path) == MagickFalse)
    {
        err(1, "Cannot load image %s", input_path);
    }

    // Create copy 1 and apply all treatments (for the NN)
    MagickWand *copy1 = CloneMagickWand(original);
    copy1 = apply_all_treatments(copy1, auto_rotation);

    // Save the image for the NN
    char *nn_path = "/tmp/nn_processed.png";
    if (MagickWriteImage(copy1, nn_path) == MagickFalse)
    {
        err(1, "Cannot save NN image");
    }
    DestroyMagickWand(copy1);

    // Create copy 2 and apply selected treatments (for the UI)
    MagickWand *copy2 = CloneMagickWand(original);
    copy2 = apply_selected_treatments(
        copy2, apply_grayscale, apply_black_and_white, apply_noise,
        auto_rotation, manual_angle
    );
    // Save the image for the UI
    char *ui_path = "/tmp/ui_processed.png";
    if (MagickWriteImage(copy2, ui_path) == MagickFalse)
    {
        err(1, "Cannot save UI image");
    }
    DestroyMagickWand(copy2);

    // Clean up
    DestroyMagickWand(original);
    MagickWandTerminus();

    // Return the paths
    ProcessedImages *result = malloc(sizeof(ProcessedImages));
    result->nn_image_path = strdup(nn_path);
    result->ui_image_path = strdup(ui_path);

    return result;
}
