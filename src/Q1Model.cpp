#include <stdio.h>
#include <assert.h>

//#include "../m_foundation/main.h"

#include <memory.h>
#include "Q1Model.h"

#include "sw.h"

using byte = uint8_t;

void Q1Model::Reset() {
	skin = nullptr;
	memset(&mdl, 0, sizeof(mdl));
	vtx = nullptr;
	tri = nullptr;
	frame = nullptr;
	num_poses = 0;
	normal = nullptr;
}

void Q1Model::Unload() {
	//unalloc
	if (skin) free(skin);
	if (vtx) free(vtx);
	if (tri) free(tri);
	if (frame) free(frame);
	if (pose) free(pose);
	if (normal) free(normal);

	//reset
	Reset();
}

/*
file format;

mdl_t mdl;

{
	aliasskintype_t aliasskintype;
	if (aliasskintype == ALIAS_SKIN_SINGLE) {
		byte skin[mdl.skinheight][mdl.skinwidth];
	} else {
		int		group_numskins;							//seems to be within [1,4]
		float	intervals[group_numskins];				//seems to be unused
		byte	skin[group_numskins][mdl.skinheight][mdl.skinwidth];
	}
} [mdl.numskins]

stvert_t stverts[mdl.numverts];

dtriangle_t triangles[mdl.numtris]

{
	aliasframetype_t frametype;
	if (frametype == ALIAS_SINGLE) {
		daliasframe_t	aliasframe;
		trivertx_t		vertexes[mdl.numverts];
	} else {
		daliasgroup_t	aliasgroup;
		float			intervals[aliasgroup.numframes]
		{
			daliasframe_t	frame;
			trivertx_t		vertexes[mdl.numverts]
		} [aliasgroup.numframes]
	}
} [mdl.numframes]
*/

