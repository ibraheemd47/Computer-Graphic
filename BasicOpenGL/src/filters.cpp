#include "filters.h"
#include <math.h>

int width, height, channels;

template <typename T>
T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}
std::vector<std::vector<std::vector<unsigned char>>> LoadImageToArray(const std::string& filePath) {
    // Load the image into a buffer
    unsigned char* buffer = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    if (buffer == nullptr) {
        std::cerr << "Error: Unable to load image." << std::endl;
        exit(1);
    }

    // Create a 3D vector to store the image data
    std::vector<std::vector<std::vector<unsigned char>>> imageArray(height, 
        std::vector<std::vector<unsigned char>>(width, std::vector<unsigned char>(channels)));

    // Copy the image data into the 3D vector
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * channels;
            for (int c = 0; c < channels; ++c) {
                imageArray[y][x][c] = buffer[index + c];
            }
        }
    }

    // Free the loaded image buffer since it's no longer needed
    stbi_image_free(buffer);

    return imageArray;
}

void ConvertToGrayscale(std::vector<std::vector<std::vector<unsigned char>>>& imageArray) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char r = imageArray[y][x][0];
            unsigned char g = imageArray[y][x][1];
            unsigned char b = imageArray[y][x][2];

            // Convert to grayscale using the luminance formula
            unsigned char gray = static_cast<unsigned char>(0.3 * r + 0.59 * g + 0.11 * b);

            // Set the RGB channels to the grayscale value
            imageArray[y][x][0] = gray;
            imageArray[y][x][1] = gray;
            imageArray[y][x][2] = gray;
        }
    }
}


void ConvertToGrayAveragescale(std::vector<std::vector<std::vector<unsigned char>>>& imageArray) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char r = imageArray[y][x][0];
            unsigned char g = imageArray[y][x][1];
            unsigned char b = imageArray[y][x][2];

            // Convert to grayscale using the luminance formula
            unsigned char gray = static_cast<unsigned char>( (r + g + b) / 3  );

            // Set the RGB channels to the grayscale value
            imageArray[y][x][0] = gray;
            imageArray[y][x][1] = gray;
            imageArray[y][x][2] = gray;
        }
    }
}



void SaveImage(const std::vector<std::vector<std::vector<unsigned char>>>& imageArray, const std::string& imageName, const std::string& outputDirectory) {
    // Construct the full file path
    std::string filePath = outputDirectory + imageName;

    // Derive image dimensions from the input array
    int height = imageArray.size();                     // Number of rows
    int width = imageArray[0].size();                   // Number of columns
    int channels = imageArray[0][0].size();             // Number of color channels (e.g., 3 for RGB)

    // Create a buffer to hold the image data in a format suitable for saving
    unsigned char* buffer = new unsigned char[width * height * channels];

    // Copy image data to the buffer
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * channels;
            for (int c = 0; c < channels; ++c) {
                buffer[index + c] = imageArray[y][x][c];
            }
        }
    }

    // Save the image as a PNG file
    int result = stbi_write_png(filePath.c_str(), width, height, channels, buffer, width * channels);
    if (result) {
        std::cout << "Image saved successfully to " << filePath << std::endl;
    } else {
        std::cerr << "Failed to save the image to " << filePath << std::endl;
    }

    // Free the buffer after saving
    delete[] buffer;
}


