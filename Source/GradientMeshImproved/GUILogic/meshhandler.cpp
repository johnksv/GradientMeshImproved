#include "GUILogic/meshhandler.h"
#include <QDebug>
#include <QPoint>

#include <OpenMesh/Core/IO/MeshIO.hh>


using namespace GUILogic;

typedef OpenMesh::PolyMesh_ArrayKernelT<OpenMeshExt::CustomTraits> OpnMesh;
typedef OpnMesh::VertexHandle vertexPntr;
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

vector<vector<float>> MeshHandler::getVertices()
{
    vector<vector<float>> result;
    for (OpnMesh::VertexIter v_it = guiMesh.vertices_sbegin();
           v_it != guiMesh.vertices_end(); ++v_it)
    {
        vector<float> point;
        point.push_back(guiMesh.point(v_it)[0]);
        point.push_back(guiMesh.point(v_it)[1]);
        point.push_back(guiMesh.point(v_it)[2]);

        //For debugging. TODO: Remove
        qDebug() << "From Meshhandler:49:  x:" << guiMesh.point(v_it)[0] << ", y:" << guiMesh.point(v_it)[1]
                 << ",z: " << guiMesh.point(v_it)[2];

        result.push_back(point);

    }
    return result;
}

void MeshHandler::addVertexFromPoint(QPoint& position)
{
    //TODO: If vertex exists -> make face
    float x = static_cast <float> (position.x());
    float y = static_cast <float> (position.y());
    guiMesh.add_vertex(OpnMesh::Point(x,y,.0f));
}

void MeshHandler::makeFace()
{

    for (OpnMesh::VertexIter v_it = guiMesh.vertices_begin();
           v_it != guiMesh.vertices_end(); ++v_it)
    {

    }

    for (OpnMesh::EdgeIter e_it=guiMesh.edges_begin(); e_it!=guiMesh.edges_end(); ++e_it)
    {

    }
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

void MeshHandler::saveGuiMeshOff(QString location){
    try
    {
        if(!OpenMesh::IO::write_mesh(guiMesh, location.toStdString())){
            qDebug("Cannot write mesh to file..");
        }
    }catch (std::exception& x){
        qDebug(x.what());
    }
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
