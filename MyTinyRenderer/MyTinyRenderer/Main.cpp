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
const char* filename2 = "D:\\DevProject\\ForkPro\\tinyrenderer\\obj\\african_head\\african_head.obj";

const char* filename = "D:\\obj\\diablo3_pose\\diablo3_pose.obj";

Vec3f light_dir(1,1,1);
Vec3f eye(-10,0,3);
Vec3f center(0,0,0);
Vec3f up(0,1,0);


struct GouraudShader : public IShader {
    Vec3f varying_intensity; // written by vertex shader, read by fragment shader
    mat<4,4,float> uniform_M;   //  Projection*ModelView
    mat<4,4,float> uniform_MIT; // (Projection*ModelView).invert_transpose()
    mat<2,3,float> varying_uv;        // same as above

    mat<4,3,float> varying_tri; // triangle coordinates (clip coordinates), written by VS, read by FS
    mat<3,3,float> varying_nrm; // normal per vertex to be interpolated by FS
    mat<3,3,float> ndc_tri;     // triangle in normalized device coordinates

    mat<4,4,float> uniform_Mshadow; // transform framebuffer screen coordinates to shadowbuffer screen coordinates
    
    GouraudShader(Matrix M, Matrix MIT, Matrix MS) : uniform_M(M), uniform_MIT(MIT), uniform_Mshadow(MS), varying_uv(), varying_tri() {}
    
    virtual Vec4f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        varying_nrm.set_col(nthvert, proj<3>((Projection*ModelView).invert_transpose()*embed<4>(model->normal(iface, nthvert), 0.f)));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;     // transform it to screen coordinates
        varying_tri.set_col(nthvert, gl_Vertex);

        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
        //Vec3f normalFilter = Vec3f(std::max(0.f,model->normal(iface, nthvert).x), std::max(0.f,model->normal(iface, nthvert).y), std::max(0.f,model->normal(iface, nthvert).z));
        //varying_intensity[nthvert] = normalFilter*light_dir; // get diffuse lighting intensity
        ndc_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        
        Vec4f sb_p = uniform_Mshadow*embed<4>(varying_tri*bar); // corresponding point in the shadow buffer
        sb_p = sb_p/sb_p[3];
        int idx = int(sb_p[0]) + int(sb_p[1])*width; // index in the shadowbuffer array
        float shadow = .3+.7*(shadowbuffer[idx]<sb_p[2]+43.34); // magic coeff to avoid z-fighting
        
        Vec3f bn = (varying_nrm*bar).normalize();
        //float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
        Vec2f uv = varying_uv*bar;                 // interpolate uv for the current pixel

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

        // 世界空间法线
        // Vec3f n = proj<3>(uniform_MIT*embed<4>(model->normal(uv))).normalize();
        // Vec3f l = proj<3>(uniform_M  *embed<4>(light_dir        )).normalize();
        // Vec3f r = (n*(n*l*2.f) - l).normalize();   // reflected light
        // float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
        // float diff = std::max(0.f, n*l);
        // TGAColor c = model->diffuse(uv);
        // color = c;
        // for (int i=0; i<3; i++) color[i] = std::min<float>(5 + c[i]*(diff + .6*spec), 255);

        
        //float intensity = std::max(0.f, n*l);
        //color = model->diffuse(uv)*intensity;      // well duh
        return false;                              // no, we do not discard this pixel
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
    // for (int i = 0; i < model->nfaces(); i++) {
    //     std::vector<int> face = model->face(i);
    //     for (int j = 0; j < 3; j++) {
    //         Vec3f v0 = model->vert(face[j]);
    //         Vec3f v1 = model->vert(face[(j + 1) % 3]);
    //         int x0 = (v0.x + 1.) * width / 2.;
    //         int y0 = (v0.y + 1.) * height / 2.;
    //         int x1 = (v1.x + 1.) * width / 2.;
    //         int y1 = (v1.y + 1.) * height / 2.;
    //         Line(x0, y0, x1, y1, image, white, LineType::Breshman);
    //     }
    // }
    // image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    // image.write_tga_file("output.tga");
    // delete model;
    // return 0;