void ApplyGaussianFilter3x3(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, int width, int height) {
    // Define a 3x3 Gaussian kernel (with sigma = 1.0 as an example)
    float kernel[3][3] = {
        {1/16.0, 2/16.0, 1/16.0},
        {2/16.0, 4/16.0, 2/16.0},
        {1/16.0, 2/16.0, 1/16.0}
    };

    // Create a new image array for the output image
    std::vector<std::vector<std::vector<unsigned char>>> outputImage(height, std::vector<std::vector<unsigned char>>(width, std::vector<unsigned char>(3)));

    // Apply convolution with the Gaussian kernel
    for (int y = 1; y < height - 1; ++y) {  // Avoid edges for simplicity
        for (int x = 1; x < width - 1; ++x) {
            float newValueR = 0.0f, newValueG = 0.0f, newValueB = 0.0f;

            // Apply the kernel to each pixel in the neighborhood
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int pixelX = x + kx;
                    int pixelY = y + ky;

                    // Get the RGB values from the original image
                    unsigned char r = imageArray[pixelY][pixelX][0];  // Red
                    unsigned char g = imageArray[pixelY][pixelX][1];  // Green
                    unsigned char b = imageArray[pixelY][pixelX][2];  // Blue

                    // Multiply the kernel value with the pixel values and accumulate
                    newValueR += kernel[ky + 1][kx + 1] * r;
                    newValueG += kernel[ky + 1][kx + 1] * g;
                    newValueB += kernel[ky + 1][kx + 1] * b;
                }
            }

            // Store the new pixel values in the output image
            outputImage[y][x][0] = static_cast<unsigned char>(std::min(std::max(newValueR, 0.0f), 255.0f));
            outputImage[y][x][1] = static_cast<unsigned char>(std::min(std::max(newValueG, 0.0f), 255.0f));
            outputImage[y][x][2] = static_cast<unsigned char>(std::min(std::max(newValueB, 0.0f), 255.0f));
        }
    }

    // Copy the processed image back to the original (or save as new image)
    imageArray = outputImage;
}


void ApplyGaussianFilter5x5(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, int width, int height) {
    // Define a 5x5 Gaussian kernel (with sigma = 1.0 as an example)
    float kernel[5][5] = {
        {1/273.0f, 4/273.0f,  6/273.0f, 4/273.0f, 1/273.0f},
        {4/273.0f, 16/273.0f, 24/273.0f, 16/273.0f, 4/273.0f},
        {6/273.0f, 24/273.0f, 36/273.0f, 24/273.0f, 6/273.0f},
        {4/273.0f, 16/273.0f, 24/273.0f, 16/273.0f, 4/273.0f},
        {1/273.0f, 4/273.0f,  6/273.0f, 4/273.0f, 1/273.0f}
    };

    // Create a new image array for the output image
    std::vector<std::vector<std::vector<unsigned char>>> outputImage(height, std::vector<std::vector<unsigned char>>(width, std::vector<unsigned char>(3)));

    // Apply convolution with the Gaussian kernel
    for (int y = 2; y < height - 2; ++y) {  // Avoid edges (2 pixels from each edge for the 5x5 filter)
        for (int x = 2; x < width - 2; ++x) {
            float newValueR = 0.0f, newValueG = 0.0f, newValueB = 0.0f;

            // Apply the kernel to each pixel in the 5x5 neighborhood
            for (int ky = -2; ky <= 2; ++ky) {
                for (int kx = -2; kx <= 2; ++kx) {
                    int pixelX = x + kx;
                    int pixelY = y + ky;

                    // Get the RGB values from the original image
                    unsigned char r = imageArray[pixelY][pixelX][0];  // Red
                    unsigned char g = imageArray[pixelY][pixelX][1];  // Green
                    unsigned char b = imageArray[pixelY][pixelX][2];  // Blue

                    // Multiply the kernel value with the pixel values and accumulate
                    newValueR += kernel[ky + 2][kx + 2] * r;
                    newValueG += kernel[ky + 2][kx + 2] * g;
                    newValueB += kernel[ky + 2][kx + 2] * b;
                }
            }

            // Store the new pixel values in the output image
            outputImage[y][x][0] = static_cast<unsigned char>(std::min(std::max(newValueR, 0.0f), 255.0f));
            outputImage[y][x][1] = static_cast<unsigned char>(std::min(std::max(newValueG, 0.0f), 255.0f));
            outputImage[y][x][2] = static_cast<unsigned char>(std::min(std::max(newValueB, 0.0f), 255.0f));
        }
    }

    // Copy the processed image back to the original (or save as new image)
    imageArray = outputImage;
}




