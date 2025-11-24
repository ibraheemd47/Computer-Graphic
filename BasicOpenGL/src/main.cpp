#include "filters.h"

#include <GLFW/glfw3.h>   // from BasicOpenGL
#include <filesystem>
#include <iostream>
#include <string>

// ----------- Forward declarations for viewer -----------
void showAllImagesWindow(const std::string& outputDir);
static GLuint loadTexture(const std::string& path, int& outWidth, int& outHeight);
static void drawTexturedQuad(GLuint tex,
                             float xMin, float yMin,
                             float xMax, float yMax);

int main() {
    // Run from bin/: input is ../src/Lenna.png, outputs to ../output/
    std::string inputPath = "../Lenna.png";
    std::string outputDir = "../output/"; // end with '/'

    // Ensure output directory exists
    std::filesystem::create_directories(outputDir);

    // Simple existence check to avoid confusing errors later
    if (!std::filesystem::exists(inputPath)) {
        std::cerr << "Missing input image at " << inputPath
                  << " (place Lenna.png in BasicOpenGL/src/)\n";
        return 1;
    }

    int width = 0, height = 0, channels = 0;

    while (true) {
        std::cout << "\nChoose a filter to apply:\n"
                  << "1. Grayscale\n"
                  << "2. Canny Edge Detection\n"
                  << "3. Halftone\n"
                  << "4. Floyd-Steinberg (16 levels)\n"
                  << "5. Exit\n"
                  << "6. Show all images in OpenGL window\n"
                  << "Enter choice: ";

        int choice;
        std::cin >> choice;

        if (choice == 5) {
            std::cout << "Exiting.\n";
            break;
        }

        if (choice == 6) {
            // Just open the window that shows the 4 generated images
            showAllImagesWindow(outputDir);
            continue;
        }

        // always start from original image for each filter
        Image img = loadImageTo3D(inputPath, width, height, channels);

        switch (choice) {
            case 1: {
                // Grayscale.png + Grayscale.txt (0..15)
                makeGrayscale(img, width, height);
                save3DImage(img, outputDir + "Grayscale.png");
                save3DImageAsText(img, outputDir + "Grayscale.txt", 16);
                break;
            }
            case 2: {
                // Canny.png + Canny.txt (0..1)
                makeGrayscale(img, width, height);
                // pick thresholds that give clear edges (you can tweak)
                Image canny = applyCanny(img, width, height, 0.03f, 0.10f);
                save3DImage(canny, outputDir + "Canny.png");
                save3DImageAsText(canny, outputDir + "Canny.txt", 2);
                break;
            }
            case 3: {
                // Halftone.png + Halftone.txt (0..1)
                makeGrayscale(img, width, height);
                Image halftone = applyHalftone(img, width, height);
                save3DImage(halftone, outputDir + "Halftone.png");
                save3DImageAsText(halftone, outputDir + "Halftone.txt", 2);
                break;
            }
            case 4: {
                // FloyedSteinberg.png + FloyedSteinberg.txt (0..15)
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

// ----------------- OpenGL helpers -----------------
//
// loadTexture: uses stb_image (already included via filters.h)
// to load a PNG file into an OpenGL texture.
//
static GLuint loadTexture(const std::string& path, int& outWidth, int& outHeight) {
    int w, h, ch;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 4); // force RGBA
    if (!data) {
        std::cerr << "Failed to load texture: " << path << "\n";
        return 0;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    outWidth  = w;
    outHeight = h;
    return tex;
}

// drawTexturedQuad: very simple immediate-mode quad
// (the style is similar to many basic GLFW/OpenGL examples).
static void drawTexturedQuad(GLuint tex,
                             float xMin, float yMin,
                             float xMax, float yMax) {
    glBindTexture(GL_TEXTURE_2D, tex);

    glBegin(GL_QUADS);
        // flip vertically so the image is not upside-down
        glTexCoord2f(0.0f, 1.0f); glVertex2f(xMin, yMin); // bottom-left
        glTexCoord2f(1.0f, 1.0f); glVertex2f(xMax, yMin); // bottom-right
        glTexCoord2f(1.0f, 0.0f); glVertex2f(xMax, yMax); // top-right
        glTexCoord2f(0.0f, 0.0f); glVertex2f(xMin, yMax); // top-left
    glEnd();
}

// showAllImagesWindow: follows the same structure as the BasicOpenGL main:
// glfwInit → create window → render loop → cleanup.
void showAllImagesWindow(const std::string& outputDir) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    GLFWwindow* window = glfwCreateWindow(
        800, 800, "All Filters - Lenna", nullptr, nullptr
    );
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glEnable(GL_TEXTURE_2D);

    // Load 4 textures from the PNGs you already generate
    int w1,h1,w2,h2,w3,h3,w4,h4;
    GLuint texGray   = loadTexture(outputDir + "Grayscale.png",       w1, h1);
    GLuint texCanny  = loadTexture(outputDir + "Canny.png",           w2, h2);
    GLuint texHalf   = loadTexture(outputDir + "Halftone.png",        w3, h3);
    GLuint texFloyd  = loadTexture(outputDir + "FloyedSteinberg.png", w4, h4);

    if (!texGray || !texCanny || !texHalf || !texFloyd) {
        std::cerr << "Make sure all PNGs exist (run options 1–4 first).\n";
    }

    // === Classic GLFW main loop (like the BasicOpenGL sample) ===
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // Top-left: Grayscale
        if (texGray)
            drawTexturedQuad(texGray, -1.0f,  0.0f,  0.0f,  1.0f);
        // Top-right: Canny
        if (texCanny)
            drawTexturedQuad(texCanny,  0.0f,  0.0f,  1.0f,  1.0f);
        // Bottom-left: Halftone
        if (texHalf)
            drawTexturedQuad(texHalf,  -1.0f, -1.0f,  0.0f,  0.0f);
        // Bottom-right: Floyd–Steinberg
        if (texFloyd)
            drawTexturedQuad(texFloyd,  0.0f, -1.0f,  1.0f,  0.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // ESC closes window (again like typical GLFW examples)
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    glDeleteTextures(1, &texGray);
    glDeleteTextures(1, &texCanny);
    glDeleteTextures(1, &texHalf);
    glDeleteTextures(1, &texFloyd);

    glfwDestroyWindow(window);
    glfwTerminate();
}
