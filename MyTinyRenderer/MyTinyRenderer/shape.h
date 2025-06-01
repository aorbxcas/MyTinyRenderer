#pragma once
#include "tgaimage.h"
enum LineType
{
    DDA,
    Breshman,
};

void Line(int x1,int y1,int x2,int y2,TGAImage image,TGAColor color,LineType type) {
    switch (type) {
    case DDA:
        for (float t = 0.; t < 1.; t += .01) {
            int x = x1 + (x2 - x1) * t;
            int y = y1 + (y2 - y1) * t;
            image.set(x, y, color);
        }
        break;
    case Breshman:
        break;
    }

}