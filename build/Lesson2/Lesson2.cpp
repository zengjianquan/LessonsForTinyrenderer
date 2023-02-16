// Lesson2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/*
* Lesson 2: Triangle rasterization and back face culling
* 三角形栅格化和背面剔除
*/

#include <iostream>
#include "Lesson2.h"

//三角形光栅化(扫描线填充)
void TriangleRasterization(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage& image, const TGAColor& color) {
	//对顶点进行 y轴 升序排序
	if (v0.y > v1.y) std::swap(v0, v1);
	if (v1.y > v2.y) std::swap(v1, v2);
	if (v0.y > v1.y) std::swap(v0, v1);

	//扫描线
	for (int y = v0.y; y <= v1.y; y++) {
		float x0 = float(v2.x - v0.x) * (y - v0.y) / (v2.y - v0.y) + v0.x;
		float x1 = float(v1.x - v0.x) * (y - v0.y) / (v1.y - v0.y) + v0.x;

		if (x0 > x1)std::swap(x0, x1);

		for (int x = x0; x <= x1; x++) {
			image.set(x, y, color);
		}
	}

    for (int y = v1.y; y <= v2.y; y++) {
        float x0 = float(v2.x - v0.x) * (y - v0.y) / (v2.y - v0.y) + v0.x;
        float x1 = float(v2.x - v1.x) * (y - v1.y) / (v2.y - v1.y) + v1.x;

        if (x0 > x1)std::swap(x0, x1);

        for (int x = x0; x <= x1; x++) {
            image.set(x, y, color);
        }
    }
}

//三角形光栅化(扫面线填充) + 背面剔除
void TriangleRasterizationWithBackFaceCulling(vec3 v0, vec3 v1, vec3 v2, TGAImage& image, const TGAColor& color, vec3 light) {
	vec3 normal = (v2 - v0) ^ (v1 - v0);
	float intensity = light * normal.normalized();

	if (intensity <= 0)
		return;

	//(-1, 1) -> (0, width)
	//(x + 1) / 2 == (x1 - 0) / width
	//x1 = (x + 1) / 2 * width
	int width = image.width();
	int height = image.height();
	Vec2i V0 = { int((v0.x + 1) / 2 * width), int((v0.y + 1) / 2 * height) };
	Vec2i V1 = { int((v1.x + 1) / 2 * width), int((v1.y + 1) / 2 * height) };
	Vec2i V2 = { int((v2.x + 1) / 2 * width), int((v2.y + 1) / 2 * height) };

	TGAColor iColor = { 255 * intensity ,
	255 * intensity ,
	255 * intensity ,
	255 };

	TriangleRasterization(V0, V1, V2, image, iColor);
}


//质心计算
Vec3f Barycentric(const Vec2i* vecs, const Vec2i& p) {
	/*
	* [AB, AC, PA]x X [AB, AC, PA]y = k[u, v, 1]
	*/
	Vec3f u = Vec3f(vecs[1][0] - vecs[0][0], vecs[2][0] - vecs[0][0], vecs[0][0] - p[0])
		^ Vec3f(vecs[1][1] - vecs[0][1], vecs[2][1] - vecs[0][1], vecs[0][1] - p[1]);
	
	//当 k = 0 时, 三角形三点共线
	if (std::abs(u.z) < 1e-2)
		return { -1, 1, 1 };

	//[1 - u - v, u, v]
	return { 1 - (u.x + u.y) / u.z, u.x / u.z, u.y / u.z };	
}

