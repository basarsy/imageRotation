# Arbitrary-Angle Image Rotation in C++

## Overview

This project is an efficient multithreaded image rotation tool implemented in C++ using the `libpng` library. It rotates all `.png` images inside a given folder by **110 degrees**.

The project was tested on a real-world dataset consisting of:

- ~8192 `.png` images
- Average size: 60 KB each
- Total data: ~480 MB
- Each image is a 4-channel RGBA image

Two different rotation implementations are provided:
- `v3.cpp`: Fast, loop-based pixel mapping
- `v3rec.cpp`: Slower, recursive pixel rotation method (for academic/comparison purposes)

---

## Features

- ✅ Arbitrary-angle rotation (110 degrees)
- ✅ Multithreaded processing (16 threads by default)
- ✅ Batch-processing support for `.png` images in the `images/` folder
- ✅ Uses the `libpng` library for reading/writing `.png` files
- ✅ Handles RGBA images with full alpha support
- ✅ Automatic memory management and robust error handling

---

## Directory Structure

- v3.cpp # Iterative pixel rotation implementation
- v3rec.cpp # Recursive pixel rotation implementation
- images/ # Folder containing PNG files to be processed

---

## How It Works

### Input

- The program scans the `images/` directory.
- Only files with `.png` extension are processed.

### Processing

- Each image is loaded using `libpng` and converted to RGBA format.
- The center of the image is calculated.
- For every output pixel in the new image (after rotation), the corresponding source pixel is reverse-mapped using rotation matrix math.
- Each rotated image overwrites the original file.

### Output

- Each `.png` file in the `images/` directory is overwritten with its rotated version.

---

## Building

### Requirements

- C++17 or later
- `libpng` development package installed
- Don't forget to multiply the images if you want to test how efficient the script is.

### Compile

```bash
# For iterative (fast) version
g++ -O3 -std=c++17 v3.cpp -lpng -o rotate_iterative

# For recursive (experimental) version
g++ -O3 -std=c++17 v3rec.cpp -lpng -o rotate_recursive

./rotate_iterative
# OR
./rotate_recursive
