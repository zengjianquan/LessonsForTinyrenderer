// Lesson1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/*
* Lesson 1: Bresenham’s Line Drawing Algorithm 
* 布兰森汉姆绘制线算法
*/

#include <iostream>
#include "Lesson1.h"

//使用设定步长的方法进行画线
/*
* 缺点:
* 1.效率较低
* 2.依赖于选择的步长, 太小会增加计算负担, 太大会走样
*/
void DrawLineStep(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color) {
    //设定步长
    float step = 0.1f;
    for (float t = 0; t <= 1; t += step) {
        int tx = x0 + (x1 - x0) * t;
        int ty = y0 + (y1 - y0) * t;
        image.set(tx, ty, color);
    }
}

//直接找到像素进行绘制
/*
* 问题: 
* 1.当 dy > dx 时, 直线走样
* 2.当 dx < 0 时, 直线无法绘制
*/
void DrawLinePixelV1(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    for (int x = x0; x <= x1; x++) {
        //斜率公式 (y1 - y0) / (x1 - x0) == (yt - y0) / (xt - x0), 求 yt
        int yt = float(x - x0) * float(dy) / float(dx) + y0;
        image.set(x, yt, color);
    }
}

/*
* 问题(通过 3000000 次调用测试):
* 根据斜率进行计算, 效率太低
* 在这个函数中除了 image.set 部分, cpu占用率最高的便是计算 yt 的部分
*/
void DrawLinePixelV2(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color) {
    //当 dy > dx 时, 使用 y 进行采样计算
    //交换 x, y
    bool transpose = false;
    if (std::abs(x1 - x0) < std::abs(y1 - y0)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        transpose = true;
    }

    //当 x0 < x1 时, 交换两点, 保持从小到大计算
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;
    for (int x = x0; x <= x1; x++) {
        int yt = float(x - x0) * float(dy) / float(dx) + y0;

        if (transpose)  //判断是否交换, 若交换, 则逆回来
            image.set(yt, x, color);

        else
            image.set(x, yt, color);
    }
}

