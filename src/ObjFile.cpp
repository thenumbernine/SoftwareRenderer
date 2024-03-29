#include <iostream>
#include <stdio.h>
#include <vector>
#include <math.h>
#include <memory.h>
#include <time.h>

#include "Common/Exception.h"
#include "Common/File.h"

#include "ObjFile.h"
#include "Main.h"

#include "sw.h"

//dont use this flag unless its with homework #2 OBJ files
//#define OFFSET_TO_CENTER

static const char *nextEndLine(const char *p, const char *buffer) {
	while(*p) {
		if (*p == '\n' || *p == '\r') break;
		p++;
	}
	return p;
}

static const char *nextNewLine(const char *p, const char *buffer) {
	while (*p) {
		if (*p != '\n' && *p != '\r') break;
		p++;
	}
	return p;
}

static const char *nextSpace(const char *p, const char *buffer) {
	while (*p) {
		if (*p == ' ' || *p == '\t') break;
		p++;
	}
	return p;
}

static const char *nextNonSpace(const char *p, const char *buffer) {
	while (*p) {
		if (*p != ' ' && *p != '\t') break;
		p++;
	}
	return p;
}

static const char *nextSpaceOrEndLine(const char *p, const char *buffer) {
	while (*p) {
		if (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') break;
		p++;
	}
	return p;
}

template<typename T>
struct ReadVector {
	template<int i>
	struct op {
		static bool exec(const char* buffer, const char* &p, T& v) {
			const char *start = p = nextNonSpace(p, buffer);
			const char *end = p = nextSpaceOrEndLine(p, buffer);
			char token[32];
			int size = 31;
			int strSize = (int)(end - start);
			if (strSize < size) size = strSize;
			if (size < 1) return true;
			memcpy(token, start, size);
			token[size] = 0;
			vec_get<T,i>::get(v) = (float)atof(token);
			return false;
		}
	};

	static void read(const char* buffer, const char* &p, T& v) {
		p = nextSpace(p, buffer);
		Common::ForLoop<0, T::dim, op>::exec(buffer, p, v);
		p = nextNewLine(nextEndLine(p, buffer), buffer);
	}
};

ObjFile::ObjFile(const char *filename) 
	: bbox(
		box3f(
			vec3f(INFINITY,INFINITY,INFINITY),
			vec3f(-INFINITY,-INFINITY,-INFINITY)
		)
	)
{
	std::string bufstr = Common::File::read(filename);
	const char* buffer = bufstr.c_str();

	//holds our current polygon indices
	//used for breaking polys down into triangles
	std::vector<triIndex_t> poly;

	const char *p = buffer;
	while(*p){
		if (*p == '\n' || *p == '\r') {
			continue;
		} else if (*p == '#') {	//comment
			p = nextNewLine(nextEndLine(p, buffer), buffer);
		} else if (*p == 'v') {

			if (p[1] == 't') {
				vec2f v(0,0);
				ReadVector<vec2f>::read(buffer,p,v);
				texCoord.push_back(v);
			} else if (p[1] == 'n') {
				vec3f v(0,0,0);
				ReadVector<vec3f>::read(buffer,p,v);
				normal.push_back(v);
			} else {
				vec3f v(0,0,0);
				ReadVector<vec3f>::read(buffer,p,v);
				bbox.stretch(v);
				vertex.push_back(v);
			}
		} else if (*p == 'f') {
			p = nextSpace(p, buffer);

			for(;;) {
				const char *start = p = nextNonSpace(p, buffer);
				const char *end = p = nextSpaceOrEndLine(p, buffer);

				char token[32];
				int size = 31;
				int strSize = (int)(end - start);
				if (strSize < size) size = strSize;
				if (size < 1) break;
				memcpy(token, start, size);
				token[size] = 0;

				triIndex_t vi = {-1,-1,-1};

				sscanf(token, "%d/%d/%d ", &vi.v, &vi.t, &vi.n);

				//if we couldnt even read the vertex index then skip this one
				//maybe even print out an error or something
				if (vi.v == -1) break;

				//should we keep a flag for every texcoord index that says
				//'the texcoord here exists' ?
				//the Java3D OBJ file loader disregards all texcoords in a group
				//(specified by the 'g' field)
				//if any face leaves out a texcoord
				//(i.e. per group, the texcoord count and vertex count must match)
				if (vi.t == -1) {
					vi.t = 1;
//					flags &= ~bitflag(OBJFILE_FLAG_USE_TEXCOORDS);
				}
				if (vi.n == -1) {
					vi.n = 1;
//					flags &= ~bitflag(OBJFILE_FLAG_USE_NORMALS);
				}

				//subtract one because the obj file starts indexing at 1
				vi.v--;
				vi.t--;
				vi.n--;

				poly.push_back(vi);
			}
			p = nextNewLine(nextEndLine(p, buffer), buffer);

			if ((int)poly.size() < 3) {
				std::cout << "found a face with only " << poly.size() << " vertices!" << std::endl;
			} else {
				//triangulate the polygon
				//TODO - add edge flags (glEdgeFlag style)
				bool high = false;	//false implies left++, true implies right--
				int triIndex[3];
				for (int left = 1, right = (int)poly.size() - 1; left != right; high = !high) {
					if (!high) {
						triIndex[0] = left - 1;
						triIndex[1] = left;
						triIndex[2] = right;
						left++;
					} else {
						triIndex[0] = right - 1;
						triIndex[1] = right;
						triIndex[2] = left - 1;
						right--;
					}

					tri_t t;
					t.p[0] = poly[triIndex[0]];
					t.p[1] = poly[triIndex[1]];
					t.p[2] = poly[triIndex[2]];
					tri.push_back(t);
				}
			}

			poly.clear();

		} else {
			throw Common::Exception() << "got unhandled char " << *p << " " << (int)*p << " at index " << (p - buffer) << " of size " << bufstr.length();
		}
	}

//	if (!texCoord.size()) {
//		flags &= ~bitflag(OBJFILE_FLAG_USE_TEXCOORDS);
//	}
//
//	if (!normal.size()) {
//		flags &= ~bitflag(OBJFILE_FLAG_USE_NORMALS);
//	}

	//verify triangle index validity
	for (int i = 0; i < (int)tri.size(); i++) {
		for (int j = 0; j < 3; j++) {
		
			triIndex_t *t = &tri[i].p[j];
			
			if (t->v < 0 || t->v >= (int)vertex.size()) {
				throw Common::Exception() << "found an out of bounds vertex at tri " << i << " point " << j << " with index " << t->v << ", max " << vertex.size();
			}

//			if (flags & bitflag(OBJFILE_FLAG_USE_TEXCOORDS)) {
//				if (t->t < 0 || t->t >= (int)texCoord.size()) {
//					printf("found an out of bounds texcoord index %d, max %d\n", t->t, texCoord.size());
//					flags &= ~bitflag(OBJFILE_FLAG_USE_TEXCOORDS);
//				}
//			}
//
//			if (flags & bitflag(OBJFILE_FLAG_USE_NORMALS)) {
//				if (t->n < 0 || t->n >= (int)normal.size()) {
//					printf("found an out of bounds normal index %d, max %d\n", t->n, normal.size());
//					flags &= ~bitflag(OBJFILE_FLAG_USE_NORMALS);
//				}
//			}
		}
	}

	//remove any flags if we didnt find any instances of the elements

//	if (!(flags & bitflag(OBJFILE_FLAG_USE_TEXCOORDS))) {
//		texCoord.clear();
//	}
//
//	if (!(flags & bitflag(OBJFILE_FLAG_USE_NORMALS))) {
//		normal.clear();
	if (!normal.size()) {
		std::cout << "found no/invaild normals - generating..." << std::endl;

		/* normal generation */

		//use this for vertices with no normals specified
		//such vertices are ones for which every face that references them never
		//provides an associated normal
		//I think.

		normal.resize(vertex.size());

		for (int i = 0; i < (int)normal.size(); i++) {
			normal[i] = vec3f(0,0,0);
		}
		for (int i = 0; i < (int)tri.size(); i++) {
			//copy over vertex references
			tri[i].p[0].n = tri[i].p[0].v;
			tri[i].p[1].n = tri[i].p[1].v;
			tri[i].p[2].n = tri[i].p[2].v;
			vec3f n = vecUnitNormal(
				vertex[tri[i].p[0].v],
				vertex[tri[i].p[1].v],
				vertex[tri[i].p[2].v]);
			normal[tri[i].p[0].n] += n;
			normal[tri[i].p[1].n] += n;
			normal[tri[i].p[2].n] += n;
		}
		for (int i = 0; i < (int)normal.size(); i++) {
			normal[i] = vecUnit(normal[i]);
		}

		/* end normal generation */

		std::cout << "done generating normals." << std::endl;

//		flags |= bitflag(OBJFILE_FLAG_USE_NORMALS);
	}

	// generate edges

	//for every pair of faces
//	clock_t startTime = clock();
//	int lastSecond = 0;

	for (int i = 0; i < (int)tri.size() - 1; i++) {

		//for complex models...
//		{
//			clock_t thisTime = clock();
//			int second = (thisTime - startTime) / (10 * CLOCKS_PER_SEC);
//			if (second != lastSecond) {
//				lastSecond = second;
//				printf(" %d of %d (%d%%) done \n", i, tri.size(), (i*100/(int)tri.size()) );
//			}
//		}


		for (int j = i + 1; j < (int)tri.size(); j++) {

			edge_t e;
			e.t[0] = i;
			e.t[1] = j;

			bool found = false;

			for (int k = 0; k < 3 && !found; k++) {
				int kk = (k + 1) % 3;
				for (int l = 0; l < 3; l++) {
					int ll = (l + 1) % 3;
					
					if (tri[i].p[kk].v == tri[j].p[l].v &&
						tri[i].p[k].v == tri[j].p[ll].v)
					{

						e.v[0] = k;
						e.v[1] = l;
						edge.push_back(e);

						found = true;

						break;
					}
				}
			}
		}
	}

#ifdef OFFSET_TO_CENTER
	{
		vec3f center = (bbox.max + bbox.min) * 0.5f;
		for (i = 0; i < (int)vertex.size(); i++) {
			vertex[i] -= center;
		}
		bbox.max -= center;
		bbox.min -= center;
	}
#endif

	// generate planes
	for (int i = 0; i < (int)tri.size(); i++) {
		tri[i].plane = vecUnitNormal(
			vertex[tri[i].p[0].v],
			vertex[tri[i].p[1].v],
			vertex[tri[i].p[2].v]);
		tri[i].plane.w = -dot(*tri[i].plane.vp(), vertex[tri[i].p[0].v]);
	}

	//calc bounding radius
	brad = vecLengthSq(vertex[0]);
	for (int i = 1; i < (int)vertex.size(); i++) {
		float r = vecLengthSq(vertex[i]);
		if (r > brad) brad = r;
	}
	brad = (float)sqrt(brad);


//	printf("found %d vertices, %d normals, %d texcoords, %d faces\n",
//		vertex.size(), normal.size(), texCoord.size(), tri.size());
//	printf("radius %f bbox %f,%f,%f to %f,%f,%f\n",
//		brad, bbox.min.x, bbox.min.y, bbox.min.z,
//		bbox.max.x, bbox.max.y,bbox.max.z);
//	printf("loaded model %s\n", filename);

	title = filename;
}


void ObjFile::render() const {
	for (int i = 0; i < (int)tri.size(); i++) {
		swBegin(SW_POLYGON);
		for (int j = 0; j < 3; j++) {
			swTexCoord2f(VEC2ELEM(texCoord[tri[i].p[j].t]));
			swNormal3f(VEC3ELEM(normal[tri[i].p[j].n]));
			swVertex3f(VEC3ELEM(vertex[tri[i].p[j].v]));
		}
		swEnd();
	}
}
