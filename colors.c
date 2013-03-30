#include "colors.h"

void HSV2RGB(COLOR* color, unsigned int i_hue, unsigned char sat, unsigned char val) {
/*
* 0 <= hue < 360
* 0 <= sat <= 255
* 0 <= val <= 255
*/
    int ii;
    double fr, hue;
    unsigned char c1, c2, c3;
    if (!sat)
        color->r = color->g = color->b = val;
    else {
        hue = i_hue;
        ii = (int)(hue /= 60.0);
        fr = hue - ii;
        c1 = (val * (255 - sat)) / 255;
        c2 = (val * (255 - sat * fr)) / 255;
        c3 = (val * (255 - sat * (1.0 - fr))) / 255;
        switch (ii) {
            case 0: color->r = val; color->g = c3;  color->b = c1;  break;
            case 1: color->r = c2;  color->g = val; color->b = c1;  break;
            case 2: color->r = c1;  color->g = val; color->b = c3;  break;
            case 3: color->r = c1;  color->g = c2;  color->b = val; break;
            case 4: color->r = c3;  color->g = c1;  color->b = val; break;
            case 5: color->r = val; color->g = c1;  color->b = c2;  break;
        }
    }
}

void HSL2RGB(COLOR* color, double h, double sl, double l) {
    double r = l, g = l, b = l; // default to gray
    double v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
    if (v > 0) {
        double m;
        double sv;
        int sextant;
        double fract, vsf, mid1, mid2;

        m = l + l - v;
        sv = (v - m) / v;
        h *= 6.0;
        sextant = (int)h;
        fract = h - sextant;
        vsf = v * sv * fract;
        mid1 = m + vsf;
        mid2 = v - vsf;
        switch (sextant) {
                case 0: r = v;    g = mid1; b = m;    break;
                case 1: r = mid2; g = v;    b = m;    break;
                case 2: r = m;    g = v;    b = mid1; break;
                case 3: r = m;    g = mid2; b = v;    break;
                case 4: r = mid1; g = m;    b = v;    break;
                case 5: r = v;    g = m;    b = mid2; break;
        }
    }
    color->r = r * 255.0;
    color->g = g * 255.0;
    color->b = b * 255.0;
}