#include "hm1_6MI0800779_DRSTImage.hpp"
#include <fstream>
#include <cmath>
#include <cstring>

constexpr double center = 0.5;
constexpr double repeatScale = 16;
constexpr double frameScale = 2;
constexpr int imagesPerRowAndCol = 4;
constexpr int imagesPerFrame = imagesPerRowAndCol * 4 - 4;
constexpr int imagesCount = 48;
constexpr double outerFrameOffset = 0.25;
constexpr double outerFrameLowerBound = center - outerFrameOffset;
constexpr double outerFrameHigherBound = center + outerFrameOffset;
constexpr double repeatOffset = center / repeatScale;
constexpr double repeatLowerBound = center - repeatOffset;
constexpr double repeatHigherBound = center + repeatOffset;

DRSTImage::DRSTImage(std::istream& is) : imagePixelLen(0){
    char magic[5];
    is.read(magic, 4);
    magic[4] = '\0';
    if (std::strcmp(magic, magicConst)) {
        std::cerr << "Not a DRST file!" << std::endl;
        return;
    }
    is.read(reinterpret_cast<char*>(&imagePixelLen), sizeof(imagePixelLen));

    long long int imagePixelSize = imagePixelLen * imagePixelLen;

    images.resize(48, std::vector<Pixel>(imagePixelSize));

    for (int i = 0; i < imagesCount; ++i) {
        for (long long int j = 0; j < imagePixelSize; j++) {
            unsigned char currpixel[3];
            is.read(reinterpret_cast<char*>(currpixel), 3);
            images[i][j] = { {currpixel[0], currpixel[1], currpixel[2]} };
        }
    }
}

unsigned int DRSTImage::getSize() const {
    return imagePixelLen * imagesPerRowAndCol;
}

Pixel DRSTImage::getPixel(int imageIndex, int x, int y) const {
    if (imageIndex < 0 || imageIndex >= imagesCount) {
        return { 0, 0, 0 };
    }
    if (x < 0 || x >= imagePixelLen || y < 0 || y >= imagePixelLen) {
        return { 0, 0, 0 };
    }
    return images[imageIndex][y * imagePixelLen + x];
}

void scalingImage(double& x, double& y, double lowerBound, double scale) {
    x = (x - lowerBound) * scale;
    y = (y - lowerBound) * scale;
}

void zoomingOut(double& x, double& y) {
    while (x < 0 || x > 1 || y < 0 || y > 1) {
        x = (x - center) / repeatScale + center;
        y = (y - center) / repeatScale + center;
    }
}

void zoomingIn(double& x, double& y) {
    while (x >= repeatLowerBound && x < repeatHigherBound && y >= repeatLowerBound && y < repeatHigherBound) {
        scalingImage(x, y, repeatLowerBound, repeatScale);
    }
}

int frameIndexCalculator(double& x, double& y) {
    int frameIndex = 0;
    while (x >= outerFrameLowerBound && x < outerFrameHigherBound && y >= outerFrameLowerBound && y < outerFrameHigherBound) {
        scalingImage(x, y, outerFrameLowerBound, frameScale);
        frameIndex++;
    }
    return frameIndex;
}

int imageIndexCalculator(double x, double y, int frameIndex) {
    int imageIndex = 0;
    if (y < outerFrameLowerBound || y >= outerFrameHigherBound) {
        imageIndex = static_cast<int>(x * imagesPerRowAndCol) + 1;
        if (y >= outerFrameHigherBound) {
            imageIndex += imagesPerFrame - imagesPerRowAndCol;
        }
    }
    else if (y < center) {
        imageIndex = imagesPerRowAndCol + (x < center ? 1 : 2);
    }
    else {
        imageIndex = imagesPerRowAndCol + (x < center ? 3 : 4);
    }

    imageIndex += frameIndex * imagesPerFrame - 1;
    return imageIndex;
}

void pixelPositionCalculator(double x, double y, int imagePixelLen, int& pixelX, int& pixelY) {
    double gridX = x * imagesPerRowAndCol;
    double gridY = y * imagesPerRowAndCol;
    int col = static_cast<int>(gridX);
    int row = static_cast<int>(gridY);

    if (col >= imagesPerRowAndCol) {
        col = imagesPerRowAndCol - 1;
    }
    if (row >= imagesPerRowAndCol) {
        row = imagesPerRowAndCol - 1;
    }

    double relativeX = gridX - col;
    double relativeY = gridY - row;
    pixelX = static_cast<int>(relativeX * imagePixelLen);
    pixelY = static_cast<int>(relativeY * imagePixelLen);
    if (pixelX == imagePixelLen) {
        pixelX = imagePixelLen - 1;
    }
    if (pixelY == imagePixelLen) {
        pixelY = imagePixelLen - 1;
    }
}

Pixel DRSTImage::getPixel(double x, double y) const {
    if (x == center && y == center) {
        return{ {0, 0, 0} };
    }
    zoomingOut(x, y);

    zoomingIn(x, y);

    int frameIndex = frameIndexCalculator(x, y);
  
    int imageIndex = imageIndexCalculator(x, y, frameIndex);

    int pixelX = 0;
    int pixelY = 0;

    pixelPositionCalculator(x, y, imagePixelLen, pixelX, pixelY);
    
    return getPixel(imageIndex, pixelX, pixelY);
}

void DRSTImage::saveAsPPM(const std::string& filename, size_t size) const {
    saveAsPPM(filename, size, [](Position p) -> Position{ return p; });
}

void DRSTImage::saveAsPPM(const std::string& filename, size_t size, std::function<Position(Position)> phi) const {
    std::ofstream out(filename, std::ios::binary);

    out << "P6\n" << size << " " << size << "\n255\n";

    for (size_t pixelY = 0; pixelY < size; pixelY++) {
        for (size_t pixelX = 0; pixelX < size; pixelX++) {
            double x = static_cast<double>(pixelX) / (size - 1);
            double y = static_cast<double>(pixelY) / (size - 1);
            Position pos = phi({ x, y });
            Pixel color = getPixel(pos.x, pos.y);
            unsigned char rgb[3] = { color.rgb[0], color.rgb[1], color.rgb[2]};
            out.write(reinterpret_cast<const char*>(rgb), 3);
        }
    }
}