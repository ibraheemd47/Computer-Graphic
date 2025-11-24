#include "filters.h"

#include <cmath>
#include <fstream>
#include <iostream>

template<typename T>
static T clamp(T v, T lo, T hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// -------------------- I/O --------------------

Image loadImageTo3D(const std::string& filePath,
                    int& width,
                    int& height,
                    int& channels) {
    unsigned char* buffer = stbi_load(filePath.c_str(),
                                      &width,
                                      &height,
                                      &channels,
                                      0);
    if (!buffer) {
        std::cerr << "Failed to load image: " << filePath << "\n";
        std::exit(EXIT_FAILURE);
    }

    Image img(height,
              std::vector<std::vector<unsigned char>>(
                  width,
                  std::vector<unsigned char>(channels, 0)));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * channels;
            for (int c = 0; c < channels; ++c) {
                img[y][x][c] = buffer[idx + c];
            }
        }
    }

    stbi_image_free(buffer);
    return img;
}

void save3DImage(const Image& img,
                 const std::string& filePath) {
    int height = static_cast<int>(img.size());
    int width  = static_cast<int>(img[0].size());
    int ch     = static_cast<int>(img[0][0].size());

    std::vector<unsigned char> buffer(width * height * ch);

    for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
        int idx = (y * width + x) * ch;
        for (int c = 0; c < ch; ++c) {
            if (ch == 4 && c == 3) {
                // force alpha channel to opaque
                buffer[idx + c] = 255;
            } else {
                buffer[idx + c] = img[y][x][c];
            }
        }
    }
}


    int stride = width * ch;
    int ok = stbi_write_png(filePath.c_str(),
                            width,
                            height,
                            ch,
                            buffer.data(),
                            stride);
    if (!ok) {
        std::cerr << "Failed to save image: " << filePath << "\n";
    } else {
        std::cout << "Saved image: " << filePath << "\n";
    }
}

// Save grayscale channel as comma-separated text
void save3DImageAsText(const Image& img,
                       const std::string& filePath,
                       int levels) {
    int height = static_cast<int>(img.size());
    int width  = static_cast<int>(img[0].size());

    std::ofstream out(filePath);
    if (!out) {
        std::cerr << "Failed to open " << filePath << " for writing\n";
        return;
    }

    bool first = true;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char g = img[y][x][0]; // first channel
            int value = static_cast<int>(
                std::round((g / 255.0) * (levels - 1))
            );

            if (!first) out << ",";
            first = false;
            out << value;
        }
    }

    std::cout << "Saved text data: " << filePath << "\n";
}

// ---------------- Grayscale ----------------

void makeGrayscale(Image& img,
                   int width,
                   int height) {
    int channels = static_cast<int>(img[0][0].size());
    if (channels < 3) return;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char r = img[y][x][0];
            unsigned char g = img[y][x][1];
            unsigned char b = img[y][x][2];

            unsigned char gray =
                static_cast<unsigned char>(0.3 * r + 0.59 * g + 0.11 * b);

            img[y][x][0] = gray;
            if (channels > 1) img[y][x][1] = gray;
            if (channels > 2) img[y][x][2] = gray;
        }
    }
}

// -------------- helper: Gaussian blur 3x3 --------------

static void gaussianBlur3x3(Image& img,
                            int width,
                            int height) {
    const float k[3][3] = {
        {1.f/16, 2.f/16, 1.f/16},
        {2.f/16, 4.f/16, 2.f/16},
        {1.f/16, 2.f/16, 1.f/16}
    };

    int channels = static_cast<int>(img[0][0].size());
    Image out = img;

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            float sum = 0.0f;

            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    unsigned char v = img[y + ky][x + kx][0];
                    sum += k[ky + 1][kx + 1] * v;
                }
            }

            unsigned char gray =
                static_cast<unsigned char>(clamp(sum, 0.f, 255.f));

            out[y][x][0] = gray;
            if (channels > 1) out[y][x][1] = gray;
            if (channels > 2) out[y][x][2] = gray;
        }
    }

    img = out;
}

// -------------- Canny edge detection --------------