//三角形光栅化(质心填充)
void TriangleRasterization(const Vec2i* vecs, TGAImage& image, const TGAColor& color) {
	Vec2i clamp(image.width() - 1, image.height() - 1);
	Vec2i minBBox = clamp;
	Vec2i maxBBox(0, 0);

	for (int i = 0; i < 3; i++) {
		minBBox.x = std::max(0, std::min(minBBox.x, vecs[i].x));
		minBBox.y = std::max(0, std::min(minBBox.y, vecs[i].y));

		maxBBox.x = std::min(clamp.x, std::max(maxBBox.x, vecs[i].x));
		maxBBox.y = std::min(clamp.y, std::max(maxBBox.y, vecs[i].y));
	}

	for (int x = minBBox.x; x <= maxBBox.x; x++) {
		for (int y = minBBox.y; y <= maxBBox.y; y++) {
			//[1 - u - v, u, v]
			Vec3f u = Barycentric(vecs, { x, y });
			
			// == 0 时, p在三角形上
			if (u.x < 0 || u.y < 0 || u.z < 0)
				continue;

			image.set(x, y, color);
		}
	}
}

//三角形光栅化(质心填充) + 背面剔除
void TriangleRasterizationWithBackFaceCulling(vec3* vec3s, TGAImage& image, const TGAColor& color, vec3 light) {
	vec3 normal = (vec3s[2] - vec3s[0]) ^ (vec3s[1] - vec3s[0]);

	// I = -n * l
	float intensity = light * normal.normalized();

	if (intensity <= 0)
		return;

	//(-1, 1) -> (0, width)
	//(x + 1) / 2 == (x1 - 0) / width
	//x1 = (x + 1) / 2 * width
	int width = image.width();
	int height = image.height();
	Vec2i V[] = {
		{ int((vec3s[0].x + 1) / 2 * width), int((vec3s[0].y + 1) / 2 * height) },
		{ int((vec3s[1].x + 1) / 2 * width), int((vec3s[1].y + 1) / 2 * height) },
		{ int((vec3s[2].x + 1) / 2 * width), int((vec3s[2].y + 1) / 2 * height) }
	};

	TGAColor iColor = { 255 * intensity ,
	255 * intensity ,
	255 * intensity ,
	255 };

	TriangleRasterization(V, image, iColor);
}

int main()
{
	int width = 1000, height = 1000;
	TGAImage image(width, height, TGAImage::RGBA);
	Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
	Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
	Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };

	TriangleRasterization(t0[0], t0[1], t0[2], image, { 0, 0, 255, 255});
	TriangleRasterization(t1[0], t1[1], t1[2], image, { 0, 255, 255, 255 });
	TriangleRasterization(t2[0], t2[1], t2[2], image, { 0, 255, 0, 255 });

	image.write_tga_file("triangle.tga");

	Model model("../../obj/african_head/african_head.obj");
	TGAImage imageWireFrame(width, height, TGAImage::RGB);
	TGAImage imageModel(width, height, TGAImage::RGB);
	TGAImage imageModelBarycentric(width, height, TGAImage::RGB);
	for (int i = 0; i < model.nfaces(); i++) {
		vec3 v0 = model.vert(i, 0);
		vec3 v1 = model.vert(i, 1);
		vec3 v2 = model.vert(i, 2);

		vec3 v[] = { v0, v1, v2 };

		TriangleRasterizationWithBackFaceCulling(v0, v1, v2, imageModel, { 255, 255, 255, 255 }, { 0, 0, -1 });

		TriangleRasterizationWithBackFaceCulling(v, imageModelBarycentric, { 255, 255, 255, 255 }, { 0, 0, -1 });

		//先要进行模型坐标到屏幕坐标的转换。  (-1,-1)对应(0,0)   (1,1)对应(width,height)
		int x0 = (v0.x + 1) * width / 2;
		int y0 = (v0.y + 1) * height / 2;

		int x1 = (v1.x + 1) * width / 2;
		int y1 = (v1.y + 1) * height / 2;

		int x2 = (v2.x + 1) * width / 2;
		int y2 = (v2.y + 1) * height / 2;
		TriangleRasterization({ x0, y0 }, { x1, y1 }, { x2, y2 }, imageWireFrame, { 255, 255, 255, 255 });
	}

	imageWireFrame.write_tga_file("WireFrame.tga");
	imageModel.write_tga_file("Model.tga");
	imageModelBarycentric.write_tga_file("ModelBarycentric.tga");
}