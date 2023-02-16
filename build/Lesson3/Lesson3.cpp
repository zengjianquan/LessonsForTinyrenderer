// Lesson3.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Lesson3.h"

/*
* 三角形光栅化
* vecs -> (A, B, C)
* 1.质心填充
* 2.背面剔除
* 3.深度缓存
* 4.贴图
*/
void TriangleRasterize(const vec3* vecs, const vec2* uvs, const vec3& light, const TGAImage& diffuseMap, TGAImage& zBufferImage, TGAImage& image, float* zBuffers) {
    const int width = image.width();
    const int height = image.height();
    const int texWidth = diffuseMap.width();
    const int texHeight = diffuseMap.height();

    /*
    * 背面剔除
    */
    vec3 normal = (vecs[2] - vecs[0]) ^ (vecs[1] - vecs[0]);

    // I = -n * l
    float intensity = normal.normalized() * light;
     
    if (intensity < 0)
        return;

    // 屏幕空间
    // (-1, 1)  --> (width, height)
    // (x + 1) / 2 = (x1 - 0) / width
    Vec2i vec2is[] = {
        { int((vecs[0].x + 1) * width / 2), int((vecs[0].y + 1) * width / 2) },
        { int((vecs[1].x + 1) * width / 2), int((vecs[1].y + 1) * width / 2) },
        { int((vecs[2].x + 1) * width / 2), int((vecs[2].y + 1) * width / 2) }
    };

    /*
    * 质心填充
    * 深度缓存 + 贴图 (质心坐标插值)
    */
    //屏幕空间裁切
    Vec2i bBoxmin = { width, height };
    Vec2i bBoxmax = { 0, 0 };
    Vec2i clamp = { width - 1, height - 1 };

    for (int i = 0; i < 3; i++) {
        bBoxmin.x = std::max(0, std::min(bBoxmin.x, vec2is[i].x));
        bBoxmin.y = std::max(0, std::min(bBoxmin.y, vec2is[i].y));

        bBoxmax.x = std::min(clamp.x, std::max(bBoxmax.x, vec2is[i].x));
        bBoxmax.y = std::min(clamp.y, std::max(bBoxmax.y, vec2is[i].y));
    }

    for (int x = bBoxmin.x; x <= bBoxmax.x; x++) {
        for (int y = bBoxmin.y; y <= bBoxmax.y; y++) {
            /*
            * 质心填充
            */
            //[1 - u - v, u, v]
            Vec3f u = Barycentric(vec2is, { x, y });

            if (u.x < 0 || u.y < 0 || u.z < 0)
                continue;

            /*
            * 深度计算
            */
            //zBuffer = (1 - u - v) * Az + u * Bz + v * Cz
            float zBuffer = u.x * vecs[0].z + u.y * vecs[1].z + u.z * vecs[2].z;
            if (zBuffers[x * width + y] >= zBuffer)
                continue;
            
            //记录深度缓冲
            zBuffers[x * width + y] = zBuffer;
            uint8_t zBufferColor = (zBuffer + 1) * 255 / 2;
            zBufferImage.set(x, y, { zBufferColor, zBufferColor, zBufferColor, zBufferColor });

            /*
            * 贴图
            */
            vec2 uv = {
                u.x * uvs[0].x + u.y * uvs[1].x + u.z * uvs[2].x,
                u.x * uvs[0].y + u.y * uvs[1].y + u.z * uvs[2].y
            };

            // (0, 1)  -->  (width, height)
            // (U - 0) / 1 = (tex - 0) / width
            float U = uv.x - int(uv.x);
            float V = uv.y - int(uv.y);

            int uTex = U * texWidth;
            int vTex = V * texHeight;

            TGAColor color = diffuseMap.get(uTex, vTex);

            TGAColor colorIntensity = {
                color.bgra[0] * intensity,
                color.bgra[1] * intensity,
                color.bgra[2] * intensity,
                255
            };

            image.set(x, y, colorIntensity);
        }
    }
}

int main()
{
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

        TriangleRasterize(v, uv, { 0, 0, -1 }, model.diffuse(), imageZBuffer, imageModel, zBuffers);
    }

    imageModel.write_tga_file("Model.tga");
    imageZBuffer.write_tga_file("ZBuffer.tga");
}