static unsigned char quakepal[256][3] = {
	{0,0,0},{15,15,15},{31,31,31},{47,47,47},{63,63,63},{75,75,75},{91,91,91},{107,107,107},{123,123,123},{139,139,139},{155,155,155},{171,171,171},{187,187,187},{203,203,203},{219,219,219},{235,235,235},
	{15,11,7},{23,15,11},{31,23,11},{39,27,15},{47,35,19},{55,43,23},{63,47,23},{75,55,27},{83,59,27},{91,67,31},{99,75,31},{107,83,31},{115,87,31},{123,95,35},{131,103,35},{143,111,35},
	{11,11,15},{19,19,27},{27,27,39},{39,39,51},{47,47,63},{55,55,75},{63,63,87},{71,71,103},{79,79,115},{91,91,127},{99,99,139},{107,107,151},{115,115,163},{123,123,175},{131,131,187},{139,139,203},
	{0,0,0},{7,7,0},{11,11,0},{19,19,0},{27,27,0},{35,35,0},{43,43,7},{47,47,7},{55,55,7},{63,63,7},{71,71,7},{75,75,11},{83,83,11},{91,91,11},{99,99,11},{107,107,15},
	{7,0,0},{15,0,0},{23,0,0},{31,0,0},{39,0,0},{47,0,0},{55,0,0},{63,0,0},{71,0,0},{79,0,0},{87,0,0},{95,0,0},{103,0,0},{111,0,0},{119,0,0},{127,0,0},
	{19,19,0},{27,27,0},{35,35,0},{47,43,0},{55,47,0},{67,55,0},{75,59,7},{87,67,7},{95,71,7},{107,75,11},{119,83,15},{131,87,19},{139,91,19},{151,95,27},{163,99,31},{175,103,35},
	{35,19,7},{47,23,11},{59,31,15},{75,35,19},{87,43,23},{99,47,31},{115,55,35},{127,59,43},{143,67,51},{159,79,51},{175,99,47},{191,119,47},{207,143,43},{223,171,39},{239,203,31},{255,243,27},
	{11,7,0},{27,19,0},{43,35,15},{55,43,19},{71,51,27},{83,55,35},{99,63,43},{111,71,51},{127,83,63},{139,95,71},{155,107,83},{167,123,95},{183,135,107},{195,147,123},{211,163,139},{227,179,151},
	{171,139,163},{159,127,151},{147,115,135},{139,103,123},{127,91,111},{119,83,99},{107,75,87},{95,63,75},{87,55,67},{75,47,55},{67,39,47},{55,31,35},{43,23,27},{35,19,19},{23,11,11},{15,7,7},
	{187,115,159},{175,107,143},{163,95,131},{151,87,119},{139,79,107},{127,75,95},{115,67,83},{107,59,75},{95,51,63},{83,43,55},{71,35,43},{59,31,35},{47,23,27},{35,19,19},{23,11,11},{15,7,7},
	{219,195,187},{203,179,167},{191,163,155},{175,151,139},{163,135,123},{151,123,111},{135,111,95},{123,99,83},{107,87,71},{95,75,59},{83,63,51},{67,51,39},{55,43,31},{39,31,23},{27,19,15},{15,11,7},
	{111,131,123},{103,123,111},{95,115,103},{87,107,95},{79,99,87},{71,91,79},{63,83,71},{55,75,63},{47,67,55},{43,59,47},{35,51,39},{31,43,31},{23,35,23},{15,27,19},{11,19,11},{7,11,7},
	{255,243,27},{239,223,23},{219,203,19},{203,183,15},{187,167,15},{171,151,11},{155,131,7},{139,115,7},{123,99,7},{107,83,0},{91,71,0},{75,55,0},{59,43,0},{43,31,0},{27,15,0},{11,7,0},
	{0,0,255},{11,11,239},{19,19,223},{27,27,207},{35,35,191},{43,43,175},{47,47,159},{47,47,143},{47,47,127},{47,47,111},{47,47,95},{43,43,79},{35,35,63},{27,27,47},{19,19,31},{11,11,15},
	{43,0,0},{59,0,0},{75,7,0},{95,7,0},{111,15,0},{127,23,7},{147,31,7},{163,39,11},{183,51,15},{195,75,27},{207,99,43},{219,127,59},{227,151,79},{231,171,95},{239,191,119},{247,211,139},
	{167,123,59},{183,155,55},{199,195,55},{231,227,87},{127,191,255},{171,231,255},{215,255,255},{103,0,0},{139,0,0},{179,0,0},{215,0,0},{255,0,0},{255,243,147},{255,247,199},{255,255,255},{159,91,83},
};

static const unsigned char *Quake1_ConvertData_8to24(const unsigned char *data, unsigned int size) {
	static unsigned char trans[640*480*3];
	unsigned char *dest = trans;
	for ( ; size ; size--) {
		dest[0] = quakepal[*data][0];
		dest[1] = quakepal[*data][1];
		dest[2] = quakepal[*data][2];

		dest += 3;
		data++;
	}
	return trans;
}

static const unsigned char *stretchToPowerOfTwo(unsigned int *width, unsigned int *height, const unsigned char *src) {
	static unsigned char stretch[1024*512*3];
	unsigned int oldWidth = *width, oldHeight = *height;
	unsigned int newWidth = 2, newHeight = 2;
	for (oldWidth >>= 1; oldWidth; oldWidth >>= 1, newWidth <<= 1);
	for (oldHeight >>= 1; oldHeight; oldHeight >>= 1, newHeight <<= 1);

	unsigned char *s = stretch;
	for (unsigned int j = 0; j < newHeight; j++) {
		int fy = (int)((float)j / (float)(newHeight - 1) * (float)height[0]);
		for (unsigned int i = 0; i < newWidth; i++) {
			int fx = (int)((float)i / (float)(newWidth - 1) * (float)width[0]);
			const unsigned char *srcp = src + ( fx + fy * width[0] ) * 3;
			*s = *srcp;  s++; srcp++;
			*s = *srcp;  s++; srcp++;
			*s = *srcp;  s++; srcp++;
		}
	}

	*width = newWidth;
	*height = newHeight;
	return stretch;
}