    // Vec3f light_dir(0,0,-1);
    // for (int i = 0; i < model->nfaces(); i++) {
    //     
    //     std::vector<int> face = model->face(i);
    //     Vec2i screen_coords[3];
    //     Vec3f world_coords[3]; 
    //     for (int j = 0; j < 3; j++) {
    //         Vec3f world_coord = model->vert(face[j]); 
    //         screen_coords[j] = Vec2i((world_coord.x+1.)*width/2., (world_coord.y+1.)*height/2.);
    //         world_coords[j]  = world_coord; 
    //     }
    //     Vec3f n = CaluateCross(world_coords[2]-world_coords[0],world_coords[1]-world_coords[0]);
    //     n.normalize();
    //     float intensity = CaluateDot(n,light_dir);
    //     if (intensity > 0) {
    //         TringleSet(screen_coords[0],screen_coords[1],screen_coords[2],image,TGAColor(intensity * 255, intensity * 255, intensity * 255, 255),width,height);
    //     }
    // }
    //
    // image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    // image.write_tga_file("output.tga");
    // delete model;
    // return 0;



    
    // TGAImage image(width, height, TGAImage::RGB);
    //
    // Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)};
    // Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)};
    // Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
    //
    // TringleSet(t0[0], t0[1], t0[2], image, red,width,height);
    // TringleSet(t1[0], t1[1], t1[2], image, white,width,height);
    // TringleSet(t2[0], t2[1], t2[2], image, green,width,height);
    //
    // image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    // image.write_tga_file("output.tga");
    // return 0;

    // TGAImage scene(width, height, TGAImage::RGB);
    //
    // // scene "2d mesh"
    // Line(20, 34,   744, 400, scene, red,Breshman);
    // Line(120, 434 ,444, 400, scene, green,Breshman);
    // Line(330, 463, 594, 200, scene, blue,Breshman);
    // // screen line
    // Line(10, 10, 790, 10, scene, white,Breshman);
    // scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    // scene.write_tga_file("scene.tga");



    
    // Vec3f light_dir(0,0,-1);
    // float *zBuffer = new float[width*height];
    //
    // for (int i = 0; i < model->nfaces(); i++) {
    //     std::vector<int> face = model->face(i);
    //     Vec3i screen_coords[3];
    //     Vec3f world_coords[3]; 
    //     for (int j = 0; j < 3; j++) {
    //         Vec3f world_coord = model->vert(face[j]); 
    //         screen_coords[j] = Vec3i((world_coord.x+1.)*width/2., (world_coord.y+1.)*height/2.,0);
    //         world_coords[j]  = world_coord; 
    //     }
    //     Vec3f n = CaluateCross(world_coords[2]-world_coords[0],world_coords[1]-world_coords[0]);
    //     n.normalize();
    //     float intensity = CaluateDot(n,light_dir);
    //     if (intensity > 0) {
    //         TringleSet(screen_coords[0],screen_coords[1],screen_coords[2],image,TGAColor(intensity * 255, intensity * 255, intensity * 255, 255),zBuffer);
    //     }
    // }
    //
    // image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    // image.write_tga_file("output.tga");
    // delete model;
    // return 0;

    // lookat(eye, center, up);
    // viewport(width/8, height/8, width*3/4, height*3/4);
    // projection(-1.f/(eye-center).norm());
    // light_dir.normalize();
    //
    // GouraudShader shader;
    // shader.uniform_M   =  Projection*ModelView;
    // shader.uniform_MIT = (Projection*ModelView).invert_transpose();
    // Tringle(model, shader, image,light_dir);
    // image.flip_vertically(); // to place the origin in the bottom left corner of the image
    // image.write_tga_file("output.tga");


    {
        lookat(light_dir, center, up);
        viewport(width/8, height/8, width*3/4, height*3/4);
        projection(0);
        light_dir.normalize();
        
        DepthShader shader;
        shadowbuffer = Tringle(model, shader, shadowbuffer_image,light_dir);
        // image.flip_vertically(); // to place the origin in the bottom left corner of the image
        // image.write_tga_file("output.tga");
    }
    Matrix M = Viewport*Projection*ModelView;
    {
        lookat(eye, center, up);
        viewport(width/8, height/8, width*3/4, height*3/4);
        projection(-1.f/(eye-center).norm());
        light_dir.normalize();
        
        GouraudShader shader(ModelView, (Projection*ModelView).invert_transpose(), M*(Viewport*Projection*ModelView).invert());
        Tringle(model, shader, image,light_dir);
        image.flip_vertically(); // to place the origin in the bottom left corner of the image
        image.write_tga_file("output.tga");
    }
    delete model;
    return 0;

}