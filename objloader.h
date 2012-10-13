#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>

namespace sp // simple and to the point
{

struct Vec3
{
    float x;
    float y;
    float z;
    Vec3(): x(0), y(0), z(0) {}
    Vec3(float _x, float _y, float _z): x(_x), y(_y), z(_z) {}
    float& operator[](int idx);
};

struct TexCoord
{
    float u;
    float v;
    TexCoord(): u(0), v(0) {}
    TexCoord(float _u, float _v): u(_u), v(_v) {}
    float& operator[](int idx);
};

struct Face
{
    int a;
    int b;
    int c;
};

class ObjLoader
{
public:
    ObjLoader();

    void load(char* filename);

    int getIndexCount();
    int getVertCount();

    const unsigned int* getFaces();
    const float* getPositions();
    const float* getNormals();

    int getTexCoordLayers();
    const float* getTexCoords(int multiTexCoordLayer);

private:

    std::vector<Face> Faces;
    std::vector<Vec3> Positions;
    std::vector<Vec3> Normals;

    // obj's only have 1 layer ever
    std::vector<TexCoord> TexCoords;
    unsigned int TexCoordLayers;
};
}
#endif // OBJLOADER_H
