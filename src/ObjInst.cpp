#include "ObjInst.h"

#include "sw.h"

void Material::setup(int face) {
}

static int objIndex = 0;

void ObjInst::reset() {
	model = NULL;

	//rendering
	mat.diff = quatf(1,1,1,1);
 
	//transformations
	basis.t = vec3f(0,0,0);
	basis.r = quatf();
	scale = vec3f(1,1,1);

	//title
	title = std::string("obj ") + std::to_string(objIndex++);
}

void ObjInst::unload() {
	reset();
}

void ObjInst::render() {
	if (!model) return;
	model->render();
}

bool ObjInst::setModel(ObjFile *model) {

	//set the new model
	this->model = model;

	//if we're null then dont calc visibility
	if (!model) return true;

	title = model->getTitle();

	return true;
}