bool Q1Model::LoadFromBuffer(const char *read_buffer) {

	int i,j,s;

	//header
	memcpy(&mdl, read_buffer, sizeof(q1_mdl_t));
	read_buffer += sizeof(q1_mdl_t);

	//validate
	if (mdl.numskins < 1 || mdl.numskins > MAX_SKINS) {
		printf("bad skin # on model\n");
		return false;
	}

	//skins
	skin = (unsigned int *)malloc(sizeof(int) * mdl.numskins * 4);
	if (!skin) {
		printf("unable to alloc texture array\n");
		return false;
	}

	s = mdl.skinwidth * mdl.skinheight;
	q1_aliasskintype_t *aliasskintype = (q1_aliasskintype_t *)read_buffer;
	read_buffer += sizeof(q1_aliasskintype_t);
	for (i = 0; i < mdl.numskins; i++) {
		if (*aliasskintype == ALIAS_SKIN_SINGLE) {
			const unsigned char *trans = Quake1_ConvertData_8to24((const unsigned char *)read_buffer, s);
			read_buffer += s;

			swGenTextures(1, &skin[i<<2]);

//			skin[i<<2].Extract(, GL_RGBA, GL_UNSIGNED_BYTE, trans, 4, 4*mdl.skinwidth);
			swBindTexture(SW_TEXTURE_2D, skin[i<<2]);

			unsigned int newWidth = mdl.skinwidth;
			unsigned int newHeight = mdl.skinheight;
			const unsigned char *stretch = stretchToPowerOfTwo(&newWidth, &newHeight, trans);

			swTexImage2D(SW_TEXTURE_2D, newWidth, newHeight, stretch);

			//fill other skins?
		} else {
			printf(" I FOUND A GROUP SKIN WITHIN THIS MODEL!\n");
			int group_numskins = *(int *)read_buffer;
			if (group_numskins < 0 || group_numskins > 4) {
				printf("found oob group_numskins: %d\n", group_numskins);
				return false;
			}
			//skip over intervals while we're at it
			read_buffer += sizeof(int) + sizeof(float) * group_numskins;
			for (j = 0; j < group_numskins; j++) {
				const unsigned char *trans = Quake1_ConvertData_8to24((const unsigned char *)read_buffer, s);

				swGenTextures(1, &skin[(i<<2)+j]);
				swBindTexture(SW_TEXTURE_2D, skin[(i<<2)+j]);
				swTexImage2D(SW_TEXTURE_2D, mdl.skinwidth, mdl.skinheight, trans);
				//skin[(i<<2)+j].Extract(mdl.skinwidth, mdl.skinheight, GL_RGBA, GL_UNSIGNED_BYTE, trans, 4, 4*mdl.skinwidth);

				read_buffer += s;
			}
			//copy over other skins?
		}
	}

	//geometry
	s = sizeof(q1_stvert_t) * mdl.numverts;
	vtx = (q1_stvert_t *)malloc(s);
	if (!vtx) {
		printf("unable to alloc vertex array\n");
		return false;
	}
	memcpy(vtx, read_buffer, s);
	read_buffer += s;

	s = sizeof(q1_dtriangle_t) * mdl.numtris;
	tri = (q1_dtriangle_t *)malloc(s);
	if (!tri) {
		printf("unable to alloc triangle array\n");
		return false;
	}
	memcpy(tri, read_buffer, s);
	read_buffer += s;

	//frames
		//keep track of where the frame reading starts
	const char *frame_read_buffer = read_buffer;
		//alloc the frames
	frame = (q1_frame_t *)malloc(sizeof(q1_frame_t) * mdl.numframes);
	if (!frame) {
		printf("unable to alloc frame array\n");
		return false;
	}
		//read the frames
	num_poses = 0;
	for (i = 0; i < mdl.numframes; i++) {
		q1_aliasframetype_t frametype = *(q1_aliasframetype_t *)read_buffer;	//needed only for load
		read_buffer += sizeof(q1_aliasframetype_t);

		if (frametype == ALIAS_SINGLE) {
			q1_daliasframe_t *aliasframe = (q1_daliasframe_t *)read_buffer;
			read_buffer += sizeof(q1_daliasframe_t);
			//calc in the frame info
			frame[i].first_pose = num_poses;
			frame[i].num_poses = 1;
			num_poses += frame[i].num_poses;
			memcpy(&frame[i].bboxmin, &aliasframe->bboxmin, sizeof(aliasframe->bboxmin));
			memcpy(&frame[i].bboxmax, &aliasframe->bboxmax, sizeof(aliasframe->bboxmax));
			memcpy(&frame[i].name, &aliasframe->name, sizeof(aliasframe->name));
			frame[i].interval = 0;
			//skip over the vertexes
			read_buffer += sizeof(q1_trivertx_t) * mdl.numverts;
		} else {
			printf(" I FOUND A FRAME GROUP WITHIN THIS MODEL!\n");
			q1_daliasgroup_t *aliasgroup = (q1_daliasgroup_t *)read_buffer;
			read_buffer += sizeof(q1_daliasgroup_t);
			//calc in the frame info
			frame[i].first_pose = num_poses;
			frame[i].num_poses = aliasgroup->numframes;
			num_poses += frame[i].num_poses;
			memcpy(&frame[i].bboxmin, &aliasgroup->bboxmin, sizeof(aliasgroup->bboxmin));
			memcpy(&frame[i].bboxmax, &aliasgroup->bboxmax, sizeof(aliasgroup->bboxmax));
			//memcpy(&frame[i].name, &aliasgroup->name, sizeof(aliasgroup->name));
			frame[i].interval = *(float *)read_buffer;		//why do we only read the first interval and skip the rest?
			//skip over the intervals
			read_buffer += sizeof(float) * aliasgroup->numframes;
			//skip over the vertexes
			read_buffer += (sizeof(q1_daliasframe_t) + sizeof(q1_trivertx_t) * mdl.numverts) * aliasgroup->numframes;
		}
	}

	//poses
	s = sizeof(q1_trivertx_t) * mdl.numverts;
		//reset read buffer pointer
	read_buffer = frame_read_buffer;
		//alloc pose buffer
	pose = (q1_trivertx_t *)malloc(sizeof(q1_trivertx_t) * mdl.numverts * num_poses);
	if (!pose) {
		printf("unable to alloc pose array\n");
		return false;
	}
		//sift thru the frame data once again
	int cur_pose = 0;
	for (i = 0; i < mdl.numframes; i++) {
		q1_aliasframetype_t frametype = *(q1_aliasframetype_t *)read_buffer;	//needed only for load
		read_buffer += sizeof(q1_aliasframetype_t);

		if (frametype == ALIAS_SINGLE) {
			//skip alias frame entry
			read_buffer += sizeof(q1_daliasframe_t);
			//read the vertexes
			memcpy(&pose[mdl.numverts * cur_pose], read_buffer, s);
			read_buffer += s;
			cur_pose++;
		} else {
			q1_daliasgroup_t *aliasgroup = (q1_daliasgroup_t *)read_buffer;
			//skip alias group entry
			read_buffer += sizeof(q1_daliasgroup_t);
			//skip over the intervals
			read_buffer += sizeof(float) * aliasgroup->numframes;
			//read the vertexes
			for (j = 0; j < aliasgroup->numframes; j++) {
				//skip the alias frame
				read_buffer += sizeof(q1_daliasframe_t);
				//read the vertex data
				memcpy(&pose[mdl.numverts * cur_pose], read_buffer, s);
				read_buffer += s;
				cur_pose++;			//shouldn't this be here?
			}
		}
	}

	assert(cur_pose == num_poses);

		//alloc normal buffer
	normal = (vec3f *)malloc(sizeof(vec3f) * mdl.numverts * num_poses);
	if (!normal) {
		printf("unable to alloc normal array\n");
		return false;
	}

	for (i = 0; i < num_poses; i++) {
		q1_trivertx_t *poseverts = &pose[i * mdl.numverts];
		vec3f *posenorms = &normal[i * mdl.numverts];

		//clear all the normals of this frame
		//memset(posenorms, 0, sizeof(vec3f) * mdl.numverts);
		for (vec3f *v = posenorms; v < posenorms + mdl.numverts; ++v) {
			*v = vec3f();
		}

		for (int t = 0; t < mdl.numtris; t++) {
			//vertex[0].{x,y,z} = poseverts[pv[0]].v[{0,1,2}]
			vec3f vtx[3];
			for (j = 0; j < 3; j++) {
				if (tri[t].vertindex[j] < 0 || tri[t].vertindex[j] >= mdl.numverts) {
					printf("OOB tri vert index\n");
					return false;
				}
				int pv = tri[t].vertindex[j];
				vtx[j].x = (float)poseverts[pv].v[0];
				vtx[j].y = (float)poseverts[pv].v[1];
				vtx[j].z = (float)poseverts[pv].v[2];
			}

			//calc the face normal
			vec3f face_normal = vecUnitNormal(vtx[0], vtx[1], vtx[2]);
			//and add it to all touching vertex normals
			for (j = 0; j < 3; j++) {
				int pv = tri[t].vertindex[j];
				posenorms[pv] += face_normal;
			}
		}

		//normalize all the vectors we found
		for (int t = 0; t < mdl.numverts; t++) {
			posenorms[t] = vecUnit(posenorms[t]);
		}
	}

	return true;
}