void GradientCalculation(std::vector<std::vector<std::vector<unsigned char>>>& imageArray,  
                         int width, int height) {
    // Create an output image for storing the gradients
    std::vector<std::vector<std::vector<unsigned char>>> outputImage( 
        height,  std::vector<std::vector<unsigned char>>(width, std::vector<unsigned char>(3, 0)));
        
        
  

    // Kernel for x-gradient (Sobel)
    float kernelX[3][3] = {
        { 1,  0, -1},
        { 2,  0, -2},
        { 1,  0, -1}
    };

    // Kernel for y-gradient (Sobel)
    float kernelY[3][3] = {
        { 1,  2,  1},
        { 0,  0,  0},
        {-1, -2, -1}
    };

    // Loop through the image pixels (excluding the borders)
    for (int y = 1; y < height - 1; ++y) {  
        for (int x = 1; x < width - 1; ++x) {
            float sumX = 0.0f;
            float sumY = 0.0f;

            // Apply kernelX and kernelY
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    sumX += imageArray[y + ky][x + kx][0] * kernelX[ky + 1][kx + 1];
                    sumY += imageArray[y + ky][x + kx][0] * kernelY[ky + 1][kx + 1];
                }
            }

            // Calculate gradient magnitude
            float magnitude = std::sqrt((sumX * sumX) + (sumY * sumY));

            // Clamp the value to [0, 255]
            magnitude = std::min(255.0f, magnitude);

            // Assign the computed magnitude to all color channels
            outputImage[y][x][0] = static_cast<unsigned char>(magnitude);  // Red
            outputImage[y][x][1] = static_cast<unsigned char>(magnitude);  // Green
            outputImage[y][x][2] = static_cast<unsigned char>(magnitude);  // Blue
        }
    }

    // After all calculations, copy the result back to the input array
    imageArray = outputImage;
}


void NonMaxSuppression(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, int width, int height) {
    // Create a copy of the image array to store the results after NMS
    std::vector<std::vector<std::vector<unsigned char>>> resultImage = imageArray;

    // Loop through each pixel (excluding borders)
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            unsigned char centerPixel = imageArray[y][x][0];  // Using grayscale value (gradient magnitude)

            bool keepPixel = false;
            if(imageArray[y][x][0] >= imageArray[y][x-1][0]  &&  imageArray[y][x][0] >= imageArray[y][x+1][0]   )
                keepPixel = true   ; 
                
              
            // Check horizontal direction (compare with left and right neighbors
            if (std::abs(imageArray[y][x+1][0] - centerPixel) <= std::abs(imageArray[y][x-1][0] - centerPixel)) {
                if (centerPixel >= imageArray[y][x-1][0] && centerPixel >= imageArray[y][x+1][0]) {
                    keepPixel = true;
                }
            }
            // Check vertical direction (compare with top and bottom neighbors)
            else if (std::abs(imageArray[y+1][x][0] - centerPixel) <= std::abs(imageArray[y-1][x][0] - centerPixel)) {
                if (centerPixel >= imageArray[y-1][x][0] && centerPixel >= imageArray[y+1][x][0]) {
                    keepPixel = true;
                }
            }

            // If the current pixel is not a local maximum, suppress it (set to 0)
            if (!keepPixel) {
                resultImage[y][x][0] = 0;
                resultImage[y][x][1] = 0;
                resultImage[y][x][2] = 0;
            }
        }
    }

    // Update the original image with the suppressed values
    imageArray = resultImage;
}

