#include "Lesson2.h"

void TriangleRasterize(const vec3* vecs, const vec2* uvs, const vec3& light, const TGAImage& diffuseMap, TGAImage& zBufferImage, TGAImage& image, float* zBuffers);