#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "matrix.h"
#include "parser.h"
#include "stack.h"

/*======== void parse_file () ==========
Inputs:   char * filename
          struct stack * csystems
          struct matrix * edges,
          struct matrix * polygons,
          screen s, zbuffer zb
Returns:
Goes through the file named filename and performs all of the actions listed in that file.
The file follows the following format:
     Every command is a single character that takes up a line
     Any command that requires arguments must have those arguments in the second line.
     The commands are as follows:
     push: push a copy of the curent top of the coordinate system stack to the stack
     pop: remove the current top of the coordinate system stack
     All the shape commands work as follows:
        1) Add the shape to a temporary matrix
        2) Multiply that matrix by the current top of the coordinate system stack
        3) Draw the shape to the screen
        4) Clear the temporary matrix
         sphere: add a sphere to the POLYGON matrix -
                 takes 4 arguemnts (cx, cy, cz, r)
         torus: add a torus to the POLYGON matrix -
                takes 5 arguemnts (cx, cy, cz, r1, r2)
         box: add a rectangular prism to the POLYGON matrix -
              takes 6 arguemnts (x, y, z, width, height, depth)
         clear: clears the edge and POLYGON matrices
         circle: add a circle to the edge matrix -
                 takes 4 arguments (cx, cy, cz, r)
         hermite: add a hermite curve to the edge matrix -
                  takes 8 arguments (x0, y0, x1, y1, rx0, ry0, rx1, ry1)
         bezier: add a bezier curve to the edge matrix -
                 takes 8 arguments (x0, y0, x1, y1, x2, y2, x3, y3)
         line: add a line to the edge matrix -
               takes 6 arguemnts (x0, y0, z0, x1, y1, z1)
         ident: set the transform matrix to the identity matrix -
         scale: create a scale matrix,
                then multiply the transform matrix by the scale matrix -
                takes 3 arguments (sx, sy, sz)
         move: create a translation matrix,
               then multiply the transform matrix by the translation matrix -
               takes 3 arguments (tx, ty, tz)
         rotate: create a rotation matrix,
                 then multiply the transform matrix by the rotation matrix -
                 takes 2 arguments (axis, theta) axis should be x y or z
         apply: apply the current transformation matrix to the edge and
                POLYGON matrices
         display: clear the screen, then
                  draw the lines of the edge and POLYGON matrices to the screen
                  display the screen
         save: clear the screen, then
               draw the lines of the edge and POLYGON matrices to the screen
               save the screen to a file -
               takes 1 argument (file name)
         quit: end parsing
See the file script for an example of the file format
IMPORTANT MATH NOTE:
the trig functions int math.h use radian mesure, but us normal
humans use degrees, so the file will contain degrees for rotations,
be sure to conver those degrees to radians (M_PI is the constant
for PI)
====================*/
void parse_file(char * filename,
                struct stack * csystems,
                struct matrix * edges,
                struct matrix * polygons,
                screen s, zbuffer zb,
                double * view, color ambient, color point, double * light,
                double * areflect, double * sreflect, double * dreflect) {
    // Init
    FILE *f;
    char line[256];

    clear_screen(s);
    clear_zbuffer(zb);
    int SIZE = 1500;
    color c;
    change_color(&c, 0, 0, 0);

    int linestep = 100;
    int polystep = 20;

    // Open script file
    if (strcmp(filename, "stdin") == 0) f = stdin;
    else f = fopen(filename, "r");

    // Copy script lines to a matrix
    char lines[SIZE][256];
    int counter = 0;

    while (fgets(line, 255, f) != NULL) {
        line[strlen(line)-1]='\0';
        strcpy(lines[counter++], line);
    }
    // Fill empty spaces in matrix with null chars
    while (counter < SIZE) {
        strcpy(lines[counter++], "\0");
    }

    for (int i = 0; i < SIZE; i++) {
        // End Script
        if (!strcmp(lines[i], "quit")) break;
        if (!strcmp(lines[i], "\0") && !strcmp(lines[i + 1], "\0")) break;

        // Stack Commands
        else if (!strcmp(lines[i], "push")) {
            push(csystems);
        }

        else if (!strcmp(lines[i], "pop")) {
            pop(csystems);
        }

        // Transformations
        else if (!strcmp(lines[i], "scale")) {
            char * args = lines[++i];
            double x, y, z;

            sscanf(args, "%le %le %le",
                   &x, &y, &z);

            struct matrix * scale = make_scale(x, y, z);

            struct matrix * matrix = peek(csystems);
            matrix_mult(matrix, scale);
            copy_matrix(scale, matrix);
        }

        else if (!strcmp(lines[i], "move")) {
            char * args = lines[++i];
            double x, y, z;

            sscanf(args, "%le %le %le",
                   &x, &y, &z);

            struct matrix * translate = make_translate(x, y, z);
            struct matrix * matrix = peek(csystems);
            matrix_mult(matrix, translate);
            copy_matrix(translate, matrix);
        }

        else if (!strcmp(lines[i], "rotate")) {
            char * args = lines[++i];

            double angle;
            char axis;
            sscanf(args, "%c %lf", &axis, &angle);
            double rad = angle * M_PI / 180;

            struct matrix * rotate;
            if ('x' == axis) rotate = make_rotX(rad);
            else if ('y' == axis) rotate = make_rotY(rad);
            else if ('z' == axis) rotate = make_rotZ(rad);

            struct matrix * matrix = peek(csystems);
            matrix_mult(matrix, rotate);
            copy_matrix(rotate, matrix);
        }

        // Lines
        else if (!strcmp(lines[i], "line")) {
            char * args = lines[++i];
            double x0, y0, z0, x1, y1, z1;

            sscanf(args, "%le %le %le %le %le %le",
                   &x0, &y0, &z0, &x1, &y1, &z1);

            add_edge(edges, x0, y0, z0, x1, y1, z1);

            struct matrix * matrix = peek(csystems);
            matrix_mult(matrix, edges);

            draw_lines(edges, s, zb, c);
            edges -> lastcol = 0;
        }

        else if (!strcmp(lines[i], "circle")) {
            char * args = lines[++i];
            double cx, cy, cz, r;

            sscanf(args, "%le %le %le %le", &cx, &cy, &cz, &r);

            add_circle(edges, cx, cy, cz, r, linestep);

            struct matrix * matrix = peek(csystems);
            matrix_mult(matrix, edges);

            draw_lines(edges, s, zb, c);
            edges -> lastcol = 0;
        }

        else if (!strcmp(lines[i], "hermite") || !strcmp(lines[i], "bezier")) {
            char * args = lines[++i];
            double x0, y0, x1, y1, x2, y2, x3, y3;
            int type;

            if (!strcmp(lines[i - 1], "hermite")) type = 0;
            else type = 1;

            sscanf(args, "%le %le %le %le %le %le %le %le",
                          &x0, &y0, &x1, &y1, &x2, &y2, &x3, &y3);

            add_curve(edges, x0, y0, x1, y1, x2, y2, x3, y3, linestep, type);

            struct matrix * matrix = peek(csystems);
            matrix_mult(matrix, edges);

            draw_lines(edges, s, zb, c);
            edges -> lastcol = 0;
        }

        // Polygons
        else if (!strcmp(lines[i], "box")) {
            char * args = lines[++i];
            double x, y, z, width, height, depth;

            sscanf(args, "%le %le %le %le %le %le",
                          &x, &y, &z, &width, &height, &depth);

            add_box(polygons, x, y, z, width, height, depth);

            struct matrix * matrix = peek(csystems);
            matrix_mult(matrix, polygons);

            draw_polygons(polygons, s, zb, c,
                          view, ambient, point, light, areflect, dreflect, sreflect);
            polygons -> lastcol = 0;
        }

        else if (!strcmp(lines[i], "sphere")) {
            char * args = lines[++i];
            double cx, cy, cz, r;

            sscanf(args, "%le %le %le %le", &cx, &cy, &cz, &r);

            add_sphere(polygons, cx, cy, cz, r, polystep);

            struct matrix * matrix = peek(csystems);
            matrix_mult(matrix, polygons);

            draw_polygons(polygons, s, zb, c,
                          view, ambient, point, light, areflect, dreflect, sreflect);
            polygons -> lastcol = 0;
        }

        else if (!strcmp(lines[i], "torus")) {
            char * args = lines[++i];
            double cx, cy, cz, r1, r2;

            sscanf(args, "%le %le %le %le %le", &cx, &cy, &cz, &r1, &r2);

            add_torus(polygons, cx, cy, cz, r1, r2, polystep);

            struct matrix * matrix = peek(csystems);
            matrix_mult(matrix, polygons);

            draw_polygons(polygons, s, zb, c,
                          view, ambient, point, light, areflect, dreflect, sreflect);
            polygons -> lastcol = 0;
        }

        //  Misc
        else if (!strcmp(lines[i], "clear")) {
            clear_screen(s);
            clear_zbuffer(zb);
        }

        else if (!strcmp(lines[i], "display")) {
            // int elastcol = edges -> lastcol;
            // int plastcol = polygons -> lastcol;

            // clear_screen(s);
            // if (elastcol > 0) draw_lines(edges, s, c);
            // if (plastcol > 0) draw_polygons(polygons, s, c);
            display(s);
        }

        else if (!strcmp(lines[i], "save")) {
            // clear_screen(s);
            // draw_lines(edges, s, c);
            // draw_lines(polygons, s, c);

            char * arg = lines[++i];
            save_extension(s, arg);
            printf("Saved as %s\n", arg);
        }

        // My Functions
        else if (!strcmp(lines[i], "color")) {
            char * args = lines[++i];
            int r, g, b;

            sscanf(args, "%d %d %d",
                   &r, &g, &b);

            change_color(&c, r, g, b);
        }

        // else if (!strcmp(lines[i], "ident")) ident(transform);

        // else if (!strcmp(lines[i], "apply")) {
        //     matrix_mult(transform, edges);
        //     matrix_mult(transform, polygons);
        // }
    }
}