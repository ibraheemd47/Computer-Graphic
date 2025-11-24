#include "filters.h"
#include <iostream>
#include <string>
#include <vector>

// Function prototypes
void ApplyGrayscaleFilter(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, const std::string& outputPath);
void ApplyCannyEdgeDetection(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, const std::string& outputPath);
void ApplyHalftoneFilter(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, const std::string& outputPath);
void ApplyFloydSteinbergDither(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, const std::string& outputPath);



std::string filepath_inputImage = "C:\\Users\\ibrah\\OneDrive\\Desktop\\computerGraphics-main\\Lenna.png";
std::string  filepath_outputImage = "C:\\Users\\ibrah\\OneDrive\\Desktop\\computerGraphics-main\\output_images\\"; 
int main() {
    // Filepaths
   

    // Load the image into a 3D vector array
    std::vector<std::vector<std::vector<unsigned char>>> imageArray = LoadImageToArray(filepath_inputImage);

    // Menu for selecting filters
    while (true) {
        std::cout << "Choose a filter to apply:\n";
        std::cout << "1. Grayscale\n";
        std::cout << "2. Canny Edge Detection  \n";
        std::cout << "3. Halftone - Grayscale \n";
        std::cout << "4. Floyd-Steinberg Dither -Grayscale \n";
        std::cout << "5. Exit\n";
        std::cout << "Enter your choice: ";

        int choice;
        std::cin >> choice;

        // Apply selected filter
        switch (choice) {
            case 1:
                ApplyGrayscaleFilter(imageArray, filepath_outputImage);
                break;
            case 2:
                ApplyCannyEdgeDetection(imageArray, filepath_outputImage);
                break;
            case 3:
                ApplyHalftoneFilter(imageArray, filepath_outputImage);
                break;
            case 4:
                ApplyFloydSteinbergDither(imageArray, filepath_outputImage);
                break;
            case 5:
                std::cout << "Exiting program.\n";
                return 0;
            default:
                std::cout << "Invalid choice. Please try again.\n";
        }
    }

    return 0;
}

// Function Definitions

void ApplyGrayscaleFilter(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, const std::string& outputPath) {
    std::string outputImageName = "grayscale.png";
    ConvertToGrayscale(imageArray);
    SaveImage(imageArray, outputImageName, outputPath);
    std::cout << "Grayscale filter applied and saved to " << outputPath + outputImageName << "\n";

    imageArray = LoadImageToArray(filepath_inputImage);
}

void ApplyCannyEdgeDetection(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, const std::string& outputPath) {
    std::string outputImageName = "canny_edge.png";
    ConvertToGrayAveragescale(imageArray);
    ApplyGaussianFilter3x3(imageArray, 256, 256);
    GradientCalculation(imageArray, 256, 256);
    NonMaxSuppression(imageArray, 256, 256);
    HysteresisThresholding(imageArray, 256, 256, 0.3, 0.7);   /// you can change theas parameters   in the lecture it was written 0.1/0.9
    SaveImage(imageArray, outputImageName, outputPath);
    std::cout << "Canny Edge Detection filter applied and saved to " << outputPath + outputImageName << "\n";

        imageArray = LoadImageToArray(filepath_inputImage);   /// updating the image 

}

void ApplyHalftoneFilter(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, const std::string& outputPath) {
    std::string outputImageName = "halftone.png";
    ConvertToGrayAveragescale(imageArray);
    std::vector<std::vector<std::vector<unsigned char>>> output = Halftone(imageArray);
    SaveImage(output, outputImageName, outputPath);
    std::cout << "Halftone filter applied and saved to " << outputPath + outputImageName << "\n";

        imageArray = LoadImageToArray(filepath_inputImage);

}

void ApplyFloydSteinbergDither(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, const std::string& outputPath) {
    std::string outputImageName = "floyd_steinberg.png";
    ConvertToGrayAveragescale(imageArray);
    floydSteinbergDither(imageArray);
    SaveImage(imageArray, outputImageName, outputPath);
    std::cout << "Floyd-Steinberg Dither filter applied and saved to " << outputPath + outputImageName << "\n";

        imageArray = LoadImageToArray(filepath_inputImage);

}