#ifndef SIMPLEXNOISE_H_
#define SIMPLEXNOISE_H_

#include "../Random.h"
#include <cmath>

class SimplexNoise {
private:
    static const int grad3[12][3];
    static const int grad2[8][2];
    static const int permSize = 512;

    int perm[permSize];
    trn::ptat::Random* rand;

    void init();

public:
    SimplexNoise(trn::ptat::Random* r);

    // 1D Simplex noise
    float noise1(double x) const;

    // 2D Simplex noise
    float noise2(double x, double y) const;
};

#endif /* SIMPLEXNOISE_H_ */
