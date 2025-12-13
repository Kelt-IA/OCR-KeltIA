#!/bin/bash

# Directories
INPUT_DIR="ressources/preprocessed_images"
OUTPUT_DIR="ressources/zones_and_letters"

# Create output directory
mkdir -p "$OUTPUT_DIR"

echo "=== Zone and Letter Detection Script ==="
echo "Input directory: $INPUT_DIR"
echo "Output directory: $OUTPUT_DIR"
echo ""

# Check if input directory exists
if [ ! -d "$INPUT_DIR" ]; then
    echo "Error: Input directory $INPUT_DIR does not exist!"
    echo "Please run preprocess_images.sh first."
    exit 1
fi

# Counter for processed images
PROCESSED=0
FAILED=0

# Process each preprocessed image
for input_file in "$INPUT_DIR"/*.png; do
    # Check if any files exist
    if [ ! -e "$input_file" ]; then
        echo "Warning: No .png files found in $INPUT_DIR"
        exit 1
    fi
    
    # Extract filename without path and extension
    filename=$(basename "$input_file")
    base_name="${filename%.png}"
    
    # Create output filename (keep same name)
    output_file="$OUTPUT_DIR/${base_name}.png"
    
    echo "Processing: $filename"
    
    # Run detect_zone_and_letters
    # Adjust executable name if different
    if ./bin/detect_zones_and_letters "$input_file" "$output_file"; then
        echo "  ✓ Success: saved to $output_file"
        ((PROCESSED++))
    else
        echo "  ✗ Failed"
        ((FAILED++))
    fi
    echo ""
done

echo "=== Detection Complete ==="
echo "Successfully processed: $PROCESSED"
echo "Failed: $FAILED"
echo "Output location: $OUTPUT_DIR"