Image applyCanny(const Image& grayInput,
                 int width,
                 int height,
                 float lowRatio,
                 float highRatio) {
    // Step 0: ensure grayscale + blur
    Image img = grayInput;
    makeGrayscale(img, width, height);
    gaussianBlur3x3(img, width, height);

    int channels = static_cast<int>(img[0][0].size());
    int N = width * height;

    // 1. gradient magnitude + direction (Sobel)
    std::vector<float> mag(N, 0.0f);
    std::vector<float> dir(N, 0.0f);

    const int gxK[3][3] = {
        { -1, 0, 1 },
        { -2, 0, 2 },
        { -1, 0, 1 }
    };
    const int gyK[3][3] = {
        {  1,  2,  1 },
        {  0,  0,  0 },
        { -1, -2, -1 }
    };

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            float gx = 0.0f;
            float gy = 0.0f;

            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    unsigned char v = img[y + ky][x + kx][0];
                    gx += gxK[ky + 1][kx + 1] * v;
                    gy += gyK[ky + 1][kx + 1] * v;
                }
            }

            float m = std::sqrt(gx * gx + gy * gy);
            float angle = std::atan2(gy, gx) * 180.0f / 3.14159265f;
            if (angle < 0) angle += 180.0f;

            float q;
            if ((angle >= 0 && angle < 22.5) || (angle >= 157.5 && angle < 180))
                q = 0;
            else if (angle >= 22.5 && angle < 67.5)
                q = 45;
            else if (angle >= 67.5 && angle < 112.5)
                q = 90;
            else
                q = 135;

            int idx = y * width + x;
            mag[idx] = m;
            dir[idx] = q;
        }
    }

    // 2. Non-maximum suppression
    std::vector<float> nms(N, 0.0f);

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int idx = y * width + x;
            float m = mag[idx];
            float d = dir[idx];

            float m1 = 0.0f, m2 = 0.0f;

            if (d == 0) {
                m1 = mag[y * width + (x - 1)];
                m2 = mag[y * width + (x + 1)];
            } else if (d == 45) {
                m1 = mag[(y - 1) * width + (x + 1)];
                m2 = mag[(y + 1) * width + (x - 1)];
            } else if (d == 90) {
                m1 = mag[(y - 1) * width + x];
                m2 = mag[(y + 1) * width + x];
            } else { // 135
                m1 = mag[(y - 1) * width + (x - 1)];
                m2 = mag[(y + 1) * width + (x + 1)];
            }

            if (m >= m1 && m >= m2)
                nms[idx] = m;
            else
                nms[idx] = 0.0f;
        }
    }

    // 3. Double threshold
    float maxMag = 0.0f;
    for (float v : nms)
        if (v > maxMag) maxMag = v;

    float high = highRatio * maxMag;
    float low  = lowRatio * maxMag;

    enum EdgeType { NONE = 0, WEAK = 1, STRONG = 2 };
    std::vector<int> edges(N, NONE);

    for (int i = 0; i < N; ++i) {
        if (nms[i] >= high)
            edges[i] = STRONG;
        else if (nms[i] >= low)
            edges[i] = WEAK;
    }

    // 4. Hysteresis (keep WEAK that touch STRONG)
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int idx = y * width + x;
            if (edges[idx] != WEAK) continue;

            bool connected = false;
            for (int dy = -1; dy <= 1 && !connected; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    int ni = (y + dy) * width + (x + dx);
                    if (edges[ni] == STRONG) {
                        connected = true;
                        break;
                    }
                }
            }
            edges[idx] = connected ? STRONG : NONE;
        }
    }

    // Build output image: 255 for edges, 0 otherwise
    Image out(height,
              std::vector<std::vector<unsigned char>>(
                  width,
                  std::vector<unsigned char>(channels, 0)));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * width + x;
            unsigned char v = (edges[idx] == STRONG) ? 255 : 0;
            for (int c = 0; c < channels && c < 3; ++c) {
                out[y][x][c] = v;
            }
        }
    }

    return out;
}

// ---------------- Halftone ----------------

Image applyHalftone(const Image& grayInput,
                    int width,
                    int height) {
    Image gray = grayInput;
    makeGrayscale(gray, width, height);

    int channels = static_cast<int>(gray[0][0].size());

    int outW = width * 2;
    int outH = height * 2;

    Image out(outH,
              std::vector<std::vector<unsigned char>>(
                  outW,
                  std::vector<unsigned char>(channels, 0)));

    const unsigned char pattern[4][2][2] = {
        { {0, 0},   {0, 0}   },
        { {0, 0},   {255, 0} },
        { {0, 255}, {255, 0} },
        { {0, 255}, {255,255} }
    };

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char g = gray[y][x][0];
            int idx = g / 64; // 0..3

            for (int dy = 0; dy < 2; ++dy) {
                for (int dx = 0; dx < 2; ++dx) {
                    unsigned char v = pattern[idx][dy][dx];
                    int oy = y * 2 + dy;
                    int ox = x * 2 + dx;

                    for (int c = 0; c < channels && c < 3; ++c) {
                        out[oy][ox][c] = v;
                    }
                }
            }
        }
    }

    return out;
}

// ------------- Floyd–Steinberg (16 gray levels) -------------

void applyFloydSteinberg16(Image& gray,
                           int width,
                           int height) {
    makeGrayscale(gray, width, height);

    int channels = static_cast<int>(gray[0][0].size());

    std::vector<float> buf(width * height, 0.0f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            buf[y * width + x] = gray[y][x][0];
        }
    }

    const float a = 7.f / 16.f;
    const float b = 3.f / 16.f;
    const float c = 5.f / 16.f;
    const float d = 1.f / 16.f;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * width + x;
            float oldVal = buf[idx];

            int level = static_cast<int>(std::round(oldVal / 255.0f * 15));
            float newVal = (level / 15.0f) * 255.0f;

            buf[idx] = newVal;
            float error = oldVal - newVal;

            if (x + 1 < width)
                buf[idx + 1] += error * a;

            if (y + 1 < height) {
                if (x > 0)
                    buf[(y + 1) * width + (x - 1)] += error * b;
                buf[(y + 1) * width + x] += error * c;
                if (x + 1 < width)
                    buf[(y + 1) * width + (x + 1)] += error * d;
            }
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float v = clamp(buf[y * width + x], 0.0f, 255.0f);
            unsigned char g = static_cast<unsigned char>(v);

            gray[y][x][0] = g;
            if (channels > 1) gray[y][x][1] = g;
            if (channels > 2) gray[y][x][2] = g;
        }
    }
}