void DoubleThresholdAndHysteresis(
    std::vector<std::vector<std::vector<unsigned char>>>& imageArray, 
    int width, 
    int height, 
    int lowThreshold, 
    int highThreshold) 
{
    // Create a result image to store the final edges
    std::vector<std::vector<std::vector<unsigned char>>> resultImage = imageArray;

    // Step 1: Apply Double Thresholding
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            unsigned char pixel = imageArray[y][x][0];  // Grayscale value (assuming R channel represents intensity)

            if (pixel >= highThreshold) {
                // Strong edge (White)
                resultImage[y][x][0] = 255;
                resultImage[y][x][1] = 255;
                resultImage[y][x][2] = 255;
            }
            else if (pixel >= lowThreshold) {
                // Weak edge (Gray)
                resultImage[y][x][0] = 128;
                resultImage[y][x][1] = 128;
                resultImage[y][x][2] = 128;
            }
            else {
                // Non-edge (Black)
                resultImage[y][x][0] = 0;
                resultImage[y][x][1] = 0;
                resultImage[y][x][2] = 0;
            }
        }
    }

    // Step 2: Apply Hysteresis
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            // If the pixel is a weak edge
            if (resultImage[y][x][0] == 128) {
                // Check all 8 neighbors for a strong edge
                bool connectedToStrongEdge = false;
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        int nx = x + dx;
                        int ny = y + dy;

                        if (resultImage[ny][nx][0] == 255) {
                            connectedToStrongEdge = true;
                            break;
                        }
                    }
                    if (connectedToStrongEdge) break;
                }

                // If connected to a strong edge, make it a strong edge
                if (connectedToStrongEdge) {
                    resultImage[y][x][0] = 255;
                    resultImage[y][x][1] = 255;
                    resultImage[y][x][2] = 255;
                } 
                else {
                    // Otherwise, suppress to non-edge
                    resultImage[y][x][0] = 0;
                    resultImage[y][x][1] = 0;
                    resultImage[y][x][2] = 0;
                }
            }
        }
    }

    // Update the original image with the final result
    imageArray = resultImage;
}





void HysteresisThresholding(std::vector<std::vector<std::vector<unsigned char>>>& imageArray,
                            int width, int height, float t1, float t2) {

    // Create a copy of the image to store the result
    std::vector<std::vector<std::vector<unsigned char>>> resultImage = imageArray;

    // Step 1: Apply High Threshold (strong edges)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char pixel = imageArray[y][x][0];  // Grayscale value of pixel
            if (pixel >= t2 * 255) {
                // Strong edge (White)
                resultImage[y][x][0] = 255;
                resultImage[y][x][1] = 255;
                resultImage[y][x][2] = 255;
            } else {
                resultImage[y][x][0] = 0;  // Non-edge (Black)
                resultImage[y][x][1] = 0;
                resultImage[y][x][2] = 0;
            }
        }
    }

    // Step 2: Apply Low Threshold (weak edges) and link weak edges to strong ones
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            unsigned char pixel = imageArray[y][x][0];  // Grayscale value of pixel

            if (pixel >= t1 * 255 && pixel < t2 * 255) {
                // If the pixel is a weak edge and connected to a strong edge, keep it
                bool connectedToStrongEdge = false;

                // Check 8 neighbors of the current pixel
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        int nx = x + dx;
                        int ny = y + dy;

                        // Check if any neighboring pixel is a strong edge
                        if (resultImage[ny][nx][0] == 255) {
                            connectedToStrongEdge = true;
                            break;
                        }
                    }
                    if (connectedToStrongEdge) break;
                }

                // If connected to strong edge, keep it as a strong edge, otherwise discard
                if (connectedToStrongEdge) {
                    resultImage[y][x][0] = 255;  // Strong edge
                    resultImage[y][x][1] = 255;
                    resultImage[y][x][2] = 255;
                } else {
                    resultImage[y][x][0] = 0;  // Non-edge
                    resultImage[y][x][1] = 0;
                    resultImage[y][x][2] = 0;
                }
            }
        }
    }

    // Step 3: Update the original image with the final result
    imageArray = resultImage;
}




// Floyd-Steinberg 

