// Lesson0.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/*
* Lesson 0: getting started
*/

#include <iostream>
#include "tgaimage.h"
int main()
{
    const TGAColor white = { 255, 255, 255, 255 };
    const TGAColor red = { 255, 0, 0, 255 };
    TGAImage image(100, 100, TGAImage::RGB);
    image.set(52, 41, red);
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}
