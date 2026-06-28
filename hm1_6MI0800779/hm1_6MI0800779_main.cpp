#include "hm1_6MI0800779_DRSTImage.hpp"
#include <fstream>
#include <complex>
#include <iostream>

Position invert(Position pos) {
    return { 1.0 - pos.x, 1.0 - pos.y };
}

Position logRemap(Position pos) {
    using namespace std::complex_literals;
    std::complex<double> z(pos.x - 0.5, pos.y - 0.5);
    z = std::log(z);
    auto c = 0.915625 * std::exp(-0.41625i);
    auto z0 = -1.1 - 3.8i;
    z = (z - z0) / c + z0;
    z = std::exp(z);

    return { z.real() + 0.5, z.imag() + 0.5 };
}

int main() {
    std::ifstream input("cats.drst", std::ios::binary);
    if (!input) {
        std::cerr << "Wrong input!" << std::endl;
        return 1;
    }

    DRSTImage img(input);

    img.saveAsPPM("original.ppm", 2048);
    std::cout << "Saved original.ppm" << std::endl;

    img.saveAsPPM("inverted.ppm", 2048, invert);
    std::cout << "Saved inverted.ppm" << std::endl;

    img.saveAsPPM("twisted.ppm", 2048, logRemap);
    std::cout << "Saved twisted.ppm" << std::endl;

    return 0;
}