#include "GUILogic/meshhandler.h"
#include <QDebug>
#include <QPointF>

#include <OpenMesh/Core/IO/MeshIO.hh>


using namespace GUILogic;

typedef OpenMesh::PolyMesh_ArrayKernelT<OpenMeshExt::CustomTraits> OpnMesh;
typedef OpnMesh::VertexHandle vertexHandle;
typedef subdivMesh::Mesh SbdvMesh;


MeshHandler::MeshHandler() :
    subdMesh{nullptr}
{
    // test OpenMesh lib
   // createTwoQuads();
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

vector<QPointF> MeshHandler::getVertices()
{
    vector<QPointF> result;
    for (OpnMesh::VertexIter v_it = guiMesh.vertices_sbegin();
           v_it != guiMesh.vertices_end(); ++v_it)
    {
        QPointF point(guiMesh.point(v_it)[0], guiMesh.point(v_it)[1]);

        //For debugging. TODO: Remove
        qDebug() << "From Meshhandler:49: " << point;

        result.push_back(point);

    }
    return result;
}

void MeshHandler::addVertexFromPoint(QPointF& position)
{
    float x = static_cast <float> (position.x());
    float y = static_cast <float> (position.y());
    vertexHandle handler= guiMesh.add_vertex(OpnMesh::Point(x,y,.0f));
    vertexHandlers.push_back(handler);
}

bool MeshHandler::makeFace()
{
    vector<OpnMesh::VertexHandle> vHandler;
    for (OpnMesh::VertexIter v_it = guiMesh.vertices_begin();
           v_it != guiMesh.vertices_end(); ++v_it)
    {
         vHandler.push_back(v_it);
    }
    if(vHandler.size()<=2){
        return false;
    }

    guiMesh.add_face(vHandler);
    return true;
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

bool MeshHandler::saveGuiMeshOff(QString location){
    try
    {
        if(!OpenMesh::IO::write_mesh(guiMesh, location.toStdString())){
            qDebug("Cannot write mesh to file..");
            return false;
        }
    }catch (std::exception& x){
        qDebug(x.what());
        return false;
    }
    return true;
}

bool MeshHandler::importGuiMesh(QString location)
{
    if ( ! OpenMesh::IO::read_mesh(guiMesh, location.toStdString()) )
      {
        qDebug() << "Error: Cannot read mesh from " << location;
        return false;
      }
    return true;
}

void MeshHandler::createTwoQuads()
{
    // create vertices: (quad 1)
    vertexHandle v0 = guiMesh.add_vertex(OpnMesh::Point(.0f, .0f, .0f));
    vertexHandle v1 = guiMesh.add_vertex(OpnMesh::Point(.0f, .0f, .0f)); // also in quad 2
    vertexHandle v2 = guiMesh.add_vertex(OpnMesh::Point(0.5f, 1.0f, .0f)); // also in quad 2
    vertexHandle v3 = guiMesh.add_vertex(OpnMesh::Point(.0f, .0f, .0f));

    // quad 2
    vertexHandle v4 = guiMesh.add_vertex(OpnMesh::Point(1.0f, .0f, .0f));
    vertexHandle v5 = guiMesh.add_vertex(OpnMesh::Point(1.0f, 1.0f, .0f));

    // create the two quads (two faces)
    vector<vertexHandle> face;
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
