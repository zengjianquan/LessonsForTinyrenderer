#include "Lesson4.h"

void TriangleRasterize(vec3* vecs, vec2* uvs, vec3* normals, const Matrix& modelView, const Matrix& projection, const vec3& light, const TGAImage& diffuseMap, TGAImage& zBufferImage, TGAImage& outputImage, float* zBuffers);