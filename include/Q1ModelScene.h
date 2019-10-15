#pragma once

#include "Q1Model.h"
#include "ObjFile.h"

#include "Scene.h"
#include "vec.h"		//texture matrix

#include <time.h>
#include <memory>

struct Q1ModelScene : public Scene {
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
	int modelMode, renderMode, textureMode, borderMode[3], filterMode;
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
