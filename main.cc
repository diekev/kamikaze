#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>

#include <iostream>

#define DWREAL_IS_DOUBLE 0
#include <openvdb/openvdb.h>

#include "render/GLSLShader.h"

#include "objects/levelset.h"
#include "objects/volume.h"
#include "render/scene.h"
#include "render/viewer.h"
#include "util/util_openvdb.h"
#include "util/utils.h"

namespace {

Viewer *viewer = nullptr;

bool initViewer(const char *filename, std::ostream &os)
{
	using namespace openvdb;

	if (viewer == nullptr) {
		viewer = new Viewer();
		viewer->init();
	}

	Scene *scene = new Scene;

	openvdb::initialize();
	openvdb::io::File file(filename);

	if (file.open()) {
		FloatGrid::Ptr grid;

		if (file.hasGrid(Name("Density"))) {
			grid = gridPtrCast<FloatGrid>(file.readGrid(Name("Density")));
		}
		else if (file.hasGrid(Name("density"))) {
			grid = gridPtrCast<FloatGrid>(file.readGrid(Name("density")));
		}
		else {
			GridPtrVecPtr grids = file.getGrids();
			grid = gridPtrCast<FloatGrid>((*grids)[0]);
//			os << "No density grid found in file: \'" << filename << "\'!\n";
//			return false;
		}

		auto meta_map = file.getMetadata();

		file.close();

		if ((*meta_map)["creator"]) {
			auto creator = (*meta_map)["creator"]->str();

			/* If the grid comes from Blender (Z-up), rotate it so it is Y-up */
			if (creator == "Blender/OpenVDBWriter") {
				Timer("Transform Blender Grid");
				grid = transform_grid(*grid, Vec3s(-M_PI_2, 0.0f, 0.0f),
				                      Vec3s(1.0f), Vec3s(0.0f), Vec3s(0.0f));
			}
		}

		if (grid->getGridClass() == GRID_LEVEL_SET) {
			LevelSet *ls = new LevelSet(grid);
			scene->add_level_set(ls);
		}
		else {
			Volume *volume = new Volume(grid);
			scene->add_volume(volume);
		}

		viewer->setScene(scene);

		return true;
	}

	os << "Unable to open file \'" << filename << "\'\n";
	return false;
}

void OnShutDownCB()
{
	delete viewer;
}

void OnRenderCB()
{
	if (viewer) {
		viewer->render();
	}
}

void OnResizeCB(int w, int h)
{
	if (viewer) {
		viewer->resize(w, h);
	}
}

void OnMouseDownCB(int button, int s, int x, int y)
{
	if (viewer) {
		viewer->mouseDownEvent(button, s, x, y);
	}
}

void OnMouseMoveCB(int x, int y)
{
	if (viewer) {
		viewer->mouseMoveEvent(x, y);
	}
}

void OnKeyEventCB(unsigned char key, int x, int y)
{
	if (viewer) {
		viewer->keyboardEvent(key, x, y);
	}
}

}  /* namespace */

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitContextProfile(GLUT_FORWARD_COMPATIBLE);

	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Hi no Kirin");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	std::ostream &os = std::cerr;

	if (GLEW_OK != err) {
		os << "Error: " << glewGetErrorString(err) << '\n';
		return 1;
	}

	if (!initViewer(argv[1], os)) {
		OnShutDownCB();
		return 1;
	}

	glutCloseFunc(OnShutDownCB);
	glutDisplayFunc(OnRenderCB);
	glutReshapeFunc(OnResizeCB);
	glutMouseFunc(OnMouseDownCB);
	glutMotionFunc(OnMouseMoveCB);
	glutKeyboardFunc(OnKeyEventCB);
	glutMainLoop();

	return 0;
}
