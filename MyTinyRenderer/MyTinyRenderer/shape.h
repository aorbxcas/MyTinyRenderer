#pragma once
#include "model.h"
struct IShader {
    virtual ~IShader();
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

enum LineType
{
    DDA,
    Breshman,
};

float CaluateCrossValue(Vec2i a, Vec2i b);

float CheckPointInLine(Vec2i a, Vec2i b, Vec2i p);

bool CheckPointInTrigle(Vec2i a, Vec2i b, Vec2i c, Vec2i p);

void TriangleSet(Vec3f *pts, IShader &shader, TGAImage &image, float *zbuffer);


// 引申

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;
const float depth = 2000.f;


void viewport(int x, int y, int w, int h);
void projection(float coeff=0.f); // coeff = -1/c
void lookat(Vec3f eye, Vec3f center, Vec3f up);
void translate(float tx = 0, float ty = 0, float tz = 0);
void scale(float sx = 1, float sy = 1, float sz = 1);
void rotates(float angle_x = 0, float angle_y = 0, float angle_z = 0);

float* Tringle(Model* model,IShader &shader, TGAImage &image,Vec3f light_dir);


