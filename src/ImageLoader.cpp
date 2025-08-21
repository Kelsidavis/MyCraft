#include "ImageLoader.h"
#include <fstream>
#include <iostream>
#include <sstream>

bool ImageLoader::loadPPM(const std::string& filename, ImageData& image) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Failed to open image file: " << filename << std::endl;
        return false;
    }
    
    std::string magic;
    file >> magic;
    
    if (magic != "P6") {
        std::cout << "Unsupported PPM format: " << magic << " (need P6)" << std::endl;
        return false;
    }
    
    int maxval;
    file >> image.width >> image.height >> maxval;
    
    if (maxval != 255) {
        std::cout << "Unsupported max value: " << maxval << " (need 255)" << std::endl;
        return false;
    }
    
    // Skip whitespace
    file.get();
    
    // Read pixel data
    image.data.resize(image.width * image.height * 3);
    file.read(reinterpret_cast<char*>(image.data.data()), image.data.size());
    
    if (!file) {
        std::cout << "Failed to read image data from: " << filename << std::endl;
        return false;
    }
    
    std::cout << "Loaded PPM image: " << filename << " (" << image.width << "x" << image.height << ")" << std::endl;
    return true;
}

ImageData ImageLoader::createSolidTexture(int width, int height, unsigned char r, unsigned char g, unsigned char b) {
    ImageData image(width, height);
    
    for (int i = 0; i < width * height; i++) {
        image.data[i * 3] = r;
        image.data[i * 3 + 1] = g;
        image.data[i * 3 + 2] = b;
    }
    
    return image;
}

ImageData ImageLoader::createPatternTexture(int width, int height, int pattern) {
    ImageData image(width, height);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 3;
            unsigned char r = 128, g = 128, b = 128;
            
            switch (pattern) {
                case 1: // Grass pattern
                    r = 50 + (x + y) % 100;
                    g = 150 + (x * y) % 105;
                    b = 50 + (x ^ y) % 50;
                    break;
                case 2: // Dirt pattern
                    r = 120 + (x * 3 + y * 2) % 60;
                    g = 80 + (x + y * 2) % 40;
                    b = 40 + (x ^ y) % 20;
                    break;
                case 3: // Stone pattern
                    r = g = b = 100 + ((x + y) * 7) % 80;
                    break;
                case 4: // Wood pattern
                    r = 120 + (y % 40);
                    g = 60 + (y % 20);
                    b = 20 + (y % 10);
                    break;
                case 5: // Water pattern
                    r = 50 + (x + y) % 30;
                    g = 100 + (x * y) % 50;
                    b = 200 + (x ^ y) % 55;
                    break;
                case 6: // Sand pattern
                    r = 220 + (x + y) % 35;
                    g = 200 + (x * 2) % 40;
                    b = 140 + (y * 3) % 50;
                    break;
                default: // Default gray
                    r = g = b = 128;
            }
            
            image.data[index] = r;
            image.data[index + 1] = g;
            image.data[index + 2] = b;
        }
    }
    
    return image;
}

bool ImageLoader::savePPM(const std::string& filename, const ImageData& image) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Failed to create file: " << filename << std::endl;
        return false;
    }
    
    // Write PPM header
    file << "P6\n" << image.width << " " << image.height << "\n255\n";
    
    // Write pixel data
    file.write(reinterpret_cast<const char*>(image.data.data()), image.data.size());
    
    std::cout << "Saved PPM image: " << filename << " (" << image.width << "x" << image.height << ")" << std::endl;
    return true;
}