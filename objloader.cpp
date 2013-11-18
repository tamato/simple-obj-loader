#include "objloader.h"

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <assert.h>
#include <map>

using namespace std;
using namespace sp;

float& Vec3::operator[](int idx)
{
    assert(idx > 0);
    assert(idx < 3);
    return *(&x + idx);
}

float& TexCoord::operator[](int idx)
{
    assert(idx > 0);
    assert(idx < 3);
    return *(&u + idx);
}

struct FaceVert
{
    FaceVert() : vert(-1), norm(-1), coord(-1) {}

    int vert;
    int norm;
    int coord;
};

struct vert_less {
    bool operator() (const FaceVert& lhs, const FaceVert& rhs) const
    {
        // handle any size mesh
        if (lhs.vert != rhs.vert) return (lhs.vert<rhs.vert);
        if (lhs.norm != rhs.norm) return (lhs.norm<rhs.norm);
        if (lhs.coord!=rhs.coord) return (lhs.coord<rhs.coord);
        return false;
        // the following breaks down on very large meshes
        // const unsigned long prime1 = 73856093;
        // const unsigned long prime2 = 19349663;
        // const unsigned long prime3 = 83492791;
        // unsigned long lh = (lhs.vert * prime1) ^ (lhs.norm * prime2) ^ (lhs.coord * prime3);
        // unsigned long rh = (rhs.vert * prime1) ^ (rhs.norm * prime2) ^ (rhs.coord * prime3);
        // return lh < rh;
    }
};

ObjLoader::ObjLoader()
    : TexCoordLayers(1)
{

}

void ObjLoader::load(char* filename)
{
    ifstream inf;
    inf.open(filename, ios_base::in);
    if (!inf.is_open()) {
        cerr << "[!] Failed to load file: " << filename << endl;
    }

    Positions.clear();
    Normals.clear();
    TexCoords.clear();
    Faces.clear();

    char *delims = " \n\r";
    const unsigned int CHARACTER_COUNT = 500;
    char line[CHARACTER_COUNT] = {0};

    std::vector<Vec3> verts;
    std::vector<Vec3> norms;
    std::vector<TexCoord> texcoords;

    std::map<FaceVert, int, vert_less> uniqueverts;
    unsigned int vert_count = 0;

    while (inf.good()) {
        memset( (void*)line, 0, CHARACTER_COUNT);
        inf.getline(line, CHARACTER_COUNT);
        if (inf.eof()) break;

        char *token = strtok(line, delims);
        if (token == NULL || token[0] == '#' || token[0] == '$')
            continue;

        // verts look like:
        //	v float float float
        if (strcmp(token, "v") == 0) {
            float x=0, y=0, z=0, w=1;
            sscanf(line+2, "%f %f %f %f", &x, &y, &z, &w);
            verts.push_back( Vec3(x/w,y/w,z/w) );
        }
        // normals:
        // 	nv float float float
        else if (strcmp(token, "vn") == 0) {
            float x=0, y=0, z=0;
            sscanf(line+3, "%f %f %f", &x, &y, &z);
            norms.push_back( Vec3(x,y,z) );
        }
        // texcoords:
        //	vt	float float
        else if (strcmp(token, "vt") == 0) {
            float x=0, y=0, z=0;
            sscanf(line+3, "%f %f %f", &x, &y, &z);
            texcoords.push_back( TexCoord(x, y) );
        }

        // keep track of smoothing groups
        // s [number|off]
        else if (strcmp(token, "s") == 0) {

        }

        // faces start with:
        //	f
        else if (strcmp(token, "f") == 0) {

            std::vector<int> vindices;
            std::vector<int> nindices;
            std::vector<int> tindices;

            // fill out a triangle from the line, it could have 3 or 4 edges
            char *lineptr = line + 2;
            while (lineptr[0] != 0) {
                while (lineptr[0] == ' ') ++lineptr;

                int vi=0, ni=0, ti=0;
                if (sscanf(lineptr, "%d/%d/%d", &vi, &ni, &ti) == 3) {
                    vindices.push_back(vi-1);
                    nindices.push_back(ni-1);
                    tindices.push_back(ti-1);
                }
                else
                if (sscanf(lineptr, "%d//%d", &vi, &ni) == 2) {
                    vindices.push_back(vi-1);
                    nindices.push_back(ni-1);
                }
                else
                if (sscanf(lineptr, "%d/%d", &vi, &ti) == 2) {
                    vindices.push_back(vi-1);
                    tindices.push_back(ti-1);
                }
                else
                if (sscanf(lineptr, "%d", &vi) == 1) {
                    vindices.push_back(vi-1);
                }

                while(lineptr[0] != ' ' && lineptr[0] != 0) ++lineptr;
            }

            // being that some exporters can export either 3 or 4 sided polygon's
            // convert what ever was exported into triangles
            for (size_t i=1; i<vindices.size()-1; ++i) {
                Face face;
                FaceVert tri;

                tri.vert = vindices[0];
                if (!nindices.empty())
                    tri.norm = nindices[0];
                if (!tindices.empty())
                    tri.norm = tindices[0];

                if (uniqueverts.count(tri) == 0)
                    uniqueverts[tri] = vert_count++;
                face.a = uniqueverts[tri];

                tri.vert = vindices[i];
                if (!nindices.empty())
                    tri.norm = nindices[i];
                if (!tindices.empty())
                    tri.norm = tindices[i];

                if (uniqueverts.count(tri) == 0)
                    uniqueverts[tri] = vert_count++;
                face.b = uniqueverts[tri];

                tri.vert = vindices[i+1];
                if (!nindices.empty())
                    tri.norm = nindices[i+1];
                if (!tindices.empty())
                    tri.norm = tindices[i+1];

                if (uniqueverts.count(tri) == 0)
                    uniqueverts[tri] = vert_count++;
                face.c = uniqueverts[tri];
                Faces.push_back(face);
            }
        }
    }
    inf.close();

    // use resize instead of reserve because we'll be indexing in random locations.
    Positions.resize(vert_count);
    if (norms.size() > 0)
        Normals.resize(vert_count);
    if (texcoords.size() > 0)
        TexCoords.resize(vert_count);

    std::map<FaceVert, int, vert_less>::iterator iter;
    for (iter = uniqueverts.begin(); iter != uniqueverts.end(); ++iter) {

        Positions[iter->second] = verts[iter->first.vert];

        if ( norms.size() > 0 ) {
            Normals[iter->second] = norms[iter->first.norm];
        }

        if ( texcoords.size() > 0) {
            TexCoords[iter->second] = texcoords[iter->first.coord];
        }
    }    
}

int ObjLoader::getIndexCount()
{
    return (int)Faces.size() * 3;
}

int ObjLoader::getVertCount()
{
    return (int)Positions.size();
}

const unsigned int* ObjLoader::getFaces()
{
    return (const unsigned int*)&Faces[0];
}

const float* ObjLoader::getPositions()
{
    return (const float*)&Positions[0];
}

const float* ObjLoader::getNormals()
{
    return (const float*)&Normals[0];
}

int ObjLoader::getTexCoordLayers()
{
    return TexCoordLayers;
}

const float* ObjLoader::getTexCoords(int multiTexCoordLayer)
{
    assert(multiTexCoordLayer < TexCoordLayers);
    return (const float*)&TexCoords[0];
}
