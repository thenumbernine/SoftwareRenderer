#pragma once

#include "Q1Model.h"
#include "ObjFile.h"

#include "Scene.h"
#include "vec.h"		//texture matrix

#include <time.h>
#include <memory>


struct Q1ModelScene : public Scene {
	enum ModelEnum {
		MODEL_CUBE,
		MODEL_CONE,
		MODEL_QUAKE,
		MODEL_TORUS,
		MODEL_SPHERE,
	};

	enum RenderEnum {
		RENDER_NONE,
		RENDER_LINEAR,
		RENDER_CYL,
		RENDER_SPH,
		RENDER_CEL,
	};

	enum TextureEnum {
		TEX_NONE,
		TEX_DEFAULT,
		TEX_CHECKER,
		TEX_CONTOUR,
		TEX_CEL,
		TEX_RGB,
		TEX_NORMALIZE,
	};

	enum BorderEnum {
		BORDER_REP,
		BORDER_U_REP,
		BORDER_CLMP,
		BORDER_U_CLMP,
		BORDER_V_REP,
		BORDER_V_CLMP,
		BORDER_W_REP,
		BORDER_W_CLMP,
		BORDER_V_CLMP2,
	};

	enum FillEnum {
		FILL_NEAR,
		FILL_LIN,
	};
	
	Q1ModelScene();
	virtual ~Q1ModelScene();
	
	virtual void update();

	virtual void render(const Viewport *viewport, const View *view);
	virtual void updateGUI();

	void setModelMode(int id) { modelMode = id; }
	void setRenderMode(int id) { renderMode = id; }
	void setTextureMode(int id) { textureMode = id; }
	void setBorderMode(int index, int id) { borderMode[index] = id; }
	void setFilterMode(int id) { filterMode = id; }
	void setTexMat(int index, float val) {  texMat.fp()[index] = val; }

//private:
	int modelMode;
	int renderMode;
	int textureMode;
	int borderMode[3];
	int filterMode;
private:	
	mat44f texMat;

	std::shared_ptr<Q1Model> quakeMdl;
	std::shared_ptr<ObjFile> cubeMdl, coneMdl, torusMdl, sphereMdl;

	unsigned int rgbTex;
	unsigned int celTex;
	unsigned int contourTex;
	unsigned int checkerTex;

	clock_t time;
	int frames;
};
