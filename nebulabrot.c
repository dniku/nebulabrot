#include <stdio.h>
#include <assert.h>
#include <string.h> // memset
#include <complex.h>
#include <time.h> // clock, CLOCKS_PER_SEC

#define _USE_MATH_DEFINES
#include <math.h>   // M_PI

#include "bmp.h"    // write_bmp
#include "my_math.h" // min, max, sqr, linear_scale
#include "colors.h"


#define SCALE 2
#define IMAGE_WIDTH (1366 * SCALE)
#define IMAGE_HEIGHT (768 * SCALE)

#define VIEW_WIDTH 4.0
#define VIEW_CENTER_X -0.5
#define VIEW_CENTER_Y 0.0

#define MAX_ITERATIONS 8192
#define BAILOUT 3.0
#define PRECISION 4

double multiplier = 0.0;
double VIEW_HEIGHT;

double START_X, START_Y;
double STEP_X, STEP_Y;

double complex counter[IMAGE_HEIGHT][IMAGE_WIDTH];
COLOR image[IMAGE_HEIGHT][IMAGE_WIDTH];

inline int in_cardioid(double x, double y) {
    double q = sqr(x - 0.25) + sqr(y);
    return q * (q + (x - 0.25)) < sqr(y) / 4.0;
}

inline int in_bulb(double x, double y) {
    return sqr(x + 1) + sqr(y) < 0.0625;
}

int in_set(double complex c) {
    double complex z = c;
    double x = creal(c);
    double y = cimag(c);
    int i;
    
    if (in_cardioid(x, y) || in_bulb(x, y)) {
        return 1;
    }

    for(i = 0; i < MAX_ITERATIONS; i++) {
        if (cabs(z) > BAILOUT) {
            return 0;
        }
        z = z*z + c;
    }
    return 1;
}

void process_point(double x, double y) {
    double complex c = x + I * y;
    double complex z = c;
    int within = in_set(c);
    int ix, iy;
    int i;

    if (!within) {
        return;
    }

    for(i = 0; i < MAX_ITERATIONS; i++) {
        // if (cabs(z) > BAILOUT) {
        //     return;
        // }
        z = z*z + c;
        x = creal(z);
        y = cimag(z);
        ix = (x - START_X) * IMAGE_WIDTH / VIEW_WIDTH;
        iy = (y - START_Y) * IMAGE_HEIGHT / VIEW_HEIGHT;
        if (ix < 0 || ix >= IMAGE_WIDTH || iy < 0 || iy >= IMAGE_HEIGHT) {
            return;
        }
        counter[iy][ix] += c;
    }
}

void calculate_nebulabrot() {
    const double END_X = START_X + VIEW_WIDTH;
    const double END_Y = START_Y + VIEW_HEIGHT;
    const int TOTAL_ROWS = VIEW_HEIGHT / STEP_Y;
    double cx, cy;
    int row_count = 1;
    for(cy = START_Y; cy <= END_Y; cy += STEP_Y) {
        printf("calculating row %d/%d...\n", row_count++, TOTAL_ROWS);
    	for(cx = START_X; cx < END_X; cx += STEP_X) {
    		process_point(cx, cy);
    	}
    }
}

inline double scale_function(double x) {
    return cbrt(1.0 - (sqr(x - 1.0)));
}

void render_nebulabrot() {
    int x, y;
    COLOR color;
    double max_abs = 0.0;

    for(y = 0; y < IMAGE_HEIGHT; y++) {
        for(x = 0; x < IMAGE_WIDTH; x++) {
            max_abs = max(max_abs, cabs(counter[y][x]));
        }
    }

    printf("max_abs = %f\n", max_abs);

    for(y = 0; y < IMAGE_HEIGHT; y++) {
        for(x = 0; x < IMAGE_WIDTH; x++) {
            double complex c = counter[y][x];
            double h = (carg(c) + M_PI) / (2.0 * M_PI);
            double s = 0.6;
            double l = scale_function(cabs(c) / max_abs); //min(cabs(c) / max_abs * 5.0, 1.0); // 5.0 - magic number, determined experimentally
            HSL2RGB(&color, h, s, l);
            image[y][x] = color;
        }
    }
}

int main() {
    char filename[50] = "nebulabrot.bmp";

    VIEW_HEIGHT = IMAGE_HEIGHT * VIEW_WIDTH / IMAGE_WIDTH;
    START_X = VIEW_CENTER_X - VIEW_WIDTH / 2;
    START_Y = VIEW_CENTER_Y - VIEW_HEIGHT / 2;
    STEP_X = VIEW_WIDTH / (IMAGE_WIDTH * PRECISION);
    STEP_Y = VIEW_HEIGHT / (IMAGE_HEIGHT * PRECISION);
    
    memset(counter, 0.0 + I * 0.0, IMAGE_WIDTH * IMAGE_HEIGHT);
    printf("calculating...\n");
    calculate_nebulabrot();
    printf("rendering...\n");
    //for (multiplier = 1.0; multiplier < 30.25; multiplier += 0.5) {
    render_nebulabrot();
    //sprintf(filename, "test\\nebulabrot.bmp");
    printf("saving to %s...\n", filename);
    write_bmp(filename, IMAGE_WIDTH, IMAGE_HEIGHT, (char*)image);
    //}
    
    printf("Total time: %f seconds", ((float)clock()) / CLOCKS_PER_SEC);
	return 0;
}