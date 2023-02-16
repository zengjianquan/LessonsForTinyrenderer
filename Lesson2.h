#pragma once
#include <cassert>
#include "Lesson1.h"

template <class T>
struct Vec2 {
	T x;
	T y;

	Vec2(T x, T y) :
		x(x), y(y){}

	Vec2<T> operator+(const Vec2& other) const {
		return { x + other.x, y + other.y };
	}

	Vec2<T> operator-(const Vec2& other) const {
		return { x - other.x, y - other.y };
	}

	Vec2<T> operator*(float t) const {
		return { T(x * t), T(y * t) };
	}

	T& operator[](const int i) {
		assert(i >= 0 && i < 2);
		return i ? x : y;
	}

	T operator[](const int i) const {
		assert(i >= 0 && i < 2);
		return i ? x : y;
	}

};

template<class T>
struct Vec3 {
	T x;
	T y;
	T z;

	Vec3(T x, T y, T z) :
		x(x), y(y), z(z)
	{}

	float norm() const {
		return std::sqrt(x * x + y * y + z * z);
	}

	Vec3<T> operator+(const Vec3<T> other) const {
		return { x + other.x, y + other.y, z + other.z };
	}

	Vec3<T> operator*(float t) const {
		return { x * t, y * t, z * t };
	}

	T& operator[](const int i) {
		assert(i >= 0 && i < 3);
		return i ? (i == 1 ? y : z) : x;
	}

	T operator*(const Vec3<T> other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	/*
	* x			x1
	* y			y1
	* z			z1
	* 
	* y * x1 - z * y1,
	* z * x1 - x * z1,
	* x * y1 - y * x1,
	*/
	Vec3<T> operator^(const Vec3<T> other) const {
		return { 
			y* other.z - z * other.y,
			z* other.x - x * other.z,
			x* other.y - y * other.x
		};
	}
};


typedef Vec2<int> Vec2i;
typedef Vec3<double> Vec3d;
typedef Vec3<float> Vec3f;

//三角形光栅化(扫描线填充)
void TriangleRasterization(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage& image, const TGAColor& color);

//三角形光栅化(扫面线填充) + 背面剔除
void TriangleRasterizationWithBackFaceCulling(vec3 v0, vec3 v1, vec3 v2, TGAImage& image, const TGAColor& color, vec3 light);

//三角形光栅化(质心填充)
void TriangleRasterization(const Vec2i* vecs, TGAImage& image, const TGAColor& color);

//三角形光栅化(质心填充) + 背面剔除
void TriangleRasterizationWithBackFaceCulling(vec3* vec3s, TGAImage& image, const TGAColor& color, vec3 light);

//质心计算
Vec3f Barycentric(const Vec2i* vecs, const Vec2i& p);