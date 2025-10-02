#!/bin/bash
# Pre-commit hook to format C files using clang-format

# get all staged .c/.h files
files=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(c|h)$')

if [ -z "$files" ]; then
    exit 0
fi

echo "Running clang-format on staged files..."
for file in $files; do
    clang-format -i "$file"    # format in-place
    git add "$file"            # re-stage the formatted file
done

exit 0

