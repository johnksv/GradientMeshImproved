#ifndef MESHHANDLER_H
#define MESHHANDLER_H

// OpenMesh requirement
#define _USE_MATH_DEFINES
#include <cmath>

#include "subdivMesh/mesh.h"
#include "customMeshing/openmeshext.h"
#include <QOpenGLFunctions_1_0>
#include <QColor>

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

typedef OpenMesh::PolyMesh_ArrayKernelT<OpenMeshExt::CustomTraits> OpnMesh;
typedef OpnMesh::VertexHandle vertexHandle;

class MeshHandler
{
public:
    MeshHandler();
    ~MeshHandler();
    /*! Draws the current mesh to an OpenGL canvas.
     * \param context the OpenGL context that is being used to draw
     */
    void drawGLMesh(QOpenGLFunctions_1_0 *context);
    /*! Get verticies from the current guiMesh object.
     * \return a vector of points. Each element contains x, y, z coordiantes respectively
     *
     */
    vector<QVector4D> vertices();
    int addVertex(const QPointF& position, const QColor vertexColor = QColor(0,0,0));
    void removeVertex(int idx);
    QVector3D vertexPoint(int idx);
    void setVertexPoint(int idx, const QPointF& position);
    QVector3D vertexColor(int idx);
    void setVertexColor(int idx, QColor vertexColor);
    uint vertexValence(int idx);
    bool isBoundaryVertex(int idx);

    bool isBoundaryEdge(int startIdx, int endIdx);
    void setConstraints(int halfedgeFromVertIdx, int halfedgeToVertIdx, QVector2D constraints);
    QVector2D constraints(int halfedgeFromVertIdx, int halfedgeToVertIdx);
    //First and last element is expected to be actual control point idx
    void deleteDiscontinuedFace(vector<int> &vertexHandlersIdx);

    //Returns  startVert.idx(),endVert.idx(), e_it->idx(), 0
    vector<QVector4D> edges();
	void insertVertexOnEdge(int edgeIdx, int vertexIdx);

    bool makeFace(vector<int> &vertexHandlersIdx, bool faceInsideFace = false);
    size_t numberOfFaces();
    bool vertsOnSameFace(int vertIdx1, int vertIdx2);

    void clearAll();


    bool saveGuiMeshOff(QString);
    bool importGuiMesh(QString);

    void prepareGuiMeshForSubd();


private:
    //Each elemnt in this vector correspond to the same index in item_points (canvas.h)
    vector<OpenMesh::PolyMesh_ArrayKernelT<OpenMeshExt::CustomTraits>::VertexHandle> vertexHandlers_;
    vector<OpenMesh::PolyMesh_ArrayKernelT<OpenMeshExt::CustomTraits>::FaceHandle> faceHandlers_;
    // mesh for rendering gradient mesh using subdivision:
    subdivMesh::Mesh* subdMesh;
    // mesh for editing gui:
    OpenMesh::PolyMesh_ArrayKernelT<OpenMeshExt::CustomTraits> guiMesh;


     /* Check if new face has same orientation as first face (CW or CCW), if NOT, true is returned (which means an extra loop in makeFace is required).
     */
    bool faceOrientation(vector<vertexHandle> &orginalvHandlersFace, OpnMesh::FaceHandle &newFace, vector<vertexHandle> &vHandlersFace);

    void subdivide(signed int steps = 3);
};

} // end of namespace GUILogic

#endif // MESHHANDLER_H
