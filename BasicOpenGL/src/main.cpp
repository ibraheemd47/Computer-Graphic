#include "filters.h"

#include <GLFW/glfw3.h>   // from BasicOpenGL / GLFW :contentReference[oaicite:4]{index=4}
#include <iostream>
#include <string>

// Forward declarations for OpenGL viewer
void showAllImagesWindow(const std::string& outputDir);
static GLuint loadTexture(const std::string& path, int& outWidth, int& outHeight);
static void drawTexturedQuad(GLuint tex,
                             float xMin, float yMin,
                             float xMax, float yMax);

int main() {
    std::string inputPath =
        "C:/Users/ibrah/Computer-Graphic/BasicOpenGL/Lenna.png";

    // IMPORTANT: keep final slash
    std::string outputDir =
        "C:/Users/ibrah/Computer-Graphic/BasicOpenGL/output/";

    while (true) {
        std::cout << "\nChoose a filter to apply:\n"
                  << "1. Grayscale\n"
                  << "2. Canny Edge Detection\n"
                  << "3. Halftone\n"
                  << "4. Floyd-Steinberg (16 levels)\n"                  
                  << "5. Show all images in OpenGL window\n"
                  << "6. Exit\n"
                  << "Enter choice: ";

        int choice;
        std::cin >> choice;

        if (choice == 6) {
            std::cout << "Exiting.\n";
            break;
        }

        if (choice == 5) {
            // show all four result images in one window
            showAllImagesWindow(outputDir);
            continue;
        }

        // For filters, always start from original Lenna
        Image img = loadImage(inputPath);

        switch (choice) {
            case 1: {
                // Grayscale.png + Grayscale.txt (0..15)
                toGrayscale(img);
                saveImage(img, outputDir + "Grayscale.png");
                break;
            }
            case 2: {
                // Canny.png + Canny.txt (0..1)
                toGrayscale(img);
                Image edges = cannyEdges(img, 0.1f, 0.3f); // as in lecture ratios
                saveImage(edges, outputDir + "Canny.png");
                break;
            }
            case 3: {
                // Halftone.png + Halftone.txt (0..1)
                toGrayscale(img);
                Image h = halftone(img);
                saveImage(h, outputDir + "Halftone.png");
                break;
            }
            case 4: {
                // FloyedSteinberg.png + FloyedSteinberg.txt (0..15)
                toGrayscale(img);
                floydSteinberg16(img);
                saveImage(img, outputDir + "FloyedSteinberg.png");
                break;
            }
            default:
                std::cout << "Invalid choice.\n";
                break;
        }
    }

    return 0;
}

// ----------------- OpenGL helper: load texture -----------------
//
// Uses stb_image to load PNG -> creates an OpenGL 2D texture. :contentReference[oaicite:5]{index=5}
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

// ----------------- OpenGL helper: draw quad -----------------
//
// Draws a textured quad in NDC coordinates (x,y in [-1,1]) using
// old-style immediate mode for simplicity. :contentReference[oaicite:6]{index=6}
static void drawTexturedQuad(GLuint tex,
                             float xMin, float yMin,
                             float xMax, float yMax) {
    glBindTexture(GL_TEXTURE_2D, tex);

    glBegin(GL_QUADS);
        // notice: v coords 1 ↔ 0 swapped
        glTexCoord2f(0.0f, 1.0f); glVertex2f(xMin, yMin); // bottom-left
        glTexCoord2f(1.0f, 1.0f); glVertex2f(xMax, yMin); // bottom-right
        glTexCoord2f(1.0f, 0.0f); glVertex2f(xMax, yMax); // top-right
        glTexCoord2f(0.0f, 0.0f); glVertex2f(xMin, yMax); // top-left
    glEnd();
}


// ----------------- Show all four images window -----------------
//
// Opens a GLFW window and shows:
// top-left:    Grayscale
// top-right:   Canny
// bottom-left: Halftone
// bottom-right:Floyd–Steinberg :contentReference[oaicite:7]{index=7}
void showAllImagesWindow(const std::string& outputDir) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    GLFWwindow* window = glfwCreateWindow(800, 800,
                                          "All Filters - Lenna",
                                          nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glEnable(GL_TEXTURE_2D);

    int w1,h1,w2,h2,w3,h3,w4,h4;
    GLuint texGray   = loadTexture(outputDir + "Grayscale.png",       w1, h1);
    GLuint texCanny  = loadTexture(outputDir + "Canny.png",           w2, h2);
    GLuint texHalf   = loadTexture(outputDir + "Halftone.png",        w3, h3);
    GLuint texFloyd  = loadTexture(outputDir + "FloyedSteinberg.png", w4, h4);

    if (!texGray || !texCanny || !texHalf || !texFloyd) {
        std::cerr << "Make sure all PNGs exist (run options 1–4 first).\n";
    }

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // Top-left: Grayscale
        if (texGray)
            drawTexturedQuad(texGray, -1.0f, 0.0f, 0.0f, 1.0f);
        // Top-right: Canny
        if (texCanny)
            drawTexturedQuad(texCanny, 0.0f, 0.0f, 1.0f, 1.0f);
        // Bottom-left: Halftone
        if (texHalf)
            drawTexturedQuad(texHalf, -1.0f, -1.0f, 0.0f, 0.0f);
        // Bottom-right: Floyd–Steinberg
        if (texFloyd)
            drawTexturedQuad(texFloyd, 0.0f, -1.0f, 1.0f, 0.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();

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
