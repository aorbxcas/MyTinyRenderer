#include <iostream>
#include "shape.h"

const int width = 800;
const int height = 800;
Model *model = NULL;
float *shadowbuffer = NULL;
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const char* filename2 = "C:\\Users\\Aorb\\MyTinyRenderer\\MyTinyRenderer\\MyTinyRenderer\\obj\\african_head\\african_head.obj";

const char* filename = "C:\\Users\\Aorb\\MyTinyRenderer\\MyTinyRenderer\\MyTinyRenderer\\obj\\diablo3_pose\\diablo3_pose.obj";

Vec3f light_dir(1,1,1);
Vec3f eye(0,0,3);
Vec3f center(0,0,0);
Vec3f up(0,1,0);


struct GouraudShader : public IShader {
    Vec3f varying_intensity; // 存储顶点的光照强度，在顶点着色器中写入，片段着色器中读取
    mat<4,4,float> uniform_M; // 存储投影*模型视图变换矩阵(Projection*ModelView)
    mat<4,4,float> uniform_MIT; // 存储(Projection*ModelView)的逆转置矩阵，用于法线变换
    mat<2,3,float> varying_uv; // 存储纹理坐标，在顶点着色器中写入，片段着色器中读取

    mat<4,3,float> varying_tri; // 存储点的裁剪坐标，在顶点着色器中写入，片段着色器中读取
    mat<3,3,float> varying_nrm; // 存储每个顶点的法线，在顶点着色器中写入，片段着色器中插值使用
    mat<3,3,float> ndc_tri;    // 存储标准化设备坐标(Normalized Device Coordinates)下的三角形

    mat<4,4,float> uniform_Mshadow; // 统一变量矩阵，用于将帧缓冲区屏幕坐标转换为阴影缓冲区屏幕坐标
    
    GouraudShader(Matrix M, Matrix MIT, Matrix MS) : uniform_M(M), uniform_MIT(MIT), uniform_Mshadow(MS), varying_uv(), varying_tri() {}
    
    virtual Vec4f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        // 设置第nthvert个顶点的法线，经过法线矩阵变换
        varying_nrm.set_col(nthvert, proj<3>((Projection*ModelView).invert_transpose()*embed<4>(model->normal(iface, nthvert), 0.f)));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));// 顶点坐标
        // 顶点坐标变换到屏幕坐标系
        gl_Vertex = Viewport*Projection*ModelView*gl_Vertex; 
        varying_tri.set_col(nthvert, gl_Vertex);
        // 计算第nthvert个顶点的漫反射光照强度
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); 
        ndc_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));
        // 此时返回的顶点是投影到1*1*1立方体后的点的坐标位置，并且存储了一些如法向量、uv等额外信息
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        
        Vec4f sb_p = uniform_Mshadow*embed<4>(varying_tri*bar);
        sb_p = sb_p/sb_p[3];
        int idx = int(sb_p[0]) + int(sb_p[1])*width; 
        float shadow = .3+.7*(shadowbuffer[idx]<sb_p[2]+43.34); 
        
        Vec3f bn = (varying_nrm*bar).normalize();
        Vec2f uv = varying_uv*bar;     

        // 切线空间法线
        mat<3,3,float> A;
        A[0] = ndc_tri.col(1) - ndc_tri.col(0);
        A[1] = ndc_tri.col(2) - ndc_tri.col(0);
        A[2] = bn;
        mat<3,3,float> AI = A.invert();
        Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
        Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);
        mat<3,3,float> B;
        B.set_col(0, i.normalize());
        B.set_col(1, j.normalize());
        B.set_col(2, bn);
        Vec3f n = (B*model->normal(uv)).normalize();
        Vec3f l = proj<3>(uniform_M  *embed<4>(light_dir        )).normalize();
        Vec3f r = (n*(n*l*2.f) - l).normalize();   // reflected light
        float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
        float diff = std::max(0.f, n*light_dir);
        TGAColor c = model->diffuse(uv);
        color = c;
        for (int i=0; i<3; i++) color[i] = std::min<float>(20 + c[i]*shadow*(1.2f*diff + .6*spec), 255);
        return false;          
    }
};

struct DepthShader : public IShader {
    mat<3,3,float> varying_tri;

    DepthShader() : varying_tri() {}

    virtual Vec4f vertex(int iface, int nthvert) {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;          // transform it to screen coordinates
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec3f p = varying_tri*bar;
        color = TGAColor(255, 255, 255)*(p.z/depth);
        return false;
    }
};

int main() {
    model = new Model(filename);
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
    TGAImage shadowbuffer_image(width, height, TGAImage::RGB);

    shadowbuffer   = new float[width*height];
    for (int i=width*height; --i; ) {
        shadowbuffer[i] = -std::numeric_limits<float>::max();
    }
    // 阴影渲染
    {
        lookat(light_dir, center, up);
        viewport(width/8, height/8, width*3/4, height*3/4);
        projection(0);
        light_dir.normalize();
        
        DepthShader shader;
        shadowbuffer = Tringle(model, shader, shadowbuffer_image,light_dir);
    }
    // 管线几何阶段--MVP坐标变换
    Matrix M = Viewport*Projection*ModelView;
    {
        lookat(eye, center, up);
        viewport(width/8, height/8, width*3/4, height*3/4);
        projection(-1.f/(eye-center).norm());
        // 获得变换矩阵
        light_dir.normalize();
        // 初始化着色器
        GouraudShader shader(ModelView, (Projection*ModelView).invert_transpose(), M*(Viewport*Projection*ModelView).invert());
        Tringle(model, shader, image,light_dir);
        image.flip_vertically(); 
        image.write_tga_file("output.tga");
    }
    delete model;
    return 0;

}