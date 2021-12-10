#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <omp.h>
#include "debugbmp.h"
#include "timing.h"

size_t pos(size_t width, size_t x, size_t y) {
    return y * width + x;
}

void write_borders(float* data, size_t width, size_t height) {
    size_t n;
    for (n = 0; n < width; n++){
        data[pos(width, n , 0)] = 20.0;
        data[pos(width, n, height-1)] = -273.15;
    }
    for (n = 0; n < height; n++){
        data[pos(width, 0, n)] = -273.15;
        data[pos(width, width-1,n)] = -273.15;
    }

}

float stencil(float* data, size_t width, size_t x, size_t y, float alpha) {
    return alpha * (data[pos(width, x,y)] + data[pos(width, x-1,y)] + data[pos(width, x+1,y)] + data[pos(width, x,y-1)] + data[pos(width, x,y+1)]);
}

// change order of loop: 1.3 sec -> 1.1 sec (1000,1000,100)  (only slightly better locality)
// parallel outer loop: 1.1 sec -> 0.6, with 2 threads
void apply_stencil(float* data, size_t width, size_t height, size_t offset, float alpha) {
    #pragma omp parallel for
    for (size_t y = 1; y < height-1; y++){
        for (size_t x = 1 + ((y+offset)%2); x < width-1; x+=2){
            data[pos(width, x, y)] = stencil(data, width, x, y, alpha);
        }
    }
}

// changed the order of the loops to get better locality
// 1.7 sec -> 0.8 sec, with (1000,1000,100)
// with parallel nested loop: 0.8 sec -> 0.4 sec, with 2 threads
float compute_delta(float* data, float* prev, size_t width, size_t height) {
    float res = 0.0;
    #pragma omp parallel for reduction(+:res) collapse(2)
    for (size_t y = 0; y < height; y++){
        for (size_t x = 0; x < width; x++){
            res += fabs(prev[pos(width, x, y)]-data[pos(width, x, y)]);
        }
    }
    return res / (width*height);
}


void run_simulation(size_t width, size_t height, size_t steps, const char* filename) {
    size_t size = width*height;
    float* data = malloc(size * sizeof(float));
    float* prev = malloc(size * sizeof(float));

    double t0, t1, t2=0, t3=0, t4=0, t5=0;
    double bef = seconds();
    memset(data, 0, size * sizeof(float));
    double aft = seconds();
    t0 = aft-bef;
    printf("%8s: %f\n", "memset", t0);

    bef = seconds();
    write_borders(data, width, height);
    aft = seconds();
    t1 = aft-bef;
    printf("%8s: %f\n", "borders", t1);

    float delta = 0.0f;
    size_t n = 0;

    
    for(; n < steps; n++) {
        bef = seconds();
        memcpy(prev, data, size*sizeof(float));
        aft = seconds();
        t2 += aft-bef;

        bef = seconds();
        apply_stencil(data, width, height, n % 2, 0.2f);
        aft = seconds();
        t3 += aft-bef;

        bef = seconds();
        delta = compute_delta(data, prev, width, height);
        aft = seconds();
        t4 += aft-bef;

        if (delta < 0.001f)
            break;
    }
   
    printf("%8s: %f\n", "memcpy", t2);
    printf("%8s: %f\n", "stencil", t3);
    printf("%8s: %f\n", "delta", t4);

    if (filename != NULL) {
        bef = seconds();
        debugbmp_writebmp(filename, (int)width, (int)height, data);
        aft = seconds();
        t5 = aft-bef;
        printf("%8s: %f\n", "to bmp", t5);
    }
    printf("\ntotal time:        %f\n", t0+t1+t2+t3+t4+t5);
    printf("p in Amdahl's law: %f\n", (t3+t4)/(t0+t1+t2+t3+t4+t5));

    printf("\nAfter %lu iterations, delta was %f\n", n, delta);

    free(data);
    free(prev);
}

int main(int argc, char** argv) {
    if (argc != 4 && argc != 5) {
        fprintf(stderr, "Usage: %s <width> <height> <steps> [output-file]\n", argv[0]);
        return 1;
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int steps = atoi(argv[3]);

    if (width <= 0 || height <= 0) {
        fprintf(stderr, "Sizes must be positive integers\n");
        return 1;
    }

    if (steps < 0) {
        fprintf(stderr, "Steps must be non-negative\n");
        return 1;
    }

    char* filename = NULL;
    if (argc == 5) {
        filename = argv[4];
    }

    run_simulation((size_t)width, (size_t)height, (size_t)steps, filename);
    return 0;
}


