import datetime
import pathlib

import numpy as np
import numba
from PIL import Image


# Size in pixels
SCALE = 2
IMAGE_WIDTH = 1366 * SCALE
IMAGE_HEIGHT = 768 * SCALE

# Size in units
VIEW_WIDTH = 4.0
VIEW_HEIGHT = IMAGE_HEIGHT * VIEW_WIDTH / IMAGE_WIDTH
VIEW_CENTER_X = -0.5
VIEW_CENTER_Y = 0.0
START_X = VIEW_CENTER_X - VIEW_WIDTH / 2
START_Y = VIEW_CENTER_Y - VIEW_HEIGHT / 2

# Calculation options
MAX_ITERATIONS = 8192
BAILOUT = 3.0
PRECISION = 4  # Subpixels per real pixel
NUM_SUBPIXELS_X = IMAGE_WIDTH * PRECISION
NUM_SUBPIXELS_Y = IMAGE_HEIGHT * PRECISION


@numba.jit(numba.boolean(numba.float64, numba.float64), fastmath=True)
def in_cardioid(x: numba.float64, y: numba.float64) -> numba.boolean:
    """Check if a point is in the main cardioid section of the Mandelbrot set."""
    q = (x - 0.25)**2 + y**2
    return q * (q + (x - 0.25)) < y**2 / 4.0


@numba.jit(numba.boolean(numba.float64, numba.float64), fastmath=True)
def in_bulb(x: numba.float64, y: numba.float64) -> numba.boolean:
    """Check if a point is in the main bulb of the Mandelbrot set."""
    return (x + 1)**2 + y**2 < 0.0625


@numba.jit(fastmath=True, parallel=True)
def calculate_nebulabrot() -> np.ndarray:
    accumulator = np.zeros((IMAGE_HEIGHT, IMAGE_WIDTH), dtype=np.complex128)

    subpixels_y = np.linspace(START_Y, START_Y + VIEW_HEIGHT, NUM_SUBPIXELS_Y)
    for subpixel_row, cy in enumerate(subpixels_y):
        print(f'Calculating row {subpixel_row}/{NUM_SUBPIXELS_Y}')

        subpixels_x = np.linspace(START_X, START_X + VIEW_WIDTH, NUM_SUBPIXELS_X)
        for subpixel_col in numba.prange(NUM_SUBPIXELS_X):
            cx = subpixels_x[subpixel_col]
            c = complex(cx, cy)
            
            # Quickly discard points inside the prominent parts of the Mandelbrot set
            if in_cardioid(cx, cy) or in_bulb(cx, cy):
                continue

            # Iterate first time, detect if the point is in Mandelbrot set
            in_set = True
            z = c
            for _ in range(MAX_ITERATIONS):
                if abs(z) > BAILOUT:
                    in_set = False
                    break
                z = z*z + c
            
            if in_set:
                continue

            # For points outside the set, iterate a second time
            z = c
            for _ in range(MAX_ITERATIONS):
                if abs(z) > BAILOUT:
                    break

                z = z*z + c
                ix = int((z.real - START_X) * IMAGE_WIDTH / VIEW_WIDTH)
                iy = int((z.imag - START_Y) * IMAGE_HEIGHT / VIEW_HEIGHT)

                if not ((0 <= ix < IMAGE_WIDTH) and (0 <= iy < IMAGE_HEIGHT)):
                    break

                # The following line is a race condition, but
                # it doesn't seem to affect the final output much.
                accumulator[iy, ix] += c

    return accumulator


@numba.jit(fastmath=True)
def hsl_to_rgb(
    h: numba.float64, sl: numba.float64, l: numba.float64
) -> tuple[numba.float64, numba.float64, numba.float64]:
    r, g, b = l, l, l  # default to gray
    v = (l * (1.0 + sl)) if l <= 0.5 else (l + sl - l * sl)
    
    if v > 0:
        m = 2 * l - v
        sv = (v - m) / v
        h *= 6.0
        sextant = int(h)
        fract = h - sextant
        vsf = v * sv * fract
        mid1 = m + vsf
        mid2 = v - vsf
        
        if sextant == 0:
            r, g, b = v, mid1, m
        elif sextant == 1:
            r, g, b = mid2, v, m
        elif sextant == 2:
            r, g, b = m, v, mid1
        elif sextant == 3:
            r, g, b = m, mid2, v
        elif sextant == 4:
            r, g, b = mid1, m, v
        elif sextant == 5:
            r, g, b = v, m, mid2
    
    return r, g, b


@numba.jit(fastmath=True)
def scale_function(x: numba.float64) -> numba.float64:
    # return (1 - (x - 1)**2)**(1/3)
    return np.minimum(x * 5, 1)


@numba.jit(fastmath=True)
def render_nebulabrot(accumulator: np.ndarray) -> np.ndarray:
    max_abs = np.abs(accumulator).max()

    image = np.zeros((IMAGE_HEIGHT, IMAGE_WIDTH, 3), dtype=np.uint8)
    for y in range(IMAGE_HEIGHT):
        for x in range(IMAGE_WIDTH):
            c = accumulator[y][x]

            h = (np.angle(c) + np.pi) / (2 * np.pi)
            s = 0.6
            l = scale_function(np.abs(c) / max_abs)
            r, g, b = hsl_to_rgb(h, s, l)
            
            image[y][x] = round(r * 255), round(g * 255), round(b * 255)
    return image


def main() -> None:
    start = datetime.datetime.now()
    accumulator = calculate_nebulabrot()
    print(f'Calculated in {datetime.datetime.now() - start}')

    start = datetime.datetime.now()
    accumulator_path = pathlib.Path('accumulator')
    np.save(accumulator_path, accumulator)
    print(f'Saved accumulator to {accumulator_path} in {datetime.datetime.now() - start}')

    start = datetime.datetime.now()
    image = render_nebulabrot(accumulator)
    print(f'Rendered in {datetime.datetime.now() - start}')

    start = datetime.datetime.now()
    image_path = pathlib.Path('nebulabrot.png')
    Image.fromarray(image).save(image_path)
    print(f'Saved image to {image_path} in {datetime.datetime.now() - start}')


if __name__ == "__main__":
    main()