bool Q1Model::LoadFromFile(const char *filename) {

	this->Unload();

	//first load the whole file into a binary buffer
	FILE *file = fopen(filename, "rb");
	if (!file) return false;

	if (fseek(file, 0, SEEK_END)) {
		fclose(file);
		return false;
	}

	long size = ftell(file);

	if (fseek(file, 0, SEEK_SET)) {
		fclose(file);
		return false;
	}

	char *buffer = (char *)malloc(size);
	if (!buffer) {
		fclose(file);
		return false;
	}

	if (!fread(buffer, size, 1, file)) {	//read no data
		free(buffer);
		fclose(file);
		return false;
	}

	fclose(file);

	bool loaded = LoadFromBuffer(buffer);

	//then shutdown the binary buffer
	free(buffer);

	return loaded;
}

int Q1Model::FindFrameIndex(const char *frame_name) {
	if (frame) {
		for (int i = 0; i < mdl.numframes; i++) {
			if (!strncmp(frame[i].name, frame_name, sizeof(frame[i].name))) {
				return i;
			}
		}
	}
	return -1;
}

int Q1Model::GetSkin(int index) {
	if (index >= 0 && index < mdl.numskins) {
		return skin[index<<2];
	}
	return 0;
}

void Q1Model::Display(int frame_index) {
	if (frame_index < 0 || frame_index >= mdl.numframes) return;

	q1_frame_t *f = &frame[frame_index];
	q1_trivertx_t *poseverts = &pose[f->first_pose * mdl.numverts];
	vec3f *posenorms = &normal[f->first_pose * mdl.numverts];

	//set the appropriate texture

//	swFrontFace(GL_CW);
	swPushMatrix();
	swTranslatef (mdl.scale_origin[0], mdl.scale_origin[1], mdl.scale_origin[2]);
	swScalef (mdl.scale[0], mdl.scale[1], mdl.scale[2]);

	//and go at it...
	for (int i = 0; i < mdl.numtris; i++) {
		swBegin(SW_POLYGON);
		for (int j = 0; j < 3; j++) {
//already tested
//			if (tri[i].vertindex[j] < 0 || tri[i].vertindex[j] >= mdl.numverts) {
//				printf("OOB tri vert index\n");
//				glVertex3f(0,0,0);
//			} else {

			int pv = tri[i].vertindex[j];
			float st[2];
			st[0] = (float)vtx[pv].s;
			st[1] = (float)vtx[pv].t;
			if (!tri[i].facesfront && vtx[pv].onseam) st[0] += mdl.skinwidth / 2;//on back side
			st[0] = (st[0] + 0.5f) / mdl.skinwidth;
			st[1] = (st[1] + 0.5f) / mdl.skinheight;
			swTexCoord2f(st[0], st[1]);
			swNormal3f(VEC3ELEM(posenorms[pv]));
			swVertex3f((float)	poseverts[pv].v[0],
						(float) poseverts[pv].v[1],
						(float) poseverts[pv].v[2]);
		}
		swEnd();
	}

	swPopMatrix();
//	swFrontFace(GL_CCW);
}

