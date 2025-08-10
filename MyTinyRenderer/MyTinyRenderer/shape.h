#pragma once
#include "model.h"
struct IShader {
    virtual ~IShader();
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    //virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
    virtual bool fragment(Vec3f gl_FragCoord = Vec3f(0,0,0), Vec3f bar, TGAColor &color) = 0;
};

enum LineType
{
    DDA,
    Breshman,
};

float CaluateCrossValue(Vec2i a, Vec2i b);

float CaluateDot(Vec3f a, Vec3f b);

Vec3f CaluateCross(Vec3f a, Vec3f b);

bool CheckPointInTringle(Vec2i a, Vec2i b, Vec2i p);

void Line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color, LineType type);

float CheckPointInLine(Vec2i a, Vec2i b, Vec2i p);

bool CheckPointInTrigle(Vec2i a, Vec2i b, Vec2i c, Vec2i p);

void TringleSet(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color,int width,int height);

void TringleSet(Vec3i t0, Vec3i t1, Vec3i t2, TGAImage &image, TGAColor color,float *zbuffer);

void TriangleSet(Vec3f *pts, IShader &shader, TGAImage &image, float *zbuffer);


// 引申

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;
const float depth = 2000.f;


void viewport(int x, int y, int w, int h);
void projection(float coeff=0.f); // coeff = -1/c
void lookat(Vec3f eye, Vec3f center, Vec3f up);
float* Tringle(Model* model,IShader &shader, TGAImage &image,Vec3f light_dir);


