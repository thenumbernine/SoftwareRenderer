#pragma once

#include <cstdint>
#include "vec.h"

typedef enum {
	ST_SYNC=0,
	ST_RAND
} q1_synctype_t;

typedef struct {
	int32_t			ident;
	int32_t			version;
	float			scale[3];
	float			scale_origin[3];
	float			boundingradius;
	float			eyeposition[3];
	int32_t			numskins;
	int32_t			skinwidth;
	int32_t			skinheight;
	int32_t			numverts;
	int32_t			numtris;
	int32_t			numframes;
	q1_synctype_t	synctype;
	int32_t			flags;
	float			size;
} q1_mdl_t;

typedef enum {
	ALIAS_SKIN_SINGLE=0,
	ALIAS_SKIN_GROUP
} q1_aliasskintype_t;

typedef struct {
	int32_t	onseam;
	int32_t	s;
	int32_t	t;
} q1_stvert_t;

typedef struct {
	int32_t	facesfront;
	int32_t	vertindex[3];
} q1_dtriangle_t;

typedef struct {
	uint8_t	v[3];
	uint8_t	lightnormalindex;
} q1_trivertx_t;

typedef enum {
	ALIAS_SINGLE=0,
	ALIAS_GROUP
} q1_aliasframetype_t;

typedef struct {
	q1_trivertx_t	bboxmin;	// lightnormal isn't used
	q1_trivertx_t	bboxmax;	// lightnormal isn't used
	int8_t		name[16];	// frame name from grabbing
} q1_daliasframe_t;

typedef struct {
	int32_t			numframes;
	q1_trivertx_t	bboxmin;	// lightnormal isn't used
	q1_trivertx_t	bboxmax;	// lightnormal isn't used
} q1_daliasgroup_t;

#define	MAX_SKINS	32

//my creation - merge of alias frame and group
typedef struct {
	int32_t			first_pose;
	int32_t			num_poses;
	q1_trivertx_t	bboxmin;
	q1_trivertx_t	bboxmax;
	char			name[16];
	float			interval;
} q1_frame_t;


struct Q1Model {
	Q1Model(const char* filename);
	~Q1Model();
	
	void LoadFromBuffer(const char *read_buffer);
	void LoadFromFile(const char *filename);

	int FindFrameIndex(const char *frame_name);
	int GetSkin(int index);
	void Display(int frame_index);
	void DisplayInterpolate(int start_index, int end_index, float fraction);
	void DisplayInterpolateLines(int start_index, int end_index, float fraction);
	void DisplayInterpolateColorNormal(int start_index, int end_index, float fraction);

	void DisplayInterpolateShader(
		int start_index,
		int end_index,
		float fraction,
		void (*shaderCallback)(
			const vec3f &v, const vec3f &t, const vec3f &n));

	void DisplayTexcoords();

	const q1_mdl_t *getMdl() { return &mdl; }

	//temporarily public
	uint32_t	*skin;

private:
	q1_mdl_t		mdl;		//header
//	Texture			*skin;		//[mdl.numskins]
	q1_stvert_t		*vtx;		//[mdl.numverts]
	q1_dtriangle_t	*tri;		//[mdl.numtris]
	q1_frame_t		*frame;		//[mdl.numframes]
	int32_t			num_poses;	//calc'd from frame data
	q1_trivertx_t	*pose;		//[num_poses * mdl.numverts]

	//my addition
	vec3f			*normal;	//[num_poses * mdl.numverts]
};