void Q1Model::DisplayInterpolate(int start_index, int end_index, float fraction) {
	if (start_index < 0 || start_index >= mdl.numframes) return;
	if (end_index < 0 || end_index >= mdl.numframes) return;

	q1_frame_t *f0 = frame + start_index;
	q1_frame_t *f1 = frame + end_index;

	q1_trivertx_t *poseverts0 = pose + (f0->first_pose * mdl.numverts);
	q1_trivertx_t *poseverts1 = pose + (f1->first_pose * mdl.numverts);

	vec3f *posenorms0 = normal + (f0->first_pose * mdl.numverts);
	vec3f *posenorms1 = normal + (f1->first_pose * mdl.numverts);

	//set the appropriate texture

//	swFrontFace(GL_CW);
	swPushMatrix();
	swTranslatef (mdl.scale_origin[0], mdl.scale_origin[1], mdl.scale_origin[2]);
	swScalef (mdl.scale[0], mdl.scale[1], mdl.scale[2]);

	//calc on the fly yo!
	int pv, i, j;
	float st[2];
	vec3f v, *n0, *n1;
	byte *v0, *v1;

	//and go at it...
	for (i = 0; i < mdl.numtris; i++) {
		swBegin(SW_POLYGON);
		for (j = 0; j < 3; j++) {
			pv = tri[i].vertindex[j];

			st[0] = (float)vtx[pv].s;
			st[1] = (float)vtx[pv].t;
			if (!tri[i].facesfront && vtx[pv].onseam) st[0] += mdl.skinwidth / 2;//on back side
			st[0] = (st[0] + 0.5f) / mdl.skinwidth;
			st[1] = (st[1] + 0.5f) / mdl.skinheight;
			swTexCoord2f(st[0], st[1]);

			n0 = posenorms0 + pv;
			n1 = posenorms1 + pv;		//negative the normals
			v.x = -( (n1->x - n0->x) * fraction + n0->x);
			v.y = -( (n1->y - n0->y) * fraction + n0->y);
			v.z = -( (n1->z - n0->z) * fraction + n0->z);
			swNormal3f(VEC3ELEM(v));

			v0 = poseverts0[pv].v;
			v1 = poseverts1[pv].v;
			v.x = (v1[0] - v0[0]) * fraction + (float)v0[0];
			v.y = (v1[1] - v0[1]) * fraction + (float)v0[1];
			v.z = (v1[2] - v0[2]) * fraction + (float)v0[2];
			swVertex3f(VEC3ELEM(v));
		}
		swEnd();
	}

	swPopMatrix();
//	swFrontFace(GL_CCW);
}

