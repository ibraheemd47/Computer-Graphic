#ifndef FILTERS_H
#define FILTERS_H

#include <vector>
#include <string>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

// 3D image type: [row][col][channel]
using Image = std::vector<std::vector<std::vector<unsigned char>>>;

// Load an image from disk into Image (fills width/height/channels)
Image loadImageTo3D(const std::string& filePath,
                    int& width,
                    int& height,
                    int& channels);

// Save an Image as PNG to disk
void save3DImage(const Image& img,
                 const std::string& filePath);

// Save one channel (grayscale) as comma-separated integers
// levels = 2  -> values 0..1 (black/white)
// levels = 16 -> values 0..15 (16 gray levels)
void save3DImageAsText(const Image& img,
                       const std::string& filePath,
                       int levels);

// --------- basic grayscale ---------

// Convert RGB image to grayscale using luminance formula
void makeGrayscale(Image& img,
                   int width,
                   int height);

// --------- Canny edge detection (all stages inside) ---------

Image applyCanny(const Image& grayInput,
                 int width,
                 int height,
                 float lowRatio,
                 float highRatio);

// --------- Halftone (each pixel -> 2x2 block) ---------

Image applyHalftone(const Image& grayInput,
                    int width,
                    int height);

// --------- Floyd–Steinberg (16 gray levels) ---------

void applyFloydSteinberg16(Image& gray,
                           int width,
                           int height);

#endif // FILTERS_H
