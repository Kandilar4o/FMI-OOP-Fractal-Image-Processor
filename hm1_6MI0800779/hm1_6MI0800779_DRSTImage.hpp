#ifndef DRST_HPP
#define DRST_HPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>

struct Pixel {
    unsigned char rgb[3];
};

struct Position {
    double x, y;
};

class DRSTImage {

public:

    DRSTImage(std::istream& is);

    unsigned int getSize() const;

    Pixel getPixel(int imageIndex, int x, int y) const;

    Pixel getPixel(double x, double y) const;

    void saveAsPPM(const std::string& filename, size_t size) const;

    void saveAsPPM(const std::string& filename, size_t size, std::function<Position(Position)> phi) const;

private:

    static constexpr const char* magicConst = "DRST";
    int imagePixelLen;
    std::vector<std::vector<Pixel>> images;

};

#endif