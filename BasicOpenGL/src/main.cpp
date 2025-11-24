#include "filters.h"

#include <iostream>
#include <string>

int main() {
    // adjust these paths for your machine
    std::string inputPath =
        "C:/Users/ibrah/Computer-Graphic/BasicOpenGL/Lenna.png";

    std::string outputDir =
        "C:/Users/ibrah/Computer-Graphic/BasicOpenGL/output/"; // end with '/'

    int width = 0, height = 0, channels = 0;

    while (true) {
        std::cout << "\nChoose a filter to apply:\n"
                  << "1. Grayscale\n"
                  << "2. Canny Edge Detection\n"
                  << "3. Halftone\n"
                  << "4. Floyd-Steinberg (16 levels)\n"
                  << "5. Exit\n"
                  << "Enter choice: ";

        int choice;
        std::cin >> choice;

        if (choice == 5) {
            std::cout << "Exiting.\n";
            break;
        }

        // always start from original image
        Image img = loadImageTo3D(inputPath, width, height, channels);

        switch (choice) {
            case 1: {
                makeGrayscale(img, width, height);
                save3DImage(img, outputDir + "Grayscale.png");
                save3DImageAsText(img, outputDir + "Grayscale.txt", 16);
                break;
            }
            case 2: {
                makeGrayscale(img, width, height);
                // softer thresholds -> better edges (you can tweak)
                Image canny = applyCanny(img, width, height, 0.1f, 0.3f);
                save3DImage(canny, outputDir + "Canny.png");
                save3DImageAsText(canny, outputDir + "Canny.txt", 2);
                break;
            }
            case 3: {
                makeGrayscale(img, width, height);
                Image halftone = applyHalftone(img, width, height);
                save3DImage(halftone, outputDir + "Halftone.png");
                save3DImageAsText(halftone, outputDir + "Halftone.txt", 2);
                break;
            }
            case 4: {
                makeGrayscale(img, width, height);
                applyFloydSteinberg16(img, width, height);
                save3DImage(img, outputDir + "FloyedSteinberg.png");
                save3DImageAsText(img, outputDir + "FloyedSteinberg.txt", 16);
                break;
            }
            default:
                std::cout << "Invalid choice.\n";
                break;
        }
    }

    return 0;
}
