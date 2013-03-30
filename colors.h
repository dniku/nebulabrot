typedef struct {
    unsigned char r, g, b;
} COLOR;

void HSV2RGB(COLOR* color, unsigned int i_hue, unsigned char sat, unsigned char val);
void HSL2RGB(COLOR* color, double h, double sl, double l);