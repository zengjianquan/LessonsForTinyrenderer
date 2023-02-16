// Lesson4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include"Lesson4.h"

vec3 vec3MultiplyMatrix(const vec3& v, const Matrix& mat) {
    vec4 homogeneousVec3;
    homogeneousVec3[3] = 1;
    for (int i = 3; i--; homogeneousVec3.data[i] = v[i]);
    homogeneousVec3 = mat * homogeneousVec3;

    vec3 result;
    for (int i = 3; i--; result[i] = homogeneousVec3[i] / homogeneousVec3[3]);
    return result;
}

int main()
{
    //构造相机位置
    Vec3f camera(0, 0, 3);

    //构造透视变换矩阵 (4， 4)
    /*
    * | 1,  0,  0,  0 |
    * | 0,  1,  0,  0 |
    * | 0,  0,  1,  0 |
    * | 0,  0,  -1/c,1 |
    */
    Matrix projectMatrix = Matrix::identity();
    projectMatrix[3][2] = -1.0f / camera.z;

    int width = 1000, height = 1000;
    Model model("../../obj/african_head/african_head.obj");
    TGAImage imageModel(width, height, TGAImage::RGB);
    TGAImage imageZBuffer(width, height, TGAImage::RGB);

    float* zBuffers = new float[width * height];
    for (int i = 0; i < width * height; i++)
        zBuffers[i] = -std::numeric_limits<float>::max();

    for (int i = 0; i < model.nfaces(); i++) {
        vec3 v0 = model.vert(i, 0);
        vec3 v1 = model.vert(i, 1);
        vec3 v2 = model.vert(i, 2);

        v0 = vec3MultiplyMatrix(v0, projectMatrix);
        v1 = vec3MultiplyMatrix(v1, projectMatrix);
        v2 = vec3MultiplyMatrix(v2, projectMatrix);

        vec3 v[] = { v0, v1, v2 };

        vec2 uv0 = model.uv(i, 0);
        vec2 uv1 = model.uv(i, 1);
        vec2 uv2 = model.uv(i, 2);

        vec2 uv[] = { uv0, uv1, uv2 };

        TriangleRasterize(v, uv, { 0, 0, -1 }, model.diffuse(), imageZBuffer, imageModel, zBuffers);
    }

    imageModel.write_tga_file("Model.tga");
    imageZBuffer.write_tga_file("ZBuffer.tga");
}
