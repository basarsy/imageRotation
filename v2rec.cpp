

#include <iostream>
#include <vector>
#include <png.h>
#include <filesystem>
#include <string>
#include <thread>

namespace fs = std::filesystem;

// Function to read a PNG file and store its data in a 3D vector
void read_png_file(const char* filename, std::vector<std::vector<std::vector<unsigned char>>>& image_data, int& width, int& height) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        std::cerr << "Error: Could not open file " << filename << " for reading." << std::endl;
        exit(1);
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        std::cerr << "Error: png_create_read_struct failed." << std::endl;
        fclose(fp);  // Added
        exit(1);
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        std::cerr << "Error: png_create_info_struct failed." << std::endl;
        png_destroy_read_struct(&png, NULL, NULL);  // Added
        fclose(fp);  // Added
        exit(1);
    }

    if (setjmp(png_jmpbuf(png))) {
        std::cerr << "Error during init_io." << std::endl;
        png_destroy_read_struct(&png, &info, NULL);  // Added
        fclose(fp);  // Added
        exit(1);
    }

    png_init_io(png, fp);
    png_read_info(png, info);

    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);

    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    // Adjust PNG settings to ensure 8-bit RGBA format
    if (bit_depth == 16)
        png_set_strip_16(png);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    // Error handling for reading the image
    if (setjmp(png_jmpbuf(png))) {
        std::cerr << "Error during read_image." << std::endl;
        png_destroy_read_struct(&png, &info, NULL);  // Added
        fclose(fp);  // Added
        exit(1);
    }


    // Read the PNG file into row pointers
    std::vector<png_bytep> row_pointers(height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_bytep)malloc(png_get_rowbytes(png, info));
        if (!row_pointers[y]) {
            std::cerr << "Error: Could not allocate memory for row pointers." << std::endl;
            for (int j = 0; j < y; j++) {
                free(row_pointers[j]);
            }
            png_destroy_read_struct(&png, &info, NULL);  // Added
            fclose(fp);  // Added
            exit(1);
        }
    }
    png_read_image(png, row_pointers.data());



    fclose(fp);

    // Store image data in a 3D vector (height x width x RGBA)
    image_data.resize(height, std::vector<std::vector<unsigned char>>(width, std::vector<unsigned char>(4)));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            png_bytep px = &(row_pointers[y][x * 4]);
            image_data[y][x][0] = px[0];
            image_data[y][x][1] = px[1];
            image_data[y][x][2] = px[2];
            image_data[y][x][3] = px[3];
        }
        free(row_pointers[y]);
    }

    png_destroy_read_struct(&png, &info, NULL);  // Added
}

// Recursive function to rotate the image
void rotateImageRecursive(const std::vector<std::vector<std::vector<unsigned char>>>& image_data,std::vector<std::vector<std::vector<unsigned char>>>& rotated_image, int i, int j, int rows, int cols) {
    if (i == rows) {
        return;
    }
    if (j < cols) {
        // Rotate the current pixel and place it in the correct position in the rotated image
        rotated_image[j][rows - 1 - i] = image_data[i][j];
        // Move to the next column and continue the recursion
        rotateImageRecursive(image_data, rotated_image, i, j + 1, rows, cols);
    } else {
        // If all columns in the current row are processed, move to the next row
        // Reset the column index to 0 and continue the recursion
        rotateImageRecursive(image_data, rotated_image, i + 1, 0, rows, cols);
    }
}

// Function to rotate the image using recursion,
std::vector<std::vector<std::vector<unsigned char>>> rotate_image(const std::vector<std::vector<std::vector<unsigned char>>>& image_data, int& width, int& height) {
    // Create a matrix to store the rotated image
    std::vector<std::vector<std::vector<unsigned char>>> rotated_image(width, std::vector<std::vector<unsigned char>>(height, std::vector<unsigned char>(4)));
    // Call the recursive function to rotate the image
    rotateImageRecursive(image_data, rotated_image, 0, 0, height, width);
    // Return the rotated image
    std::swap(width, height);
    return rotated_image;
}

