#!/bin/bash

# Create output directory
OUTPUT_DIR="ressources/preprocessed_images"
mkdir -p "$OUTPUT_DIR"

echo "=== Image Preprocessing Script ==="
echo "Input directory: ressources/images_test/"
echo "Output directory: $OUTPUT_DIR"
echo ""

# Counter for processed images
PROCESSED=0
FAILED=0

# Process each image in images_test directory
for input_file in ressources/images_test/*.png; do
    # Extract filename without path
    filename=$(basename "$input_file")
    
    # Extract level and image numbers using regex
    # level_1_image_2.png -> lvl-1-2.png
    if [[ $filename =~ level_([0-9]+)_image_([0-9]+)\.png ]]; then
        level_num="${BASH_REMATCH[1]}"
        image_num="${BASH_REMATCH[2]}"
        
        # Create output filename
        output_file="$OUTPUT_DIR/lvl-${level_num}-${image_num}.png"
        
        echo "Processing: $filename -> lvl-${level_num}-${image_num}.png"
        
        # Run preprocessing (adjust command to your executable name)
        # Add -removenoise flag if you want noise reduction
        if ./bin/preprocessing "$input_file" "$output_file"; then
            echo "  ✓ Success"
            ((PROCESSED++))
        else
            echo "  ✗ Failed"
            ((FAILED++))
        fi
        echo ""
    else
        echo "Warning: Skipping $filename (doesn't match expected format)"
        echo ""
    fi
done

echo "=== Processing Complete ==="
echo "Successfully processed: $PROCESSED"
echo "Failed: $FAILED"
echo "Output location: $OUTPUT_DIR"

