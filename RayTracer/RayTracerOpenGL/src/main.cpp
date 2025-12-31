#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Debugger.h>
#include <VertexBuffer.h>
#include <VertexBufferLayout.h>
#include <IndexBuffer.h>
#include <VertexArray.h>
#include <Shader.h>
#include <Texture.h>   
#include <Camera.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cerrno>
#include <cstdio>
#include <sys/stat.h>

#include <../include/stb/stb_image.h>
#include <../include/stb/stb_image_write.h>

#include <Raytracer/SceneReader.h>
#include "Raytracer/phong.h"


#define WIDTH 1000
#define HEIGHT 1000

using Image3D = std::vector<std::vector<std::vector<unsigned char>>>;

static std::string MakeUniquePngName(const std::string& baseNameNoExt);
static std::string SaveImage(const Image3D& imageArray,
                             const std::string& imageName,
                             const std::string& outputDirectory);

static Image3D RayTrace(Scene& scene, int width, int height);

static GLuint CreateTextureFromImage(const Image3D& image);

static void framebuffer_size_callback(GLFWwindow* window, int fbW, int fbH)
{
    glViewport(0, 0, fbW, fbH);
}


int main(int argc, char *argv[])
{
    std::string filepath_input = "../src/sample_scene1.txt";
    std::string filepath_outputImage = "../output/";
    std::string outputBaseName = "raytracing"; 

    if (argc > 1) {
        filepath_input = argv[1];
    }

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    SceneReader reader;
    Scene* scene = reader.readScene(filepath_input);
    if (!scene) {
        std::cerr << "Failed to read scene: " << filepath_input << "\n";
        return -1;
    }

    std::cout << "Rendering...\n";
    Image3D image = RayTrace(*scene, WIDTH, HEIGHT);

    std::string uniqueName = MakeUniquePngName(outputBaseName); 
    std::string savedPath = SaveImage(image, uniqueName, filepath_outputImage);

    std::cout << "Saved: " << savedPath << "\n";

    return 0;

}


static Image3D RayTrace(Scene &scene, int width, int height)
{
    const int SAMPLES_PER_PIXEL = 1;

    Image3D image(height,
        std::vector<std::vector<unsigned char>>(width, std::vector<unsigned char>(3)));

    for (int y = 0; y < height; y++)
    {
        

        for (int x = 0; x < width; x++)
        {
            glm::vec3 accumulatedColor(0.0f);

            for (int sample = 0; sample < SAMPLES_PER_PIXEL; sample++)
            {
                float offsetX = 0.0f;
                float offsetY = 0.0f;
                if (SAMPLES_PER_PIXEL > 1) {
                    offsetX = (static_cast<float>(rand()) / RAND_MAX - 0.5f) / width;
                    offsetY = (static_cast<float>(rand()) / RAND_MAX - 0.5f) / height;
                }

                Ray ray = SceneReader::ConstructRayThroughPixel(
                    x + offsetX, y + offsetY, width, height, scene);

                accumulatedColor += Phong::calcColor(scene, ray, 0);
            }

            glm::vec3 finalColor = accumulatedColor / static_cast<float>(SAMPLES_PER_PIXEL);

            image[y][x][0] = static_cast<unsigned char>(255 * glm::clamp(finalColor.r, 0.0f, 1.0f));
            image[y][x][1] = static_cast<unsigned char>(255 * glm::clamp(finalColor.g, 0.0f, 1.0f));
            image[y][x][2] = static_cast<unsigned char>(255 * glm::clamp(finalColor.b, 0.0f, 1.0f));
        }
    }

    return image;
}


static std::string MakeUniquePngName(const std::string& baseNameNoExt)
{
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    tm = *std::localtime(&t);
#endif

    char buf[64];
    std::snprintf(buf, sizeof(buf),
                  "%s_%04d%02d%02d_%02d%02d%02d.png",
                  baseNameNoExt.c_str(),
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec);

    return std::string(buf);
}

static std::string SaveImage(const Image3D &imageArray,
                             const std::string &imageName,
                             const std::string &outputDirectory)
{
    std::string filePath = outputDirectory + imageName;

    int height = static_cast<int>(imageArray.size());
    int width  = static_cast<int>(imageArray[0].size());
    int channels = static_cast<int>(imageArray[0][0].size()); 

    std::vector<unsigned char> buffer(width * height * channels);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int index = (y * width + x) * channels;
            for (int c = 0; c < channels; ++c)
                buffer[index + c] = imageArray[y][x][c];
        }
    }

    int stride = width * channels;
    int result = stbi_write_png(filePath.c_str(), width, height, channels, buffer.data(), stride);

    if (!result)
        std::cerr << "Failed to save image: " << filePath << "\n";
    else
        std::cout << "Image saved successfully: " << filePath << "\n";

    return filePath;
}


static GLuint CreateTextureFromImage(const Image3D& image)
{
    int height = static_cast<int>(image.size());
    int width  = static_cast<int>(image[0].size());
    int channels = static_cast<int>(image[0][0].size()); 

    std::vector<unsigned char> pixels(width * height * channels);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int flipped_y = height - 1 - y; 
            int idx = (y * width + x) * channels;
            pixels[idx + 0] = image[flipped_y][x][0];
            pixels[idx + 1] = image[flipped_y][x][1];
            pixels[idx + 2] = image[flipped_y][x][2];
        }
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB8,
                 width,
                 height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 pixels.data());

    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}