void Q1Model::DisplayInterpolateShader(
	int start_index,
	int end_index,
	float fraction,
	void (*shaderCallback)(const vec3f &v, const vec3f &t, const vec3f &n))
{
	if (start_index < 0 || start_index >= mdl.numframes) return;
	if (end_index < 0 || end_index >= mdl.numframes) return;

	q1_frame_t *f0 = frame + start_index;
	q1_frame_t *f1 = frame + end_index;

	q1_trivertx_t *poseverts0 = pose + (f0->first_pose * mdl.numverts);
	q1_trivertx_t *poseverts1 = pose + (f1->first_pose * mdl.numverts);

	vec3f *posenorms0 = normal + (f0->first_pose * mdl.numverts);
	vec3f *posenorms1 = normal + (f1->first_pose * mdl.numverts);

	//set the appropriate texture

//	swFrontFace(GL_CW);
	swPushMatrix();
	swTranslatef (mdl.scale_origin[0], mdl.scale_origin[1], mdl.scale_origin[2]);
	swScalef (mdl.scale[0], mdl.scale[1], mdl.scale[2]);

	//calc on the fly yo!
	int pv, i, j;
	vec3f *n0, *n1;
	byte *v0, *v1;
	vec3f t, n, v;

	//and go at it...
	for (i = 0; i < mdl.numtris; i++) {
		swBegin(SW_POLYGON);
		for (j = 0; j < 3; j++) {
			pv = tri[i].vertindex[j];

			t.x = (float)vtx[pv].s;
			t.y = (float)vtx[pv].t;
			if (!tri[i].facesfront && vtx[pv].onseam) t.x += mdl.skinwidth / 2;//on back side
			t.x = (t.x + 0.5f) / mdl.skinwidth;
			t.y = (t.y + 0.5f) / mdl.skinheight;

			n0 = posenorms0 + pv;
			n1 = posenorms1 + pv;
			n.x = (n1->x - n0->x) * fraction + n0->x;
			n.y = (n1->y - n0->y) * fraction + n0->y;
			n.z = (n1->z - n0->z) * fraction + n0->z;

			v0 = poseverts0[pv].v;
			v1 = poseverts1[pv].v;
			v.x = (v1[0] - v0[0]) * fraction + (float)v0[0];
			v.y = (v1[1] - v0[1]) * fraction + (float)v0[1];
			v.z = (v1[2] - v0[2]) * fraction + (float)v0[2];

			shaderCallback(v,t,n);
		}
		swEnd();
	}

	swPopMatrix();
//	swFrontFace(GL_CCW);
}

