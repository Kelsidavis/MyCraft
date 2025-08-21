#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <vector>
#include <string>

struct ImageData {
    int width;
    int height;
    std::vector<unsigned char> data;
    
    ImageData() : width(0), height(0) {}
    ImageData(int w, int h) : width(w), height(h), data(w * h * 3) {}
};

class ImageLoader {
public:
    // Load PPM image (simple format, no external dependencies)
    static bool loadPPM(const std::string& filename, ImageData& image);
    
    // Create a simple solid color texture
    static ImageData createSolidTexture(int width, int height, unsigned char r, unsigned char g, unsigned char b);
    
    // Create a pattern texture (for testing)
    static ImageData createPatternTexture(int width, int height, int pattern);
    
    // Save PPM image (for generating textures)
    static bool savePPM(const std::string& filename, const ImageData& image);
};

#endif // IMAGELOADER_H