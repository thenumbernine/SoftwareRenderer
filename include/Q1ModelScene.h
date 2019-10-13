#pragma once

#include <time.h>
#include "Q1Model.h"
#include "ObjFile.h"

#include "Scene.h"
#include "vec.h"		//texture matrix

class Q1ModelScene : public Scene {
public:
	virtual bool init();
	virtual void shutdown();
	virtual void update();

	virtual void render(const Viewport *viewport, const View *view);

	void setModelMode(int id) { modelMode = id; }
	void setRenderMode(int id) { renderMode = id; }
	void setTextureMode(int id) { textureMode = id; }
	void setBorderMode(int index, int id) { borderMode[index] = id; }
	void setFilterMode(int id) { filterMode = id; }
	void setTexMat(int index, float val) {  texMat.fp()[index] = val; }

	Q1ModelScene() {clear();}
	~Q1ModelScene() {shutdown();}

private:

	int modelMode, renderMode, textureMode, borderMode[3], filterMode;
	mat44f texMat;

	void clear();

	Q1Model *quakeMdl;
	ObjFile *cubeMdl, *coneMdl, *torusMdl, *sphereMdl;

	unsigned int rgbTex;
	unsigned int celTex;
	unsigned int contourTex;
	unsigned int checkerTex;

	unsigned long controlThreadID;

	clock_t time;
	int frames;
};
