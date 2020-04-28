#ifndef GMATH_H
#define GMATH_H

#include "matrix.h"
#include "ml6.h"

// Constants for lighting
#define AMBIENT 0
#define DIFFUSE 1
#define SPECULAR 2
#define LOCATION 0
#define COLOR 1
#define RED 0
#define GREEN 1
#define BLUE 2
#define SPECULAR_EXP 4

// Lighting functions
color get_lighting(double * normal, double * view, color ambient, color point, double * light, double * areflect, double * dreflect, double * sreflect);
color calculate_ambient(color ambient, double * areflect);
color calculate_diffuse(color point, double * dreflect, double * normal, double * light);
color calculate_specular(color point, double * sreflect, double * view, double * normal, double * light);
void limit_color(color * c);

// Vector functions
void normalize(double * vector);
double dot_product(double * a, double * b);
double * calculate_normal(struct matrix * polygons, int i);

#endif