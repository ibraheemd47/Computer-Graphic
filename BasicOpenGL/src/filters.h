#ifndef FILTERS_H
#define FILTERS_H

#include <vector>
#include <string>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

// A very simple image structure: flat array + dimensions
struct Image {
    int width  = 0;
    int height = 0;
    int channels = 0;          // usually 3 (RGB) or 4 (RGBA)
    std::vector<unsigned char> data;

    // Access helpers
    unsigned char& at(int x, int y, int c) {
        return data[(y * width + x) * channels + c];
    }
    unsigned char at(int x, int y, int c) const {
        return data[(y * width + x) * channels + c];
    }
};

// --------- Load / Save ---------

// Load PNG (or other stb-supported image) from disk into Image
Image loadImage(const std::string& filePath);

// Save Image as PNG file
void saveImage(const Image& img, const std::string& filePath);

// Save first channel of image to a text file:
//   - grayscaleLevels = 2  -> numbers 0..1 (black/white)
//   - grayscaleLevels = 16 -> numbers 0..15 (16 gray levels)
void saveImageAsText(const Image& img,
                     const std::string& filePath,
                     int grayscaleLevels);

// --------- Basic operations ---------

// Convert RGB image to grayscale using luminance formula
// (0.3 R + 0.59 G + 0.11 B)
void toGrayscale(Image& img);

// --------- Assignment filters ---------

// 1. Grayscale (just use toGrayscale + save)

// 2. Canny edge detection (on grayscale image)
// lowRatio, highRatio in [0,1], e.g. 0.1 and 0.9
Image cannyEdges(Image gray,
                 float lowRatio,
                 float highRatio);

// 3. Halftone: each pixel -> 2x2 black/white block
Image halftone(const Image& gray);

// 4. Floyd–Steinberg dithering (16 gray levels) on grayscale
void floydSteinberg16(Image& gray);

#endif // FILTERS_H
