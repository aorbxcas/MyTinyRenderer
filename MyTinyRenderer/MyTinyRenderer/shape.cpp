
#pragma once
#include "shape.h"
#include <iostream>

IShader::~IShader() {}


void Line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color, LineType type) {
    switch (type) {
    case DDA:
        for (float t = 0.; t < 1.; t += .01) {
            int x = x0 + (x1 - x0) * t;
            int y = y0 + (y1 - y0) * t;
            image.set(x, y, color);
        }
        break;
    case Breshman:
        bool steep = false;
        if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
            std::swap(x0, y0);
            std::swap(x1, y1);
            steep = true;
        }
        if (x0 > x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        for (int x = x0; x <= x1; x++) {
            float t = (x - x0) / (float)(x1 - x0);
            int y = y0 * (1. - t) + y1 * t;
            if (steep) {
                image.set(y, x, color);
            }
            else {
                image.set(x, y, color);
            }
        }
        break;
    }
}

float CaluateCrossValue(Vec2i a,Vec2i b) {
    return (a.x * b.y) - (a.y * b.x);
}
Vec3f CaluateCross(Vec3f a,Vec3f b) {
    return Vec3f(
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x);
}

float CaluateDot(Vec3f a,Vec3f b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float CheckPointInLine(Vec2i a, Vec2i b, Vec2i p) {
    return CaluateCrossValue(Vec2i(p.x-a.x,p.y-a.y),Vec2i(p.x-b.x,p.y-b.y));
}
float CheckPointInLine(Vec3i a, Vec3i b, Vec3i p) {
    return CaluateCrossValue(Vec2i(p.x-a.x,p.y-a.y),Vec2i(p.x-b.x,p.y-b.y));
}

bool CheckPointInTrigle(Vec2i a, Vec2i b, Vec2i c, Vec2i p) {
    float line_1 = CheckPointInLine(a, b, p);
    float line_2 = CheckPointInLine(b, c, p);
    float line_3 = CheckPointInLine(c, a, p);
    return (line_1 >= 0&&line_2 >= 0&&line_3 >= 0) || (line_1 <= 0&&line_2 <= 0&&line_3 <= 0);
}

bool CheckPointInTrigle(Vec3i a, Vec3i b, Vec3i c, Vec3i p) {
    float line_1 = CheckPointInLine(a, b, p);
    float line_2 = CheckPointInLine(b, c, p);
    float line_3 = CheckPointInLine(c, a, p);
    return (line_1 >= 0&&line_2 >= 0&&line_3 >= 0) || (line_1 <= 0&&line_2 <= 0&&line_3 <= 0);
}

void TringleSet(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color,int width,int height)
{
    int minx = std::min(t0.x, t1.x);
    minx = std::min(minx, t2.x);
    minx = std::max(minx, 0);
    int maxx = std::max(t0.x, t1.x);
    maxx = std::max(maxx, t2.x);
    maxx = std::min(width - 1, maxx);
    int miny = std::min(t0.y, t1.y);
    miny = std::min(miny, t2.y);
    miny = std::max(miny, 0);
    int maxy = std::max(t0.y, t1.y);
    maxy = std::max(maxy, t2.y);
    maxy = std::min(height - 1, maxy);

    for (int x = minx; x <= maxx; x++)
    {
        for (int y = miny; y <= maxy; y++)
        {
            Vec2i p = Vec2i(x, y);
            if (CheckPointInTrigle(t0, t1, t2, p))
            {
                image.set(x,y,color);
            }
        }
    }
}
// 判断点是否在三角形内
bool Barycentric(Vec3i a, Vec3i b, Vec3i c, Vec3i p, float &u, float &v, float &w)
{
    // 向量差
    Vec3i v0 = b - a;
    Vec3i v1 = c - a;
    Vec3i v2 = Vec3i(p.x, p.y, 0) - a;

    float d00 = CaluateDot(Vec3f(v0.x, v0.y, 0), Vec3f(v0.x, v0.y, 0));
    float d01 = CaluateDot(Vec3f(v0.x, v0.y, 0), Vec3f(v1.x, v1.y, 0));
    float d11 = CaluateDot(Vec3f(v1.x, v1.y, 0), Vec3f(v1.x, v1.y, 0));
    float d20 = CaluateDot(Vec3f(v2.x, v2.y, 0), Vec3f(v0.x, v0.y, 0));
    float d21 = CaluateDot(Vec3f(v2.x, v2.y, 0), Vec3f(v1.x, v1.y, 0));

    float denom = d00 * d11 - d01 * d01;

    if (denom == 0.f) return false; // 防止除以零

    float invDenom = 1.0f / denom;

    v = (d00 * d21 - d01 * d20) * invDenom;
    w = (d11 * d20 - d01 * d21) * invDenom;
    u = 1.0f - v - w;

    return (u >= 0) && (v >= 0) && (w >= 0); // 判断是否在三角形内
}

// 3维
void TringleSet(Vec3i t0, Vec3i t1, Vec3i t2, TGAImage &image, TGAColor color,float *zbuffer)
{
    int minx = std::min(t0.x, t1.x);
    minx = std::min(minx, t2.x);
    minx = std::max(minx, 0);
    int maxx = std::max(t0.x, t1.x);
    maxx = std::max(maxx, t2.x);
    maxx = std::min(image.get_width() - 1, maxx);
    int miny = std::min(t0.y, t1.y);
    miny = std::min(miny, t2.y);
    miny = std::max(miny, 0);
    int maxy = std::max(t0.y, t1.y);
    maxy = std::max(maxy, t2.y);
    maxy = std::min(image.get_height() - 1, maxy);

    for (int x = minx; x <= maxx; x++)
    {
        for (int y = miny; y <= maxy; y++)
        {
            Vec3i p = Vec3i(x, y,0);

            if (CheckPointInTrigle(t0, t1, t2, p))
            {
                // 计算重心坐标
                float alpha,beta,gamma;
                Barycentric(t0, t1, t2, p, alpha, beta, gamma);
                // 计算深度值
                float z = alpha * t0.z + beta * t1.z + gamma * t2.z;
                if (zbuffer[int(p.x+p.y*image.get_width())]<p.z) {
                    zbuffer[int(p.x+p.y*image.get_width())] = p.z;
                    image.set(p.x, p.y, color);
                }
            }
        }
    }
}

// 带着色
void TriangleSet(Vec3f *pts, IShader &shader, TGAImage &image, float *zbuffer) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j]);
        }
    }
    //std::cout<<"Tringle:"<<std::endl;
    // for (int i=0; i<3; i++)
    // {
    //     std::cout<<pts[i].x<<"--"<<pts[i].y<<"--"<<pts[i].z<<std::endl;
    // }
    Vec3i P;
    TGAColor color;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            if (CheckPointInTrigle(pts[0],pts[1],pts[2],P))
            {
                // 计算重心坐标
                float alpha,beta,gamma;
                Barycentric(pts[0], pts[1], pts[2], P, alpha, beta, gamma);
                // 计算深度值
                if (alpha<0||beta<0||gamma<0)continue;
                float z = alpha * pts[0].z + beta * pts[1].z + gamma * pts[2].z;
                P.z = z;
                bool discard = shader.fragment(Vec3f(alpha,beta,gamma), color);
                if (zbuffer[int(P.x+P.y*image.get_width())]<P.z&&!discard) {
                    // if (zbuffer[int(P.x+P.y*image.get_width())]!=0)
                    // {
                    //     std::cout<<"Position:x:"<<P.x<<";y:"<<P.y<<"\nzBuffer:"<<zbuffer[int(P.x+P.y*image.get_width())]<<"--->"<<P.z<<std::endl;
                    //     image.set(P.x,P.y,TGAColor(255,0,0,255));
                    //     continue;
                    // }
                    zbuffer[int(P.x+P.y*image.get_width())] = P.z;
                    image.set(P.x, P.y, color);
                }

            }
            // Vec3f c = barycentric(proj<2>(pts[0]/pts[0][3]), proj<2>(pts[1]/pts[1][3]), proj<2>(pts[2]/pts[2][3]), proj<2>(P));
            // float z = pts[0][2]*c.x + pts[1][2]*c.y + pts[2][2]*c.z;
            // float w = pts[0][3]*c.x + pts[1][3]*c.y + pts[2][3]*c.z;
            // int frag_depth = std::max(0, std::min(255, int(z/w+.5)));
            // if (c.x<0 || c.y<0 || c.z<0 || zbuffer.get(P.x, P.y)[0]>frag_depth) continue;
            // bool discard = shader.fragment(c, color);
            // if (!discard) {
            //     zbuffer.set(P.x, P.y, TGAColor(frag_depth));
            //     image.set(P.x, P.y, color);
            // }
        }
    }
}

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