/*
* 还有浮点数计算的问题
*/
void DrawLinePixelV3(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color) {
    bool transpose = false;
    if (std::abs(x1 - x0) < std::abs(y1 - y0)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        transpose = true;
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    float dk = std::abs(dy / float(dx));
    float k = 0;

    int yt = y0;

    for (int x = x0; x <= x1; x++) {
        if (transpose)  //判断是否交换, 若交换, 则逆回来
            image.set(yt, x, color);

        else
            image.set(x, yt, color);

        k += dk;
        //斜截式 y = kx + b;
        //x 每增加一个单位, y 增加 dk
        //当 y 的增量大于 0.5 时, 即绘制下一个 y 坐标的像素
        //并且将 dk 的累计增量 -1
        if (k > .5f) {
            yt += (y1 > y0 ? 1 : -1);
            k -= 1;
        }
    }
}

void DrawLinePixelV4(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color) {
    bool transpose = false;
    if (std::abs(x1 - x0) < std::abs(y1 - y0)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        transpose = true;
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    int derror = std::abs(dy) << 1;
    int error = 0;

    int yt = y0;

    for (int x = x0; x <= x1; x++) {
        if (transpose)  //判断是否交换, 若交换, 则逆回来
            image.set(yt, x, color);

        else
            image.set(x, yt, color);

        error += derror;
        if (error > dx) {
            yt += (y1 > y0 ? 1 : -1);
            error -= dx * 2;
        }
    }
}

//Bresenham画线算法
void Bresenham(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color) {
    /*
    * 将直线转换到:
    * x0 < x1
    * 0 < slope < 1, -1 < slope < 0
    */
    bool bTranspose = false;
    if (std::abs(x1 - x0) < std::abs(y1 - y0)) {    //做 y = x ; y = -x 对称
        std::swap(x0, y0);
        std::swap(x1, y1);
        bTranspose = true;
    }

    if (x0 > x1) {  //对调两点, 保持 x0 < x1
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    
    /*
    * 将直线转换到: 
    * 0 < slope < 1
    * 对斜率为负的直线做 y轴对称
    */
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);

    int dEast = 2 * dy;
    int dEastNorth = 2 * dy - 2 * dx;
    int d = 2 * dy - dx;

    int y = y0;
    if (bTranspose) {   //对做 y = x ; y = -x 对称 进行 逆变换
        image.set(y, x0, color);
        for (int x = x0 + 1; x <= x1; x++) {
            if (d >= 0) {
                y += (y1 > y0 ? 1 : -1);    //判断直线是否经过 y = x 对称
                d += dEastNorth;
            }
            else {
                d += dEast;
            }
            image.set(y, x, color);
        }
    }

    else {
        image.set(x0, y, color);
        for (int x = x0 + 1; x <= x1; x++) {
            if (d >= 0) {
                y += (y1 > y0 ? 1 : -1);    //判断直线是否经过 y = x 对称
                d += dEastNorth;
            }
            else {
                d += dEast;
            }
            image.set(x, y, color);
        }
    }
}

/*
* 生成 lib 时请注释主函数
*/
int main()
{
    int width = 1000, height = 1000;
    TGAImage imageDrawLineStep(width, height, TGAImage::RGB);
    DrawLineStep(130, 200, 800, 400, imageDrawLineStep, { 0, 255, 255, 255 });
    imageDrawLineStep.write_tga_file("DrawLineStep.tga");

    TGAImage imageDrawLinePixelV1(width, height, TGAImage::RGB);
    DrawLinePixelV1(130, 200, 800, 400, imageDrawLinePixelV1, { 0, 0, 255, 255 });
    DrawLinePixelV1(200, 130, 400, 800, imageDrawLinePixelV1, { 255, 0, 0, 255 });
    DrawLinePixelV1(800, 500, 130, 300, imageDrawLinePixelV1, { 0, 255, 0, 255 });
    imageDrawLinePixelV1.write_tga_file("DrawLinePixelV1.tga");

    TGAImage imageDrawLinePixelV2(width, height, TGAImage::RGB);
    DrawLinePixelV2(130, 200, 800, 400, imageDrawLinePixelV2, { 0, 0, 255, 255 });
    DrawLinePixelV2(200, 130, 400, 800, imageDrawLinePixelV2, { 255, 0, 0, 255 });
    DrawLinePixelV2(800, 500, 130, 300, imageDrawLinePixelV2, { 0, 255, 0, 255 });
    imageDrawLinePixelV2.write_tga_file("DrawLinePixelV2.tga");

    TGAImage imageDrawLinePixelV3(width, height, TGAImage::RGB);
    DrawLinePixelV3(130, 200, 800, 400, imageDrawLinePixelV3, { 0, 0, 255, 255 });
    DrawLinePixelV3(200, 130, 400, 800, imageDrawLinePixelV3, { 255, 0, 0, 255 });
    DrawLinePixelV3(800, 500, 130, 300, imageDrawLinePixelV3, { 0, 255, 0, 255 });
    imageDrawLinePixelV3.write_tga_file("DrawLinePixelV3.tga");

    TGAImage imageDrawLinePixelV4(width, height, TGAImage::RGB);
    DrawLinePixelV4(130, 200, 800, 400, imageDrawLinePixelV4, { 0, 0, 255, 255 });
    DrawLinePixelV4(200, 130, 400, 800, imageDrawLinePixelV4, { 255, 0, 0, 255 });
    DrawLinePixelV4(800, 500, 130, 300, imageDrawLinePixelV4, { 0, 255, 0, 255 });
    DrawLinePixelV4(300, 900, 550, 200, imageDrawLinePixelV4, { 255, 255, 255, 255 });
    DrawLinePixelV4(300, 500, 900, 200, imageDrawLinePixelV4, { 255, 255, 255, 255 });
    imageDrawLinePixelV4.write_tga_file("DrawLinePixelV4.tga");

    TGAImage imageBresenham(width, height, TGAImage::RGB);
    Bresenham(130, 200, 800, 400, imageBresenham, { 0, 0, 255, 255 });
    Bresenham(200, 130, 400, 800, imageBresenham, { 255, 0, 0, 255 });
    Bresenham(800, 500, 130, 300, imageBresenham, { 0, 255, 0, 255 });
    Bresenham(300, 900, 550, 200, imageBresenham, { 255, 255, 255, 255 });
    Bresenham(300, 500, 900, 200, imageBresenham, { 255, 255, 255, 255 });
    imageBresenham.write_tga_file("Bresenham.tga");

    Model model("../../obj/african_head/african_head.obj");
    TGAImage imageWireFrame(width, height, TGAImage::RGB);
    for (int i = 0; i < model.nfaces(); i++) {
        for (int j = 0; j < 3; j++) {
            vec3 v0 = model.vert(i, j);
            vec3 v1 = model.vert(i, (j + 1) % 3);

            //先要进行模型坐标到屏幕坐标的转换。  (-1,-1)对应(0,0)   (1,1)对应(width,height)
            int x0 = (v0.x + 1) * width / 2;
            int y0 = (v0.y + 1) * height / 2;

            int x1 = (v1.x + 1) * width / 2;
            int y1 = (v1.y + 1) * height / 2;

            Bresenham(x0, y0, x1, y1, imageWireFrame, { 255, 255, 255, 255 });
        }
    }
    imageWireFrame.write_tga_file("WireFrame1.tga");
    return 0;
}
