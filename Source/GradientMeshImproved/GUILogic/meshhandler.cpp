#include "GUILogic/meshhandler.h"
#include <QDebug>

#include <OpenMesh/Core/IO/MeshIO.hh>

using namespace GUILogic;

typedef subdivMesh::Mesh SbdvMesh;
typedef OpenMesh::PolyMesh_ArrayKernelT<OpenMeshExt::CustomTraits> OpnMesh;
typedef OpnMesh::VertexHandle vertexPntr;

MeshHandler::MeshHandler() :
    subdMesh{nullptr}
{
    // test OpenMesh lib
    createTwoQuads();
}

MeshHandler::~MeshHandler()
{
    delete subdMesh;
}

void MeshHandler::drawGLMesh(QOpenGLFunctions_1_0* context)
{
    // draw our two meshes (mesh creation should not happen here of course...)
    setUpSubdMeshFile();
    subdMesh->draw(context);

    setUpSubdMeshStream();
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

    SbdvMesh *currentMsh = new SbdvMesh();
    SbdvMesh *nextMesh;

    // perform the first step of subdivision (special rules - Lieng et al.)
    subdMesh->LinearTernarySubdiv(currentMsh);
    delete subdMesh; // delete old mesh from heap

    // subdivide steps-1 of CC-subdivision
    for(int i = 0; i < steps-1; i++) {
        nextMesh = new SbdvMesh();
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
    vertexPntr v0 = guiMesh.add_vertex(OpnMesh::Point(.0f, .0f, .0f));
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
      }

}

void MeshHandler::setUpSubdMeshStream()
{
    // setup a string stream
    stringstream strStream;
    strStream << TESTMESH; // pass string to stream (other types can also be passed -> see stringstream doc

    // delete current mesh object and insert a new one
    delete subdMesh; // delete from heap
    subdMesh = new SbdvMesh();

    // insert new mesh using custom OFF format
    subdMesh->loadV3(strStream);
    subdMesh->build(); // build mesh topology from data
    subdivide();
}

void MeshHandler::setUpSubdMeshFile()
{
    // create new mesh object
    delete subdMesh; // delete from heap (if any)
    subdMesh = new SbdvMesh();

    subdMesh->loadV3(TEMPFILEPATH);
    subdMesh->build(); // must be called after load
    subdivide();
}
