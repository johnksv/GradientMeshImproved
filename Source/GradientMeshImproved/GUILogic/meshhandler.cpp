#include "GUILogic/meshhandler.h"
#include <QDebug>
#include <QPointF>
#include <QColor>

#include <OpenMesh/Core/IO/MeshIO.hh>


using namespace GUILogic;

typedef OpenMesh::PolyMesh_ArrayKernelT<OpenMeshExt::CustomTraits> OpnMesh;
typedef OpnMesh::VertexHandle vertexHandle;
typedef subdivMesh::Mesh SbdvMesh;


MeshHandler::MeshHandler() :
    subdMesh{nullptr}
{

}

MeshHandler::~MeshHandler()
{
    delete subdMesh;
}

void MeshHandler::drawGLMesh(QOpenGLFunctions_1_0* context)
{
    if(subdMesh != nullptr)
    {
        subdMesh->draw(context);
    }

    // draw our two meshes (mesh creation should not happen here of course...)
//    setUpSubdMeshFile();
//    subdMesh->draw(context);

//    setUpSubdMeshStream();
//    subdMesh->draw(context);
}

vector<QPointF> MeshHandler::vertices()
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

QVector3D MeshHandler::color(int index)
{
    if( index <0 || index > vertexHandlers.size())
    {
        return QVector3D();
    }

    return guiMesh.data(vertexHandlers[index]).color();

}

void MeshHandler::setColor(int index, QColor color)
{
    if( index <0 || index > vertexHandlers.size())
    {
        throw "Index out of bounds";
    }

    QVector3D color_ = QVector3D(color.red(), color.green(), color.blue());
    guiMesh.data(vertexHandlers[index]).setColor(color_);
}

double MeshHandler::weight(int index)
{
    if( index <0 || index > vertexHandlers.size())
    {
        throw "Fatal error";
    }

    return guiMesh.data(vertexHandlers[index]).weight();
}

bool MeshHandler::setWeight(int index, double weight)
{
    if( index <0 || index > vertexHandlers.size()){
        throw "Fatal error";
    }

    guiMesh.data(vertexHandlers[index]).setWeight(weight);
    return true;
}

void MeshHandler::addVertexFromPoint(QPointF& position, QColor color)
{
    float x = static_cast <float> (position.x());
    float y = static_cast <float> (position.y());
    vertexHandle handler= guiMesh.add_vertex(OpnMesh::Point(x,y,.0f));
    vertexHandlers.push_back(handler);

    setColor(vertexHandlers.size()-1, color);
}

void MeshHandler::removeVertex(int index)
{
    vertexHandle handle = vertexHandlers.at(index);
    guiMesh.delete_vertex(handle);
}



bool MeshHandler::makeFace()
{
//    vector<OpnMesh::VertexHandle> vHandler;
//    for (OpnMesh::VertexIter v_it = guiMesh.vertices_begin();
//           v_it != guiMesh.vertices_end(); ++v_it)
//    {
//         vHandler.push_back(v_it);
//    }
    if(vertexHandlers.size()<=2){
        return false;
    }
    try
    {
        guiMesh.add_face(vertexHandlers);
    }catch(exception& x){
        qDebug() << x.what();
    }

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

void MeshHandler::prepareGuiMeshForSubd()
{
    string tempString;
    tempString +="OFF\n";

    size_t vertices = guiMesh.n_vertices();
    size_t edges = guiMesh.n_edges();
    //0 for Lab, 1 for RGB.
    short colormode = 1;

    //Metadata
    tempString.append(to_string(vertices) + " " + to_string(edges) + " " + to_string(colormode) + "\n");

    for(OpnMesh::VertexIter ite = guiMesh.vertices_sbegin();
        ite != guiMesh.vertices_end(); ite++ )
    {
        OpnMesh::Point point = guiMesh.point(ite);
        QVector3D color = guiMesh.data(ite).color();
        //x y z
        tempString += to_string(point[0]) + " " + to_string(point[1]) + " " + to_string(point[2]);
        tempString += " ";

        //r g b
        tempString += to_string(color.x()) + " " + to_string(color.y()) + " " + to_string(color.z());
        tempString += " ";

        //valence
        unsigned int valence = guiMesh.valence(ite);
        tempString += to_string(valence);
        tempString += " ";

            //id_to_neighbour..valence
            for(OpnMesh::VertexVertexIter vv_ite = guiMesh.vv_begin(ite);
                vv_ite != guiMesh.vv_end(ite); vv_ite++)
            {
                for(int i = 0; i < vertexHandlers.size(); i++)
                {
                    if(*vv_ite == vertexHandlers[i])
                    {
                        tempString += to_string(i);
                        tempString += " ";
                        i = vertexHandlers.size();
                    }
                }
            }

            //weight..valence
            tempString += to_string(guiMesh.data(ite).weight());
            tempString += " ";

        //0..valency(not in use)
        for(unsigned int i = 0; i < valence; i++)
        {
            tempString += "0";
            tempString += " ";
        }

        //0
        tempString += "0";
        //New point
        tempString += "\n";
    }

    //TODO: Add edges

    qDebug() << QString::fromUtf8(tempString.c_str());

    // delete current mesh object and insert a new one
    delete subdMesh; // delete from heap
    subdMesh = new SbdvMesh();

    //Convert stringstream for support with V2
    qDebug() <<"Sucsess with loadV2?" << subdMesh->loadV2(tempString.c_str());
    subdMesh->build(); // build mesh topology from data
    subdivide();
}

bool MeshHandler::saveGuiMeshOff(QString location)
{
    //TODO: Custom save
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
    //TODO: Support for multiple formats
    if ( ! OpenMesh::IO::read_mesh(guiMesh, location.toStdString()) )
      {
        qDebug() << "Error: Cannot read mesh from " << location;
        return false;
      }
    return true;
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
