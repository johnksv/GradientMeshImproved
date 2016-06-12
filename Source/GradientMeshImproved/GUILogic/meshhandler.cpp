#include "GUILogic/meshhandler.h"
#include <QDebug>

//#include <OpenMesh/Core/IO/MeshIO.hh>

using namespace GUILogic;

typedef subdivMesh::Mesh Mesh;
//typedef OpenMesh::PolyMesh_ArrayKernelT<OpenMeshExt::CustomTraits> OpnMesh;
//typedef OpnMesh::VertexHandle vertexPntr;

MeshHandler::MeshHandler()
{
    subdMesh = new Mesh();
    subdMesh->loadV3(TEMPFILEPATH); // TEMPORARY TEST CASE. REMOVE ASAP
    subdMesh->build(); // must be called after load
    subdivide();

    // test OpenMesh lib
//    createTwoQuads();
}

MeshHandler::~MeshHandler()
{
    delete subdMesh;
}

void MeshHandler::drawGLMesh(QOpenGLFunctions_1_0* context)
{
    subdMesh->draw(context);
}

/********************************************
 * HELPER METHODS
********************************************/

// Subdivide mesh for rendering
// using method of Lieng et al.
void MeshHandler::subdivide(signed int steps)
{
    if(steps==0) return; // not in function domain

    Mesh *currentMsh = new Mesh();
    Mesh *nextMesh;

    // perform the first step of subdivision (special rules - Lieng et al.)
    subdMesh->LinearTernarySubdiv(currentMsh);
    delete subdMesh; // delete old mesh from heap

    // subdivide steps-1 of CC-subdivision
    for(int i = 0; i < steps-1; i++) {
        nextMesh = new Mesh();
        currentMsh->CatmullClarkColour(nextMesh);

        // delete old mesh from heap and swap
        delete currentMsh;
        currentMsh = nextMesh;
    }

    // set the final mesh
    subdMesh = currentMsh;
}

void MeshHandler::createTwoQuads()
{
    // create vertices: (quad 1)
/*    vertexPntr v0 = guiMesh.add_vertex(OpnMesh::Point(.0f, .0f, .0f));
    vertexPntr v1 = guiMesh.add_vertex(OpnMesh::Point(0.5f, .0f, .0f)); // also in quad 2
    vertexPntr v2 = guiMesh.add_vertex(OpnMesh::Point(0.5f, 1.0f, .0f)); // also in quad 2
    vertexPntr v3 = guiMesh.add_vertex(OpnMesh::Point(.0f, 1.0f, .0f));

    // quad 2
    vertexPntr v4 = guiMesh.add_vertex(OpnMesh::Point(1.0f, .0f, .0f));
    vertexPntr v5 = guiMesh.add_vertex(OpnMesh::Point(1.0f, 1.0f, .0f));

    // create the two quads (two faces)
    vector<vertexPntr> face;
    face.push_back(v0);
    face.push_back(v1);
    face.push_back(v2);
    face.push_back(v3);
    guiMesh.add_face(face);

    face.clear();
    face.push_back(v1);
    face.push_back(v4);
    face.push_back(v5);
    face.push_back(v2);
    guiMesh.add_face(face);

    try
      {
        if ( !OpenMesh::IO::write_mesh(guiMesh, "output.off") )
        {
          std::cerr << "Cannot write mesh to file 'output.off'" << std::endl;
        }
      }
      catch( std::exception& x )
      {
        std::cerr << x.what() << std::endl;
      }*/

}
