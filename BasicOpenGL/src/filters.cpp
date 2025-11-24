#include "filters.h"

#include <cmath>
#include <iostream>
#include <fstream>

template<typename T>
T clamp(T v, T lo, T hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// ----------------- I/O ------------------

Image loadImage(const std::string& filePath) {
    Image img;
    int w, h, ch;
    unsigned char* buffer = stbi_load(filePath.c_str(), &w, &h, &ch, 0);
    if (!buffer) {
        std::cerr << "Failed to load image: " << filePath << "\n";
        std::exit(EXIT_FAILURE);
    }

    img.width = w;
    img.height = h;
    img.channels = ch;
    img.data.assign(buffer, buffer + (w * h * ch));

    stbi_image_free(buffer);
    return img;
}

void saveImage(const Image& img, const std::string& filePath) {
    if (img.data.empty()) {
        std::cerr << "Image is empty, not saving.\n";
        return;
    }

    int stride = img.width * img.channels;
    int ok = stbi_write_png(filePath.c_str(),
                            img.width,
                            img.height,
                            img.channels,
                            img.data.data(),
                            stride);
    if (!ok) {
        std::cerr << "Failed to save image: " << filePath << "\n";
    } else {
        std::cout << "Saved image: " << filePath << "\n";
    }
}

 

// ------------- basic grayscale -----------

void toGrayscale(Image& img) {
    if (img.channels < 3) {
        // already gray (1 channel) or similar
        return;
    }

    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            unsigned char r = img.at(x, y, 0);
            unsigned char g = img.at(x, y, 1);
            unsigned char b = img.at(x, y, 2);

            unsigned char gray =
                static_cast<unsigned char>(0.3 * r + 0.59 * g + 0.11 * b);

            img.at(x, y, 0) = gray;
            if (img.channels > 1) img.at(x, y, 1) = gray;
            if (img.channels > 2) img.at(x, y, 2) = gray;
        }
    }
}

// ------------- small Gaussian blur (3x3) -------------
// Standard 3x3 Gaussian kernel (sigma ~1) :contentReference[oaicite:1]{index=1}
static void gaussianBlur3x3(Image& img) {
    const float k[3][3] = {
        {1.f/16, 2.f/16, 1.f/16},
        {2.f/16, 4.f/16, 2.f/16},
        {1.f/16, 2.f/16, 1.f/16}
    };

    Image out = img; // same size/channels

    for (int y = 1; y < img.height - 1; ++y) {
        for (int x = 1; x < img.width - 1; ++x) {

            float sum = 0.0f;

            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    unsigned char v = img.at(x + kx, y + ky, 0);
                    sum += k[ky + 1][kx + 1] * v;
                }
            }

            unsigned char g = static_cast<unsigned char>(clamp(sum, 0.f, 255.f));

            out.at(x, y, 0) = g;
            if (out.channels > 1) out.at(x, y, 1) = g;
            if (out.channels > 2) out.at(x, y, 2) = g;
        }
    }

    img = out;
}