// Function to write the image data to a PNG file
void write_png_file(const char* filename, const std::vector<std::vector<std::vector<unsigned char>>>& image_data, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        exit(1);
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        std::cerr << "Error: png_create_write_struct failed." << std::endl;
        fclose(fp);  // Added
        exit(1);
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        std::cerr << "Error: png_create_info_struct failed." << std::endl;
        png_destroy_write_struct(&png, NULL);  // Added
        fclose(fp);  // Added
        exit(1);
    }

    if (setjmp(png_jmpbuf(png))) {
        std::cerr << "Error during init_io." << std::endl;
        png_destroy_write_struct(&png, &info);  // Added
        fclose(fp);  // Added
        exit(1);
    }

    png_init_io(png, fp);

    // Error handling for writing header
    if (setjmp(png_jmpbuf(png))) {
        std::cerr << "Error during writing header." << std::endl;
        png_destroy_write_struct(&png, &info);  // Added
        fclose(fp);  // Added
        exit(1);
    }

    // Set the image header information
    png_set_IHDR(
        png,
        info,
        width, height,
        8,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    if (setjmp(png_jmpbuf(png))) {
        std::cerr << "Error during writing bytes." << std::endl;
        png_destroy_write_struct(&png, &info);  // Added
        fclose(fp);  // Added
        exit(1);
    }

    // Prepare row pointers and write the image data
    std::vector<png_bytep> row_pointers(height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_bytep)malloc(png_get_rowbytes(png, info));
        if (!row_pointers[y]) {
            std::cerr << "Error: Could not allocate memory for row pointers." << std::endl;
            for (int j = 0; j < y; j++) {
                free(row_pointers[j]);
            }
            png_destroy_write_struct(&png, &info);  // Added
            fclose(fp);  // Added
            exit(1);
        }
        for (int x = 0; x < width; x++) {
            png_bytep px = &(row_pointers[y][x * 4]);
            px[0] = image_data[y][x][0];
            px[1] = image_data[y][x][1];
            px[2] = image_data[y][x][2];
            px[3] = image_data[y][x][3];
        }
    }
    png_write_image(png, row_pointers.data());

    if (setjmp(png_jmpbuf(png))) {
        std::cerr << "Error during end of write." << std::endl;
        png_destroy_write_struct(&png, &info);  // Added
        for (int y = 0; y < height; y++) {
            free(row_pointers[y]);
        }
        fclose(fp);  // Added
        exit(1);
    }

    png_write_end(png, NULL);

    for (int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }

    fclose(fp);
    png_destroy_write_struct(&png, &info);  // Added
}

// Function to process each image
void process_image(const fs::path& image_path) {
    std::vector<std::vector<std::vector<unsigned char>>> image_data;
    int width, height;

    try {
        read_png_file(image_path.c_str(), image_data, width, height);  // Read image
        auto rotated_image = rotate_image(image_data, width, height);  // Rotate image
        write_png_file(image_path.c_str(), rotated_image, width, height);  // Write rotated image
    } catch (const std::exception& e) {
        std::cerr << "Error processing file " << image_path << ": " << e.what() << std::endl;
    }
}

// Main function
int main() {
    const std::string input_folder = "images";  // Input folder containing images

    // Collect all image paths
    std::vector<fs::path> image_paths;
    for (const auto& entry : fs::directory_iterator(input_folder)) {
        if (entry.path().extension() == ".png") {
            image_paths.push_back(entry.path());
        }
    }

    const int numThreads = 16;  // Number of threads to use
    std::vector<std::thread> threads(numThreads);

    // Divide the image paths among threads
    size_t numImages = image_paths.size();
    size_t imagesPerThread = numImages / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        size_t startIdx = i * imagesPerThread;
        size_t endIdx = (i == numThreads - 1) ? numImages : (i + 1) * imagesPerThread;

        // Assign a thread to process a subset of images
        threads[i] = std::thread([&, startIdx, endIdx]() {
            // Iterate over the subset of images assigned to this thread
            for (size_t j = startIdx; j < endIdx; ++j) {
                // Process the image
                process_image(image_paths[j]);
            }
        });
    }

    // Join threads
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    return 0;
}
