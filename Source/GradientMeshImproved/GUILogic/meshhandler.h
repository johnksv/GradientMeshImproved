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
    void setVertexPoint(int idx, const QPointF& position);
    QVector3D vertexColor(int idx);
    void setVertexColor(int idx, QColor vertexColor);
    double vertexWeight(int idx);
    bool setVertexWeight(int idx, double vertexWeight);
    uint vertexValence(int idx);

    //Returns  startVert.idx(),endVert.idx(), e_it->idx(), 0
    vector<QVector4D> edges();
    int addEdge(int startVertexIdx, int endVertexIdx);
    void removeEdge(int idx);
	void insertVertexOnEdge(int edgeIdx, int vertexIdx);

    bool makeFace(vector<int> &vertexHandlersIdx);


    bool saveGuiMeshOff(QString);
    bool importGuiMesh(QString);

    void prepareGuiMeshForSubd();


private:
    //Each elemnt in this vector correspond to the same index in item_points (canvas.h)
    vector<OpenMesh::PolyMesh_ArrayKernelT<OpenMeshExt::CustomTraits>::VertexHandle> vertexHandlers;
    vector<OpenMesh::PolyMesh_ArrayKernelT<OpenMeshExt::CustomTraits>::EdgeHandle> edgeHandlers;
    vector<OpenMesh::PolyMesh_ArrayKernelT<OpenMeshExt::CustomTraits>::FaceHandle> faceHandlers;
    // mesh for rendering gradient mesh using subdivision:
    subdivMesh::Mesh* subdMesh;
    // mesh for editing gui:
    OpenMesh::PolyMesh_ArrayKernelT<OpenMeshExt::CustomTraits> guiMesh;

    /*	Returns the index (in "vertexHandlers") to the vertexhandler with idx == idxToFind
    *	If no match is found, -1 is returned (which will lead to an error as it should)
    *
    */
    int findVertexHandler(int idxToFind);
    int findEdgeHandler(int idxToFind);

    /*METHODS*/
    void subdivide(signed int steps = 3);


    // TEMPORARY TEST VARIABLES
    const char* const TEMPFILEPATH = "../../testMesh-removeASAP.off";
    const char* const TEMPFILEPATH2 = "../../testMesh-removeASAP2.off";
    const string TESTMESH = "OFF\n7 1 0\n362.583 137.229 1 72.4768 19.2033 73.9236 2 1 6 85.417 16.771 1.41699 83.771 0 0 0\n614.075 57 1 51.4831 77.6974 62.9024 2 0 2 -71.075 -4 72.925 -1 0 0 0\n816.259 137.229 1 93.9804 -17.8959 88.7035 2 1 3 -80.259 23.771 -15.259 83.771 0 0 0\n1001 310.095 1 85.3188 -83.5835 78.1661 2 2 4 -31 -75.095 34 100.905 0 0 0\n816.259 616 1 89.3447 -52.4927 -10.7167 2 3 5 171.741 5 -120.259 -134 0 0 0\n362.583 616 1 31.568 76.7502 -109.955 2 4 6 139.417 -154 -160.583 5 0 0 0\n199 310.095 1 56.3779 92.0092 -57.8666 2 5 0 -17 79.905 7 -61.095 0 0 0\n\n7 0 1 2 3 4 5 6\n";

    // TEMPORARY: demo method demonstrating how to set up a subdivision mesh
    void setUpSubdMeshStream();
    void setUpSubdMeshFile();
};

} // end of namespace GUILogic

#endif // MESHHANDLER_H