void viewport(int x, int y, int w, int h) {
    Viewport = Matrix::identity();
    Viewport[0][3] = x+w/2.f;
    Viewport[1][3] = y+h/2.f;
    Viewport[2][3] = 255.f/2.f;
    Viewport[0][0] = w/2.f;
    Viewport[1][1] = h/2.f;
    Viewport[2][2] = 255.f/2.f;
}

void projection(float coeff) {
    Projection = Matrix::identity();
    Projection[3][2] = coeff;
}

void lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye-center).normalize();
    Vec3f x = cross(up,z).normalize();
    Vec3f y = cross(z,x).normalize();
    ModelView = Matrix::identity();
    for (int i=0; i<3; i++) {
        ModelView[0][i] = x[i];
        ModelView[1][i] = y[i];
        ModelView[2][i] = z[i];
        ModelView[i][3] = -center[i];
    }
}

// 无材质
void Tringle(Model* model,TGAImage &image)
{
    int width = image.get_width();
    int height = image.get_height();
    Vec3f light_dir(0,0,-1);
    float *zBuffer = new float[width*height];
    
    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec3i screen_coords[3];
        Vec3f world_coords[3]; 
        for (int j = 0; j < 3; j++) {
            Vec3f world_coord = model->vert(face[j]); 
            screen_coords[j] = Vec3i((world_coord.x+1.)*width/2., (world_coord.y+1.)*height/2.,0);
            world_coords[j]  = world_coord; 
        }
        Vec3f n = CaluateCross(world_coords[2]-world_coords[0],world_coords[1]-world_coords[0]);
        n.normalize();
        float intensity = CaluateDot(n,light_dir);
        if (intensity > 0) {
            TringleSet(screen_coords[0],screen_coords[1],screen_coords[2],image,TGAColor(intensity * 255, intensity * 255, intensity * 255, 255),zBuffer);
        }
    }
}
// TODO 有材质
void Tringle(Model* model,IShader &shader,TGAImage &image,Vec3f light_dir)
{
    int width = image.get_width();
    int height = image.get_height();
    float *zBuffer = new float[width*height]();

    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        //Vec3f screen_coords[3];
        Vec3f world_coords[3];
        Vec4f pts_proj_4[3];
        Vec3f pts_proj_3[3];
        for (int j = 0; j < 3; j++) {
            pts_proj_4[j] = shader.vertex(i, j);
            pts_proj_3[j] = proj<3>(pts_proj_4[j]);
            Vec3f world_coord = pts_proj_3[j]; 
            //screen_coords[j] = Vec3i((world_coord.x+1.)*width/2., (world_coord.y+1.)*height/2.,0);
            world_coords[j]  = world_coord;
        }
        TriangleSet(pts_proj_3,shader,image,zBuffer);
    }
}
// void Tringle(Vec4f* pts, TGAImage &image, TGAImage &zbuffer)
// {
//     Vec3f light_dir(0,0,-1);
//     
//     for (int i = 0; i < model->nfaces(); i++) {
//         
//         std::vector<int> face = model->face(i);
//         Vec2i screen_coords[3];
//         Vec3f world_coords[3]; 
//         for (int j = 0; j < 3; j++) {
//             Vec3f world_coord = model->vert(face[j]); 
//             screen_coords[j] = Vec2i((world_coord.x+1.)*image.get_width()/2., (world_coord.y+1.)*image.get_height()/2.);
//             world_coords[j]  = world_coord; 
//         }
//         Vec3f n = CaluateCross(world_coords[2]-world_coords[0],world_coords[1]-world_coords[0]);
//         n.normalize();
//         float intensity = CaluateDot(n,light_dir);
//         if (intensity > 0) {
//             TringleSet(screen_coords[0],screen_coords[1],screen_coords[2],image,TGAColor(intensity * 255, intensity * 255, intensity * 255, 255),width,height);
//         }
//     }
// }


