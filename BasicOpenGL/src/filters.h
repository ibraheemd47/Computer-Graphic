#ifndef FILTERS_H
#define FILTERS_H

#include <iostream>
#include <vector>
#include <string>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

// Image processing variables (size and channels)
 extern int width, height, channels;

// Function declarations
std::vector<std::vector<std::vector<unsigned char>>> LoadImageToArray(const std::string& filePath);

// gray scale
void ConvertToGrayscale(std::vector<std::vector<std::vector<unsigned char>>>& imageArray);
void ConvertToGrayAveragescale(std::vector<std::vector<std::vector<unsigned char>>>& imageArray); 


//  GaussianFilters
void ApplyGaussianFilter3x3(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, int width, int height); 
void ApplyGaussianFilter5x5(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, int width, int height);



// canny edge detection 
void GradientCalculation(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, int width, int height); 
void NonMaxSuppression(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, int width, int height)  ; 
void DoubleThresholdAndHysteresis(std::vector<std::vector<std::vector<unsigned char>>>& imageArray, int width, int height, int lowThreshold, int highThreshold) ; 
void HysteresisThresholding(std::vector<std::vector<std::vector<unsigned char>>>& imageArray,int width, int height, float t1, float t2);
                            





//Halftone
std::vector<std::vector<std::vector<unsigned char>>> Halftone(
    const std::vector<std::vector<std::vector<unsigned char>>>& image) ; 

//  FloyedSteinberg    
void floydSteinbergDither(std::vector<std::vector<std::vector<unsigned char>>>& image) ; 



void SaveImage(const std::vector<std::vector<std::vector<unsigned char>>>& imageArray, const std::string& imageName, const std::string& outputDirectory) ; 





#endif 