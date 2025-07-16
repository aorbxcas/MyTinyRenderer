#include <iostream>
#include "shape.h"

const int width = 800;
const int height = 800;
Model *model = NULL;
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const char* filename = "D:\\DevProject\\ForkPro\\tinyrenderer\\obj\\african_head\\african_head.obj";

Vec3f light_dir(1,1,1);
Vec3f eye(1,1,3);
Vec3f center(0,0,0);
Vec3f up(0,1,0);


struct GouraudShader : public IShader {
    Vec3f varying_intensity; // written by vertex shader, read by fragment shader
    virtual Vec4f vertex(int iface, int nthvert) {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;     // transform it to screen coordinates
        Vec3f s = model->normal(iface, nthvert);
        //varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
        Vec3f normalFilter = Vec3f(std::max(0.f,model->normal(iface, nthvert).x), std::max(0.f,model->normal(iface, nthvert).y), std::max(0.f,model->normal(iface, nthvert).z));
        varying_intensity[nthvert] = normalFilter*light_dir; // get diffuse lighting intensity
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
        color = TGAColor(255, 255, 255)*intensity; // well duh
        return false;                              // no, we do not discard this pixel
    }
};

int main() {
    model = new Model(filename);
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

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

    lookat(eye, center, up);
    viewport(width/8, height/8, width*3/4, height*3/4);
    projection(-1.f/(eye-center).norm());
    light_dir.normalize();
    
    GouraudShader shader;

    Tringle(model, shader, image,light_dir);
    image.flip_vertically(); // to place the origin in the bottom left corner of the image
    image.write_tga_file("output.tga");

    delete model;
    return 0;

}