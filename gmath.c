#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gmath.h"
#include "matrix.h"
#include "ml6.h"

/*============================================
  IMPORANT NOTE
  Ambient light is represeneted by a color value
  Point light sources are 2D arrays of doubles.
       - The fist index (LOCATION) represents the vector to the light.
       - The second index (COLOR) represents the color.
  Reflection constants (ka, kd, ks) are represened as arrays of
  doubles (red, green, blue)
  ============================================*/

// Lighting functions
color get_lighting(double * normal, double * view, color ambient, color point, double * light, double * areflect, double * dreflect, double * sreflect) {
	color a = calculate_ambient(ambient, areflect);
	color d = calculate_diffuse(point, dreflect, normal, light);
	color s = calculate_specular(point, sreflect, view, normal, light);

	color i;

	i.red = a.red + d.red + s.red;
	i.green = a.green + d.green + s.green;
	i.blue = a.blue + d.blue + s.blue;

	limit_color(&i);

	return i;
}

color calculate_ambient(color ambient, double * areflect) {
	color a;

	a.red = ambient.red * areflect[RED];
	a.green = ambient.green * areflect[GREEN];
	a.blue = ambient.blue * areflect[BLUE];

	return a;
}

color calculate_diffuse(color point, double * dreflect, double * normal, double * light) {
	normalize(normal);
	normalize(light);

	double reflection = dot_product(normal, light);
	if (reflection < 0) reflection = 0;
	
	color d;

	d.red = point.red * dreflect[RED] * reflection;
	d.green = point.green * dreflect[GREEN] * reflection;
	d.blue = point.blue * dreflect[BLUE] * reflection;

	return d;
}

color calculate_specular(color point, double * sreflect, double * view, double * normal, double * light) {
	normalize(normal);
	normalize(light);
	normalize(view);

	double costheta = dot_product(normal, view);
	if (costheta < 0) costheta = 0;

	double R[3];
	R[0] = 2 * normal[0] * costheta - light[0];
	R[1] = 2 * normal[1] * costheta - light[1];
	R[2] = 2 * normal[2] * costheta - light[2];
	normalize(R);

	double reflection = dot_product(R, view);
	if (reflection < 0) reflection = 0;
	
	color s;

	s.red = point.red * sreflect[RED] * pow(reflection, SPECULAR_EXP);
	s.green = point.green * sreflect[GREEN] * pow(reflection, SPECULAR_EXP);
	s.blue = point.blue * sreflect[BLUE] * pow(reflection, SPECULAR_EXP);

	return s;
}

// Limit each component of c to a max of 255
void limit_color(color * c) {
	int r = c -> red;
	int g = c -> green;
	int b = c -> blue;

	if (r < 0) c -> red = 0;
	if (g < 0) c -> green = 0;
	if (b < 0) c -> blue = 0;

	if (r > 255) c -> red = 255;
	if (g > 255) c -> green = 255;
	if (b > 255) c -> blue = 255;
}

// Vector functions
// Normalize vetor, should modify the parameter
void normalize(double * vector) {
	double dp = dot_product(vector, vector);
	double magnitude = sqrt(dp);

	for (int i = 0; i < 3; i++) {
		vector[i] = vector[i] / magnitude;
	}
}

// Return the dot product of a . b
double dot_product(double * a, double * b) {
	return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

// Calculate the surface normal for the triangle whose first
// point is located at index i in polygons
double * calculate_normal(struct matrix * polygons, int i) {
	double ** matrix = polygons -> m;
	double * norm = malloc(3 * sizeof(double));
  	double a[3];
  	double b[3];

  	a[0] = matrix[0][i + 1] - matrix[0][i];
  	a[1] = matrix[1][i + 1] - matrix[1][i];
  	a[2] = matrix[2][i + 1] - matrix[2][i];

  	b[0] = matrix[0][i + 2] - matrix[0][i];
  	b[1] = matrix[1][i + 2] - matrix[1][i];
  	b[2] = matrix[2][i + 2] - matrix[2][i];

  	norm[0] = (a[1] * b[2]) - (a[2] * b[1]);
  	norm[1] = (a[2] * b[0]) - (a[0] * b[2]);
  	norm[2] = (a[0] * b[1]) - (a[1] * b[0]);

  	return norm;
}