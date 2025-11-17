#pragma once
#include "shape.h"
#include <iostream>

IShader::~IShader() {}

float CaluateCrossValue(Vec2i a,Vec2i b) {
    return (a.x * b.y) - (a.y * b.x);
}

float CheckPointInLine(Vec3i a, Vec3i b, Vec3i p) {
    return CaluateCrossValue(Vec2i(p.x-a.x,p.y-a.y),Vec2i(p.x-b.x,p.y-b.y));
}

bool CheckPointInTrigle(Vec3i a, Vec3i b, Vec3i c, Vec3i p) {
    float line_1 = CheckPointInLine(a, b, p);
    float line_2 = CheckPointInLine(b, c, p);
    float line_3 = CheckPointInLine(c, a, p);
    return (line_1 >= 0&&line_2 >= 0&&line_3 >= 0) || (line_1 <= 0&&line_2 <= 0&&line_3 <= 0);
}
// 判断点是否在三角形内，返回三角形重心坐标
bool Barycentric(Vec3i a, Vec3i b, Vec3i c, Vec3i p, float &u, float &v, float &w)
{
    Vec2f A = Vec2f(a.x, a.y);
    Vec2f B = Vec2f(b.x, b.y);
    Vec2f C = Vec2f(c.x, c.y);
    Vec2f P = Vec2f(p.x, p.y);
    Vec3f s[2];
    for (int i=2; i--; ) {
        s[i][0] = C[i]-A[i];
        s[i][1] = B[i]-A[i];
        s[i][2] = A[i]-P[i];
    }
    Vec3f o = cross(s[0], s[1]);
    if (std::abs(o[2])>1e-2)
    {
        u=1.f-(o.x+o.y)/o.z;
        v = o.y/o.z;
        w = o.x/o.z;
        return true;
    }else
    {
        return false;
    }
}

// 光栅化阶段
void TriangleSet(Vec3f *pts, IShader &shader, TGAImage &image, float *zbuffer) {
    // 计算三个点的包围盒
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j]);
        }
    }
    Vec3i P;
    TGAColor color;
    // 扫描包围盒进行点处理
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            // 首先检查点是否在三角形中
            if (CheckPointInTrigle(pts[0],pts[1],pts[2],P))
            {
                // 计算重心坐标
                float alpha,beta,gamma;
                Barycentric(pts[0], pts[1], pts[2], P, alpha, beta, gamma);
                // 计算深度值
                if (alpha<0||beta<0||gamma<0)continue;
                float z = alpha * pts[0].z + beta * pts[1].z + gamma * pts[2].z;// 利用重心坐标对三角形顶点插值计算当前点深度值
                P.z = z;
                // 光栅化阶段--像素（片段）着色器
                bool discard = shader.fragment(Vec3f(alpha,beta,gamma), color);
                // 进行深度测试，确定是否丢弃该像素点
                if (zbuffer[int(P.x+P.y*image.get_width())]<P.z&&!discard) {
                    zbuffer[int(P.x+P.y*image.get_width())] = P.z;
                    image.set(P.x, P.y, color);
                }

            }
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
    Viewport[2][3] = depth/2.f;
    Viewport[0][0] = w/2.f;
    Viewport[1][1] = h/2.f;
    Viewport[2][2] = depth/2.f;
}

void projection(float coeff) {
    Projection = Matrix::identity();
    Projection[3][2] = coeff;
}
// 模型变换，默认1,1,1
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
// 矩阵平移，m为平移矩阵
void translate(float tx, float ty, float tz) {
    Matrix m = Matrix::identity();
    m[0][3] = tx;
    m[1][3] = ty;
    m[2][3] = tz;
    ModelView = m * ModelView;
}
// 矩阵缩放，m为缩放矩阵
void scale(float sx, float sy, float sz) {
    Matrix m = Matrix::identity();
    m[0][0] = sx;
    m[1][1] = sy;
    m[2][2] = sz;
    ModelView = m * ModelView;
}

// 旋转矩阵
void rotates(float angle_x, float angle_y, float angle_z) {
    // Create rotation matrices for each axis
    Matrix rx = Matrix::identity();
    Matrix ry = Matrix::identity();
    Matrix rz = Matrix::identity();
    
    // Rotation around X-axis
    float cx = cos(angle_x);
    float sx = sin(angle_x);
    rx[1][1] = cx;
    rx[1][2] = -sx;
    rx[2][1] = sx;
    rx[2][2] = cx;
    
    // Rotation around Y-axis
    float cy = cos(angle_y);
    float sy = sin(angle_y);
    ry[0][0] = cy;
    ry[0][2] = sy;
    ry[2][0] = -sy;
    ry[2][2] = cy;
    
    // Rotation around Z-axis
    float cz = cos(angle_z);
    float sz = sin(angle_z);
    rz[0][0] = cz;
    rz[0][1] = -sz;
    rz[1][0] = sz;
    rz[1][1] = cz;
    
    // Combine rotations (order: Z * Y * X)
    Matrix rotation_matrix = rz * ry * rx;
    
    // Apply to ModelView matrix
    ModelView = rotation_matrix * ModelView;
}
// TODO 有材质
float* Tringle(Model* model,IShader &shader,TGAImage &image,Vec3f light_dir)
{
    int width = image.get_width();
    int height = image.get_height();
    float *zBuffer = new float[width*height]();

    for (int i = 0; i < model->nfaces(); i++) {
        // 获得模型所有坐标点
        std::vector<int> face = model->face(i);
        Vec4f pts_proj_4[3];
        Vec3f pts_proj_3[3];
        for (int j = 0; j < 3; j++) {
            // 几何阶段--顶点着色shader
            pts_proj_4[j] = shader.vertex(i, j);
            // 拿到的点是mvp变换后的坐标，1*1*1立方体
            pts_proj_3[j] = proj<3>(pts_proj_4[j]);
        }
        // 三个点一组送入光栅化阶段、光栅化
        TriangleSet(pts_proj_3,shader,image,zBuffer);
    }
    // 返回zBuffer供后续使用
    return zBuffer;
}