void Q1Model::DisplayInterpolateColorNormal(int start_index, int end_index, float fraction) {
	if (start_index < 0 || start_index >= mdl.numframes) return;
	if (end_index < 0 || end_index >= mdl.numframes) return;

	q1_frame_t *f0 = frame + start_index;
	q1_frame_t *f1 = frame + end_index;

	q1_trivertx_t *poseverts0 = pose + (f0->first_pose * mdl.numverts);
	q1_trivertx_t *poseverts1 = pose + (f1->first_pose * mdl.numverts);

	vec3f *posenorms0 = normal + (f0->first_pose * mdl.numverts);
	vec3f *posenorms1 = normal + (f1->first_pose * mdl.numverts);

	//set the appropriate texture

//	swFrontFace(GL_CW);
	swPushMatrix();
	swTranslatef (mdl.scale_origin[0], mdl.scale_origin[1], mdl.scale_origin[2]);
	swScalef (mdl.scale[0], mdl.scale[1], mdl.scale[2]);

	//calc on the fly yo!
	int pv, i, j;
	vec3f v, *n0, *n1;
	byte *v0, *v1;

	//and go at it...
	for (i = 0; i < mdl.numtris; i++) {
		swBegin(SW_POLYGON);
		for (j = 0; j < 3; j++) {
			pv = tri[i].vertindex[j];

			n0 = posenorms0 + pv;
			n1 = posenorms1 + pv;
			v.x = (n1->x - n0->x) * fraction + n0->x;
			v.y = (n1->y - n0->y) * fraction + n0->y;
			v.z = (n1->z - n0->z) * fraction + n0->z;
			swNormal3f(VEC3ELEM(v));
			v = v * 0.5f + vec3f(0.5f,0.5f,0.5f);
			swColor3f(VEC3ELEM(v));

			v0 = poseverts0[pv].v;
			v1 = poseverts1[pv].v;
			v.x = (v1[0] - v0[0]) * fraction + (float)v0[0];
			v.y = (v1[1] - v0[1]) * fraction + (float)v0[1];
			v.z = (v1[2] - v0[2]) * fraction + (float)v0[2];
			swVertex3f(VEC3ELEM(v));
		}
		swEnd();
	}

	swPopMatrix();
//	swFrontFace(GL_CCW);
}

