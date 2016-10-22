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
     * \return Vertices in first vector. Points and color of vert in second vector.
     * Element at [i][0] is x,y,z = point, w = vertIdx. Element at [i][1] is x,y,z = color(rgb)
     *
     */
    vector<vector<QVector4D> > vertices();
    int addVertex(const QPointF& position, const QColor vertexColor = QColor(0,0,0));
    void deleteVertex(int idx);
    QVector3D vertexPoint(int idx);
    void setVertexPoint(int idx, const QPointF& position);
    QVector3D vertexColor(int idx);
    void setVertexColor(int idx, QColor vertexColor);
    uint vertexValence(int idx);
    bool isBoundaryVertex(int idx);
    bool isValidVertex(int idx);

    bool isBoundaryEdge(int startIdx, int endIdx);
    void setConstraints(int halfedgeFromVertIdx, int halfedgeToVertIdx, subdivMesh::Point_3D constraints);
    void setConstraints(int halfedgeFromVertIdx, int halfedgeToVertIdx, QVector2D constraints);
    QVector2D constraints(int halfedgeFromVertIdx, int halfedgeToVertIdx);
    vector<QVector2D> constraints(int edgeIdx);
    //First and last element is expected to be actual control point idx
    void deleteDiscontinuedFace(vector<int> &vertexHandlersIdx);

    //Returns  startVert.idx(),endVert.idx(), e_it->idx(), 0
    vector<QVector4D> edges();
    bool collapseEdge(int startVertIdx, int endVertIdx);
    //Returns the idx of the new vertex added
    int insertVertexOnEdge(int edgeStartVertIdx, int edgeEndVertIdx, const QPointF& position, const QColor &color = QColor(0,0,0));

    //TODO move face functions to openmesh.
    bool addFaceClosed(vector<int> &vertexHandlersIdx);
    bool makeFace(vector<int> &vertexHandlersIdx, bool faceInsideFace = false, bool sameStartAndEnd = false);
    bool addFaceWIthSameStartAndEnd(vector<int> &vertexHandlersIdx, bool faceInsideFace = false);
    size_t numberOfFaces();
    bool vertsOnSameFace(int vertIdx1, int vertIdx2);
    vector<vector<int> > facesIdx();
    void deleteFace(const int &faceIdx, bool delete_isolated_vertices = true);
    bool isBoundaryFace(const int &idx) const;

    void clearAll();

    bool importGuiMesh(QString, bool draw = true);

    void garbageCollectOpenMesh();

    //TODO: Rename function.
    void prepareGuiMeshForSubd(bool saveFileOFF = false, QString location = QString(""));

    MeshHandler *oneStepSubdMesh();

    static int subdivisionSteps;

private:
    // mesh for rendering gradient mesh using subdivision:
    subdivMesh::Mesh* subdMesh;
    subdivMesh::Mesh* oneStepSubdMesh_ = nullptr;
    // mesh for editing gui:
    OpenMesh::PolyMesh_ArrayKernelT<OpenMeshExt::CustomTraits> guiMesh;


     /* Check if new face has same orientation as first face (CW or CCW), if NOT, true is returned (which means an extra loop in makeFace is required).
     */
    bool faceOrientation(vector<vertexHandle> &orginalvHandlersFace, OpnMesh::FaceHandle &newFace, vector<vertexHandle> &vHandlersFace);

    void subdivide();
};

} // end of namespace GUILogic

#endif // MESHHANDLER_H
