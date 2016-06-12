#ifndef MESHHANDLER_H
#define MESHHANDLER_H

// OpenMesh requirement
#define _USE_MATH_DEFINES
#include <cmath>

#include "subdivMesh/mesh.h"
#include "customMeshing/openmeshext.h"
#include <QOpenGLFunctions_1_0>

#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

/*! Namespace for handling logic related to user input
 * and underlying data of the application.
 */
namespace GUILogic {

/*! \brief GUILogic.MeshHandler.
 *         A class for handling interactions
 *         between GUI and underlying mesh objects.
 *
 *  Usage: GUI classes/controller classes.
 *  Dependencies: subdivMesh namespace and OpenMesh library.
 *
 * Version: 0 (class in development)
 */

class MeshHandler
{
public:
    MeshHandler();
    ~MeshHandler();
    /*! Draws the current mesh to an OpenGL canvas.
     * \param context the OpenGL context that is being used to draw
     */
    void drawGLMesh(QOpenGLFunctions_1_0 *context);

private:
    void subdivide(signed int steps = 3);

    // mesh for rendering gradient mesh using subdivision:
    subdivMesh::Mesh* subdMesh;
    // mesh for editing gui:
//    OpenMesh::PolyMesh_ArrayKernelT<OpenMeshExt::CustomTraits> guiMesh;

    // TEMPORARY TEST VARIABLE
    const char* const TEMPFILEPATH = "../../testMesh-removeASAP.off";

    // TEMPORARY: example method for setting up an OpenMesh mesh.
    void createTwoQuads();
};

} // end of namespace GUILogic

#endif // MESHHANDLER_H