void floydSteinbergDither(std::vector<std::vector<std::vector<unsigned char>>>& image) {
    // Define the error diffusion coefficients
    const float alpha = 7.0f / 16.0f;
    const float beta = 3.0f / 16.0f;
    const float gamma = 5.0f / 16.0f;
    const float delta = 1.0f / 16.0f;

    int height = image.size(); 
    int width = image[0].size ();  

    // Process each pixel in the image (3D array for RGB)
    for (int y = 0; y < height - 1; ++y) {
        for (int x = 0; x < width - 1; ++x) {
            for (int c = 0; c < 3; ++c) {  // Loop through the RGB channels
                int oldPixel = image[y][x][c];
                
                // Map the pixel to one of 16 levels (0, 16, 32, ..., 240)
                int newPixel = static_cast<int>(std::round(oldPixel / 16.0)) * 16;

                // Ensure the new pixel value stays within [0, 255]
                if (newPixel < 0) newPixel = 0;
                if (newPixel > 255) newPixel = 255;

                // Set the pixel to the new value
                image[y][x][c] = newPixel;

                // Calculate the error
                int error = oldPixel - newPixel;

                // Distribute the error to neighboring pixels
                if (x + 1 < width) {
                    int temp = image[y][x + 1][c] + static_cast<int>(error * alpha);
                    image[y][x + 1][c] = (temp < 0) ? 0 : (temp > 255 ? 255 : temp);
                }

                if (y + 1 < height) {
                    if (x - 1 >= 0) {
                        int temp = image[y + 1][x - 1][c] + static_cast<int>(error * beta);
                        image[y + 1][x - 1][c] = (temp < 0) ? 0 : (temp > 255 ? 255 : temp);
                    }
                    int temp = image[y + 1][x][c] + static_cast<int>(error * gamma);
                    image[y + 1][x][c] = (temp < 0) ? 0 : (temp > 255 ? 255 : temp);

                    if (x + 1 < width) {
                        int temp = image[y + 1][x + 1][c] + static_cast<int>(error * delta);
                        image[y + 1][x + 1][c] = (temp < 0) ? 0 : (temp > 255 ? 255 : temp);
                    }
                }
            }
        }
    }
}



//  Halftone

std::vector<std::vector<std::vector<unsigned char>>> Halftone(
    const std::vector<std::vector<std::vector<unsigned char>>>& image) {
    
    int inputHeight = image.size(); // Number of rows
    int inputWidth = image[0].size(); // Number of columns
    int channels = image[0][0].size(); // Number of color channels (e.g., 3 for RGB)

    int outputHeight = inputHeight * 2;
    int outputWidth = inputWidth * 2;

    std::vector<std::vector<std::vector<unsigned char>>> outputImage(
        outputHeight, std::vector<std::vector<unsigned char>>(outputWidth, std::vector<unsigned char>(channels, 0))
    );



    
    const unsigned char pattern[4][2][2] = {
            {{0, 0}, {0, 0}},   // All black (Intensity: 0–63)
        {{0, 0},{255, 0}}, // 1 white (Intensity: 64–127)          here you can change it    as the lectures said 
        {{0,255}, {255,0}}, // 2 white (Intensity: 128–191)
        {{0, 255}, {255, 255}}  // 3 white (Intensity: 192–255)
    };

    // Loop over the input image (height and width)
    for (int y = 0; y < inputHeight; ++y) {
        for (int x = 0; x < inputWidth; ++x) {
            for (int c = 0; c < channels; ++c) {
                // Get the intensity of the current pixel in the current channel
                unsigned char intensity = image[y][x][c];

                // Determine the halftone pattern based on intensity
                int patternIndex = intensity / 64;

                // Map the 2x2 halftone block to the output image
                for (int dy = 0; dy < 2; ++dy) {
                    for (int dx = 0; dx < 2; ++dx) {
                        int outY = y * 2 + dy;
                        int outX = x * 2 + dx;

                        // Assign the corresponding intensity from the pattern
                        outputImage[outY][outX][c] = pattern[patternIndex][dy][dx];
                    }
                }
            }
        }
    }

    return outputImage;
}