void Q1Model::DisplayInterpolateLines(int start_index, int end_index, float fraction) {
	if (start_index < 0 || start_index >= mdl.numframes) return;
	if (end_index < 0 || end_index >= mdl.numframes) return;

	q1_frame_t *f0 = frame + start_index;
	q1_frame_t *f1 = frame + end_index;

	q1_trivertx_t *poseverts0 = pose + (f0->first_pose * mdl.numverts);
	q1_trivertx_t *poseverts1 = pose + (f1->first_pose * mdl.numverts);

	vec3f *posenorms0 = normal + (f0->first_pose * mdl.numverts);
	vec3f *posenorms1 = normal + (f1->first_pose * mdl.numverts);

	//set the appropriate texture

//	swFrontFace(GL_CW);
	swPushMatrix();
	swTranslatef (mdl.scale_origin[0], mdl.scale_origin[1], mdl.scale_origin[2]);
	swScalef (mdl.scale[0], mdl.scale[1], mdl.scale[2]);

	//calc on the fly yo!
	int pv, i, j;
	float st[2];
	vec3f v, n, *n0, *n1;
	byte *v0, *v1;

	//and go at it...
	for (i = 0; i < mdl.numtris; i++) {
		swBegin(SW_LINES);
		for (j = 0; j < 6; j++) {
			pv = tri[i].vertindex[j%3];

			st[0] = (float)vtx[pv].s;
			st[1] = (float)vtx[pv].t;
			if (!tri[i].facesfront && vtx[pv].onseam) st[0] += mdl.skinwidth / 2;//on back side
			st[0] = (st[0] + 0.5f) / mdl.skinwidth;
			st[1] = (st[1] + 0.5f) / mdl.skinheight;
			swTexCoord2f(st[0], st[1]);

			n0 = posenorms0 + pv;
			n1 = posenorms1 + pv;
			n.x = (n1->x - n0->x) * fraction + n0->x;
			n.y = (n1->y - n0->y) * fraction + n0->y;
			n.z = (n1->z - n0->z) * fraction + n0->z;
			swNormal3f(VEC3ELEM(n));

			v0 = poseverts0[pv].v;
			v1 = poseverts1[pv].v;
			v.x = (v1[0] - v0[0]) * fraction + (float)v0[0];
			v.y = (v1[1] - v0[1]) * fraction + (float)v0[1];
			v.z = (v1[2] - v0[2]) * fraction + (float)v0[2];
			swVertex3fv( (v - n ).fp() );
		}
		swEnd();
	}

	swPopMatrix();
//	swFrontFace(GL_CCW);
}

void Q1Model::DisplayTexcoords() {
	for (int i = 0; i < mdl.numtris; i++) {
		swBegin(SW_LINES);

		for (int j = 0; j < 6; j++) {
			int pv = tri[i].vertindex[j%3];
			float st[2];

			st[0] = (float)vtx[pv].s;
			st[1] = (float)vtx[pv].t;

			if (!tri[i].facesfront && vtx[pv].onseam) st[0] += mdl.skinwidth / 2;//on back side
			st[0] = (st[0] + 0.5f) / mdl.skinwidth;
			st[1] = (st[1] + 0.5f) / mdl.skinheight;
			swVertex3f(st[0], st[1], 1.f);
		}
		swEnd();
	}
}
