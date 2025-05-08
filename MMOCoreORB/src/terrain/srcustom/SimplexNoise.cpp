#include "SimplexNoise.h"

// Gradients for 3D (not used, but kept for possible extension)
const int SimplexNoise::grad3[12][3] = {
    {1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
    {1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
    {0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1}
};

// Gradients for 2D
const int SimplexNoise::grad2[8][2] = {
    {1,1}, {-1,1}, {1,-1}, {-1,-1},
    {1,0}, {-1,0}, {0,1}, {0,-1}
};

SimplexNoise::SimplexNoise(trn::ptat::Random* r) : rand(r) {
    init();
}

void SimplexNoise::init() {
    // Fill perm array with values from 0 to 255, then shuffle using project RNG
    for (int i = 0; i < 256; ++i) {
        perm[i] = i;
    }
    for (int i = 255; i > 0; --i) {
        int j = rand->next() % (i + 1);
        int tmp = perm[i];
        perm[i] = perm[j];
        perm[j] = tmp;
    }
    // Duplicate the permutation array
    for (int i = 0; i < 256; ++i) {
        perm[256 + i] = perm[i];
    }
}

// Helper: fast floor
static int fastfloor(double x) {
    return x > 0 ? (int)x : (int)x - 1;
}

// Helper: dot product for 2D
static float dot(const int* g, double x, double y) {
    return (float)(g[0] * x + g[1] * y);
}

// Helper: dot product for 1D
static float dot1(const int* g, double x) {
    return (float)(g[0] * x);
}

// 1D Simplex noise
float SimplexNoise::noise1(double x) const {
    int i0 = fastfloor(x);
    int i1 = i0 + 1;
    double x0 = x - i0;
    double x1 = x0 - 1.0;

    double t0 = 1.0 - x0 * x0;
    double n0 = 0.0;
    if (t0 > 0.0) {
        t0 *= t0;
        int gi0 = perm[i0 & 0xFF] & 7;
        n0 = t0 * t0 * dot1(grad2[gi0], x0);
    }

    double t1 = 1.0 - x1 * x1;
    double n1 = 0.0;
    if (t1 > 0.0) {
        t1 *= t1;
        int gi1 = perm[i1 & 0xFF] & 7;
        n1 = t1 * t1 * dot1(grad2[gi1], x1);
    }

    // Scale to match Perlin's [-1,1] range
    return 0.395f * (float)(n0 + n1);
}

// 2D Simplex noise
float SimplexNoise::noise2(double xin, double yin) const {
    // Skewing and unskewing factors for 2D
    const double F2 = 0.5 * (sqrt(3.0) - 1.0);
    const double G2 = (3.0 - sqrt(3.0)) / 6.0;

    double s = (xin + yin) * F2;
    int i = fastfloor(xin + s);
    int j = fastfloor(yin + s);

    double t = (i + j) * G2;
    double X0 = i - t;
    double Y0 = j - t;
    double x0 = xin - X0;
    double y0 = yin - Y0;

    int i1, j1;
    if (x0 > y0) {
        i1 = 1; j1 = 0;
    } else {
        i1 = 0; j1 = 1;
    }

    double x1 = x0 - i1 + G2;
    double y1 = y0 - j1 + G2;
    double x2 = x0 - 1.0 + 2.0 * G2;
    double y2 = y0 - 1.0 + 2.0 * G2;

    int ii = i & 0xFF;
    int jj = j & 0xFF;

    int gi0 = perm[ii + perm[jj]] & 7;
    int gi1 = perm[ii + i1 + perm[jj + j1]] & 7;
    int gi2 = perm[ii + 1 + perm[jj + 1]] & 7;

    double t0 = 0.5 - x0 * x0 - y0 * y0;
    double n0 = 0.0;
    if (t0 > 0.0) {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad2[gi0], x0, y0);
    }

    double t1 = 0.5 - x1 * x1 - y1 * y1;
    double n1 = 0.0;
    if (t1 > 0.0) {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad2[gi1], x1, y1);
    }

    double t2 = 0.5 - x2 * x2 - y2 * y2;
    double n2 = 0.0;
    if (t2 > 0.0) {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad2[gi2], x2, y2);
    }

    // Scale to match Perlin's [-1,1] range
    return 70.0f * (float)(n0 + n1 + n2);
}