// ---------------- Canny edges ----------------
//
// Algorithm: Gaussian -> Sobel -> NMS -> double threshold -> hysteresis. :contentReference[oaicite:2]{index=2}
Image cannyEdges(Image gray, float lowRatio, float highRatio) {
    toGrayscale(gray);         // make sure it's gray

    // 1. Noise reduction
    gaussianBlur3x3(gray);

    int w = gray.width;
    int h = gray.height;
    int N = w * h;

    // 2. Gradient (Sobel) and direction
    std::vector<float> mag(N, 0.0f);
    std::vector<float> dir(N, 0.0f); // angle in degrees (0,45,90,135)

    const int gxKernel[3][3] = {
        { -1, 0, 1 },
        { -2, 0, 2 },
        { -1, 0, 1 }
    };
    const int gyKernel[3][3] = {
        {  1,  2,  1 },
        {  0,  0,  0 },
        { -1, -2, -1 }
    };

    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            float gx = 0.0f;
            float gy = 0.0f;

            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    unsigned char v = gray.at(x + kx, y + ky, 0);
                    gx += gxKernel[ky + 1][kx + 1] * v;
                    gy += gyKernel[ky + 1][kx + 1] * v;
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

            int idx = y * w + x;
            mag[idx] = m;
            dir[idx] = q;
        }
    }

    // 3. Non-maximum suppression
    std::vector<float> nms(N, 0.0f);

    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            int idx = y * w + x;
            float m = mag[idx];
            float d = dir[idx];

            float m1 = 0.0f, m2 = 0.0f;

            if (d == 0) {
                m1 = mag[y * w + (x - 1)];
                m2 = mag[y * w + (x + 1)];
            } else if (d == 45) {
                m1 = mag[(y - 1) * w + (x + 1)];
                m2 = mag[(y + 1) * w + (x - 1)];
            } else if (d == 90) {
                m1 = mag[(y - 1) * w + x];
                m2 = mag[(y + 1) * w + x];
            } else { // 135
                m1 = mag[(y - 1) * w + (x - 1)];
                m2 = mag[(y + 1) * w + (x + 1)];
            }

            if (m >= m1 && m >= m2) {
                nms[idx] = m;
            } else {
                nms[idx] = 0.0f;
            }
        }
    }

    // 4. Double threshold + hysteresis
    float maxMag = 0.0f;
    for (float val : nms) {
        if (val > maxMag) maxMag = val;
    }

    float high = highRatio * maxMag;
    float low  = lowRatio  * maxMag;

    enum EdgeType { NONE = 0, WEAK = 1, STRONG = 2 };
    std::vector<int> edges(N, 0);

    for (int i = 0; i < N; ++i) {
        if (nms[i] >= high)
            edges[i] = STRONG;
        else if (nms[i] >= low)
            edges[i] = WEAK;
        else
            edges[i] = NONE;
    }

    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            int idx = y * w + x;
            if (edges[idx] != WEAK) continue;

            bool connected = false;
            for (int dy = -1; dy <= 1 && !connected; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    int ni = (y + dy) * w + (x + dx);
                    if (edges[ni] == STRONG) {
                        connected = true;
                        break;
                    }
                }
            }
            edges[idx] = connected ? STRONG : NONE;
        }
    }

    Image out;
    out.width = w;
    out.height = h;
    out.channels = gray.channels;
    out.data.assign(w * h * out.channels, 0);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            unsigned char value = (edges[idx] == STRONG) ? 255 : 0;
            for (int c = 0; c < out.channels && c < 3; ++c) {
                out.at(x, y, c) = value;
            }
        }
    }

    return out;
}

// ------------------ Halftone ------------------
//
// Each input pixel becomes a 2x2 block of black/white according to
// four intensity ranges.
//
Image halftone(const Image& grayInput) {
    Image gray = grayInput;
    toGrayscale(gray);

    Image out;
    out.width  = gray.width * 2;
    out.height = gray.height * 2;
    out.channels = gray.channels;
    out.data.assign(out.width * out.height * out.channels, 0);

    const unsigned char pattern[4][2][2] = {
        { {0, 0},   {0, 0}   },   // darkest
        { {0, 0},   {255, 0} },   // one white
        { {0, 255}, {255, 0} },   // two whites
        { {0, 255}, {255,255} }   // three whites (brightest)
    };

    for (int y = 0; y < gray.height; ++y) {
        for (int x = 0; x < gray.width; ++x) {
            unsigned char intensity = gray.at(x, y, 0);
            int idx = intensity / 64; // 0..3

            for (int dy = 0; dy < 2; ++dy) {
                for (int dx = 0; dx < 2; ++dx) {
                    int ox = x * 2 + dx;
                    int oy = y * 2 + dy;
                    unsigned char v = pattern[idx][dy][dx];

                    for (int c = 0; c < out.channels && c < 3; ++c) {
                        out.at(ox, oy, c) = v;
                    }
                }
            }
        }
    }

    return out;
}

// ------------- Floyd–Steinberg (16 levels) -------------
//
// Classic error-diffusion pattern. :contentReference[oaicite:3]{index=3}
void floydSteinberg16(Image& gray) {
    toGrayscale(gray);

    int w = gray.width;
    int h = gray.height;

    std::vector<float> buffer(w * h, 0.0f);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            buffer[y * w + x] = gray.at(x, y, 0);
        }
    }

    const float a = 7.f / 16.f;
    const float b = 3.f / 16.f;
    const float c = 5.f / 16.f;
    const float d = 1.f / 16.f;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            float oldVal = buffer[idx];

            int level = static_cast<int>(std::round(oldVal / 255.0f * 15));
            float newVal = (level / 15.0f) * 255.0f;

            buffer[idx] = newVal;
            float error = oldVal - newVal;

            if (x + 1 < w)
                buffer[idx + 1] += error * a;

            if (y + 1 < h) {
                if (x > 0)
                    buffer[(y + 1) * w + (x - 1)] += error * b;

                buffer[(y + 1) * w + x] += error * c;

                if (x + 1 < w)
                    buffer[(y + 1) * w + (x + 1)] += error * d;
            }
        }
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float v = clamp(buffer[y * w + x], 0.0f, 255.0f);
            unsigned char g = static_cast<unsigned char>(v);

            gray.at(x, y, 0) = g;
            if (gray.channels > 1) gray.at(x, y, 1) = g;
            if (gray.channels > 2) gray.at(x, y, 2) = g;
        }
    }
}
