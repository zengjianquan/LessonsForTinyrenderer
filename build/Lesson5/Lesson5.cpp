// Lesson5.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "Lesson5.h"

void TriangleRasterize(vec3* vecs, vec2* uvs, vec3* normals, const Matrix& modelView, const Matrix& projection, const vec3& light, const TGAImage& diffuseMap, TGAImage& zBufferImage, TGAImage& outputImage, float* zBuffers)
{
    //背面剔除
    vec3 negativeCameraZ = { -modelView[2][0], -modelView[2][1], -modelView[2][2] };
    vec3 normal = ((vecs[2] - vecs[0]) ^ (vecs[1] - vecs[0])).normalized();
    if (negativeCameraZ * normal < 0)
        return;

    const int width = outputImage.width();
    const int height = outputImage.height();
    const int textureWidth = diffuseMap.width();
    const int textureHeight = diffuseMap.width();

    /*
    * viewport matrix
    * 视角变换矩阵
    */
    // (-1, 1) / (-1, 1) -->> (0, width) / (0, height)
    // (x + 1) / 2 = (x' - 0) / width
    // x' = (x + 1) * width / 2 = x * width / 2 + width / 2
    // 
    // | width / 2,     0     , 0, width / 2 |
    // |    0     , height / 2, 0, height / 2|
    // |    0     ,     0     , 1,      0    |
    // |    0     ,     0     , 0,      1    |
    Matrix viewportMat = Matrix::identity();
    viewportMat[0][0] = width / 2;
    viewportMat[0][3] = width / 2;
    viewportMat[1][1] = height / 2;
    viewportMat[1][3] = height / 2;

    vec3 v0 = vec3MultiplyMatrix(vecs[0], viewportMat * projection * modelView);
    vec3 v1 = vec3MultiplyMatrix(vecs[1], viewportMat * projection * modelView);
    vec3 v2 = vec3MultiplyMatrix(vecs[2], viewportMat * projection * modelView);

    Vec2i v[] = {
        {int(v0.x), int(v0.y)},
        {int(v1.x), int(v1.y)},
        {int(v2.x), int(v2.y)}
    };

    Vec2i bBoxMin = { width, height };
    Vec2i bBoxMax = { 0, 0 };
    Vec2i bBoxClamp = { width, height };

    for (int i = 0; i < 3; i++) {
        bBoxMin.x = std::max(std::min(bBoxMin.x, v[i].x), 0);
        bBoxMin.y = std::max(std::min(bBoxMin.y, v[i].y), 0);

        bBoxMax.x = std::min(std::max(bBoxMax.x, v[i].x), bBoxClamp.x);
        bBoxMax.y = std::min(std::max(bBoxMax.y, v[i].y), bBoxClamp.y);
    }

    for (int x = bBoxMin.x; x <= bBoxMax.x; x++) {
        for (int y = bBoxMin.y; y <= bBoxMax.y; y++) {
            Vec3f u = Barycentric(v, { x, y });
            if (u.x < 0 || u.y < 0 || u.z < 0)
                continue;

            float zBuffer = u.x * vecs[0].z + u.y * vecs[1].z + u.z * vecs[2].z;
            if (zBuffers[x * width + y] >= zBuffer)
                continue;

            zBuffers[x * width + y] = zBuffer;
            uint8_t zBufferColor = (zBuffer + 1) * 255 / 2;
            zBufferImage.set(x, y, { zBufferColor, zBufferColor, zBufferColor, zBufferColor });

            // (0, U) / (0, V) -->> (0, textureWidth) / (0, textureHeight)
            // u / U = x / textureWidth
            vec2 uv = {
                u.x * uvs[0].x + u.y * uvs[1].x + u.z * uvs[2].x,
                u.x * uvs[0].y + u.y * uvs[1].y + u.z * uvs[2].y
            };
            int texX = (uv.x - int(uv.x)) * textureWidth;
            int texY = (uv.y - int(uv.y)) * textureHeight;
            
            //intensity: Gouraud Shading 高洛德着色
            /*
            * 对平面中的点插值
            * 根据质心坐标以及通过顶点法线计算的光照强度, 计算当前点的光照强度
            */
            //直接使用插值计算当前点的法向量
            vec3 pNormal = {
                u.x* normals[0].x + u.y * normals[1].x + u.z * normals[2].x,
                u.x* normals[0].y + u.y * normals[1].y + u.z * normals[2].y,
                u.x* normals[0].z + u.y * normals[1].z + u.z * normals[2].z
            };

            float intensity = light * pNormal.normalized() * -1;

            if (intensity < 0)
                intensity = 0;

            TGAColor diffuseSample = diffuseMap.get(texX, texY);
            for (int i = 3; i--;) {
                diffuseSample.bgra[i] *= intensity;
            }
            diffuseSample.bgra[3] = 255;
            outputImage.set(x, y, diffuseSample);
        }
    }

}

int main()
{
    //相机位置
    vec3 camera = { 0, 0, 1 };

    //眼睛位置
    vec3 eye = { 2, 1, 3 };

    //向上向量
    vec3 up = { 0, 1, 0 };

    //光线方向
    vec3 light = { 0, 0, -1 };
    light.normalized();

    //构造透视变换矩阵 (4， 4)
    /*
    * | 1,  0,  0,  0 |
    * | 0,  1,  0,  0 |
    * | 0,  0,  1,  0 |
    * | 0,  0, -1/c,1 |
    */
    Matrix projectMatrix = Matrix::identity();
    projectMatrix[3][2] = -1.0f / (camera - eye).norm();

    //构造视图变换矩阵
    vec3 cameraZ = (eye - camera).normalized();
    vec3 cameraX = (up ^ cameraZ).normalized();
    vec3 cameraY = (cameraZ ^ cameraX).normalized();
    Matrix modelView = Matrix::identity();
    for (int i = 0; i < 3; i++) {
        //平移项
        modelView[i][3] = -camera[i];

        //旋转项
        modelView[0][i] = cameraX[i];
        modelView[1][i] = cameraY[i];
        modelView[2][i] = cameraZ[i];
    }

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
        vec3 v[] = { v0, v1, v2 };

        vec2 uv0 = model.uv(i, 0);
        vec2 uv1 = model.uv(i, 1);
        vec2 uv2 = model.uv(i, 2);
        vec2 uv[] = { uv0, uv1, uv2 };

        vec3 n0 = model.normal(i, 0);
        vec3 n1 = model.normal(i, 1);
        vec3 n2 = model.normal(i, 2);
        vec3 n[] = { n0, n1, n2 };

        TriangleRasterize(v, uv, n, modelView, projectMatrix, light, model.diffuse(), imageZBuffer, imageModel, zBuffers);
    }

    imageModel.write_tga_file("Model.tga");
    imageZBuffer.write_tga_file("ZBuffer.tga");
}