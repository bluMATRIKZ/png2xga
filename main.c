#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct { int r, g, b; } RGB;
typedef struct { double l, a, b; } Lab;

RGB palette_rgb[16] = {
    {0,0,0}, {128,0,0}, {0,128,0}, {128,128,0},
    {0,0,128}, {128,0,128}, {0,128,128}, {192,192,192},
    {128,128,128}, {255,0,0}, {0,255,0}, {255,255,0},
    {0,0,255}, {255,0,255}, {0,255,255}, {255,255,255}
};

Lab palette_lab[16];

Lab rgb_to_lab(RGB c) {
    double r = c.r / 255.0, g = c.g / 255.0, b = c.b / 255.0;
    double x = r*.4124 + g*.3576 + b*.1805;
    double y = r*.2126 + g*.7152 + b*.0722;
    double z = r*.0193 + g*.1192 + b*.9505;
    x /= 0.95047; y /= 1.0; z /= 1.08883;
    x = x > .008856 ? pow(x, 1.0/3) : (7.787*x + 16.0/116);
    y = y > .008856 ? pow(y, 1.0/3) : (7.787*y + 16.0/116);
    z = z > .008856 ? pow(z, 1.0/3) : (7.787*z + 16.0/116);
    return (Lab){(116*y)-16, 500*(x-y), 200*(y-z)};
}

int closest_index_lab(RGB c) {
    Lab t = rgb_to_lab(c);
    int best = 0;
    double best_dist = 1e9;
    for (int i = 0; i < 16; ++i) {
        double dl = t.l - palette_lab[i].l;
        double da = t.a - palette_lab[i].a;
        double db = t.b - palette_lab[i].b;
        double dist = dl*dl + da*da + db*db;
        if (dist < best_dist) {
            best = i;
            best_dist = dist;
        }
    }
    return best;
}

int clamp(int v) { return v < 0 ? 0 : v > 255 ? 255 : v; }

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input.png output.xga\n", argv[0]);
        return 1;
    }

    for (int i = 0; i < 16; ++i)
        palette_lab[i] = rgb_to_lab(palette_rgb[i]);

    int w, h, ch;
    unsigned char* img = stbi_load(argv[1], &w, &h, &ch, 3);
    if (!img) { fprintf(stderr, "Failed to load image.\n"); return 1; }

    RGB* buffer = malloc(w * h * sizeof(RGB));
    int* indices = malloc(w * h * sizeof(int));
    for (int i = 0; i < w * h; ++i)
        buffer[i] = (RGB){ img[i*3], img[i*3+1], img[i*3+2] };

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            RGB old = buffer[idx];
            int pi = closest_index_lab(old);
            indices[idx] = pi;
            RGB new = palette_rgb[pi];
            int err_r = old.r - new.r;
            int err_g = old.g - new.g;
            int err_b = old.b - new.b;

            buffer[idx] = new;

            int dx[] = {1, -1, 0, 1};
            int dy[] = {0, 1, 1, 1};
            float wgt[] = {7/16.f, 3/16.f, 5/16.f, 1/16.f};
            for (int k = 0; k < 4; ++k) {
                int nx = x + dx[k], ny = y + dy[k];
                if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                    int ni = ny * w + nx;
                    buffer[ni].r = clamp(buffer[ni].r + err_r * wgt[k]);
                    buffer[ni].g = clamp(buffer[ni].g + err_g * wgt[k]);
                    buffer[ni].b = clamp(buffer[ni].b + err_b * wgt[k]);
                }
            }
        }
    }

    FILE* out = fopen(argv[2], "w");
    if (!out) { fprintf(stderr, "Failed to open output file.\n"); return 1; }

    fprintf(out, "%dx%d;", w, h);
    int count = 1, last = indices[0];
    for (int i = 1; i < w * h; ++i) {
        if (indices[i] == last) {
            count++;
        } else {
            if (count > 1) fprintf(out, "%d%c", count, 'a' + last);
            else fprintf(out, "%c", 'a' + last);
            count = 1;
            last = indices[i];
        }
    }
    if (count > 1) fprintf(out, "%d%c\n", count, 'a' + last);
    else fprintf(out, "%c\n", 'a' + last);

    fclose(out);
    free(buffer);
    free(indices);
    stbi_image_free(img);

    printf("✓ Converted %s to %s (%dx%d, 16-color XGA)\n", argv[1], argv[2], w, h);
    return 0;
}
