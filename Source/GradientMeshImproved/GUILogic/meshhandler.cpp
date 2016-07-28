#include "GUILogic/meshhandler.h"
#include <QDebug>
#include <QPointF>
#include <QVector4D>
#include <QColor>
#include "subdivMesh/utils.h"
#include <QLineF>

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
    setUpSubdMeshFile();
    subdMesh->draw(context);

    setUpSubdMeshStream();
    subdMesh->draw(context);
}

vector<QVector4D> MeshHandler::vertices()
{
    vector<QVector4D> result;

    for (OpnMesh::VertexIter v_it = guiMesh.vertices_sbegin();
           v_it != guiMesh.vertices_end(); ++v_it)
    {
        QVector4D point(guiMesh.point(v_it)[0], guiMesh.point(v_it)[1], guiMesh.point(v_it)[2], v_it->idx());
        qDebug() << v_it->idx();
        //For debugging. TODO: Remove
        qDebug() << "From Meshhandler:49: " << point;

        result.push_back(point);

    }
    return result;
}

int MeshHandler::addVertex(const QPointF &position, const QColor color)
{
    float x = static_cast <float> (position.x());
    float y = static_cast <float> (position.y());
    vertexHandle handler = guiMesh.add_vertex(OpnMesh::Point(x, y, .0f));
    vertexHandlers.push_back(handler);
    setVertexColor(vertexHandlers.size() - 1, color);
    return handler.idx();
}

void MeshHandler::removeVertex(int idx)
{
    int index = findVertexHandler(idx);
    vertexHandle handle = vertexHandlers.at(index);
    guiMesh.delete_vertex(handle);
    vertexHandlers.erase(vertexHandlers.begin()+index);
}

void MeshHandler::setVertexPoint(int idx, const QPointF &position)
{
        float x = static_cast <float> (position.x());
        float y = static_cast <float> (position.y());
        int index = findVertexHandler(idx);
        guiMesh.set_point(vertexHandlers.at(index), OpnMesh::Point(x, y, .0f));
}

QVector3D MeshHandler::vertexColor(int idx)
{
    int index = findVertexHandler(idx);
    return guiMesh.data(vertexHandlers.at(index)).color();
}

void MeshHandler::setVertexColor(int idx, QColor color)
{
    int index = findVertexHandler(idx);
    QVector3D color_ = QVector3D(color.red(), color.green(), color.blue());
    guiMesh.data(vertexHandlers.at(index)).setColor(color_);
}

double MeshHandler::vertexWeight(int idx)
{
    int index = findVertexHandler(idx);
    return guiMesh.data(vertexHandlers.at(index)).weight();
}

bool MeshHandler::setVertexWeight(int idx, double weight)
{
    int index = findVertexHandler(idx);
    guiMesh.data(vertexHandlers.at(index)).setWeight(weight);
    return true;
}

uint MeshHandler::vertexValence(int idx)
{
    int index = findVertexHandler(idx);
    return guiMesh.valence(vertexHandlers.at(index));
}

vector<QVector4D> MeshHandler::edges()
{
    vector<QVector4D> result;

    for (OpnMesh::EdgeIter e_it = guiMesh.edges_sbegin();
           e_it != guiMesh.edges_end(); ++e_it)
    {
        vertexHandle startVert = guiMesh.from_vertex_handle(guiMesh.halfedge_handle(e_it, 0));
        vertexHandle endVert = guiMesh.from_vertex_handle(guiMesh.halfedge_handle(e_it, 1));
        QVector4D edge(startVert.idx(),endVert.idx(), e_it->idx(), 0);

        result.push_back(edge);

    }
    return result;
}

void GUILogic::MeshHandler::insertVertexOnEdge(int edgeIdx, int vertexIdx)
{
    int vertexIndex = findVertexHandler(vertexIdx);
    //TODO: implement

}

bool MeshHandler::makeFace(vector<int>& vertexHandlersIdx)
{
    //TODO: Check orientation of edge

    vector<vertexHandle> vHandlersFace;
    for(int idx : vertexHandlersIdx)
    {
        int index = findVertexHandler(idx);
        vHandlersFace.push_back(vertexHandlers.at(index));
    }
    vertexHandle lastVertex = vHandlersFace.back();
    OpnMesh::Point lastPoint = guiMesh.point(lastVertex);

    vertexHandle secondLastVertex = vHandlersFace.at(vHandlersFace.size()-2);
    QLineF toBeNewEdge(lastPoint[0],lastPoint[1],
                        guiMesh.point(secondLastVertex)[0],guiMesh.point(secondLastVertex)[1]);


    //To be the halfedge that is outgoing from the last vertex the user clicked.
    OpnMesh::HalfedgeHandle outgoingHalfedge;

    double angle = 361;
    //Outgoing halfedge always point towards the start vertex. Assumes that the faces always are added in same orientation (CW or CCW).
    for(OpnMesh::VertexOHalfedgeIter vOutH_ite = guiMesh.voh_begin(lastVertex); vOutH_ite != guiMesh.voh_end(lastVertex); vOutH_ite++)
    {
        if(guiMesh.is_boundary(vOutH_ite))
        {
            //Calculate angle relative to the new edge
            OpnMesh::Point point = guiMesh.point(guiMesh.to_vertex_handle(*vOutH_ite));
            QLineF vertVertEdge(lastPoint[0],lastPoint[1],point[0],point[1]);

            double angleTo = toBeNewEdge.angleTo(vertVertEdge);

            //The edge with the smallest angle relative to the new edge should be the next vertex in the face.
            if(angleTo < angle)
            {
                 angle = angleTo;
                 outgoingHalfedge = vOutH_ite;
            }
        }
    }

    if(outgoingHalfedge.is_valid())
    {
        bool  runLoop = true;
        while(runLoop)
        {
            vertexHandle nextVertex = guiMesh.to_vertex_handle(outgoingHalfedge);
            outgoingHalfedge = guiMesh.next_halfedge_handle(outgoingHalfedge);

            if(nextVertex != vHandlersFace.front())
            {
                vHandlersFace.push_back(nextVertex);
                vertexHandlersIdx.push_back(nextVertex.idx());
            }
            else
            {
                runLoop = false;
            }
        }
    }



    faceHandlers.push_back(guiMesh.add_face(vHandlersFace));

    //Check which vertecies are in current face, for debugging
    //Crashes if new face are added in opposite direction.
    for (OpnMesh::FaceVertexIter fv_ite = guiMesh.fv_begin(faceHandlers.back()); fv_ite != guiMesh.fv_end(faceHandlers.back()); fv_ite++)
    {
        qDebug() << "vertexHandleIdx: " << fv_ite->idx();
    }

    return true;
}

void MeshHandler::clearAll()
{
    vertexHandlers.clear();
    faceHandlers.clear();
    guiMesh.clear();
    guiMesh.garbage_collection();
}

/********************************************
 * HELPER METHODS
********************************************/

int GUILogic::MeshHandler::findVertexHandler(int idxToFind)
{
    for (int i = 0; i < vertexHandlers.size(); i++)
    {
        if (vertexHandlers.at(i).idx() == idxToFind)
        {
            return i;
        }
    }
    return -1;
}

int GUILogic::MeshHandler::findFaceHandler(int idxToFind)
{
    for (int i = 0; i < faceHandlers.size(); i++)
    {
        if (faceHandlers.at(i).idx() == idxToFind)
        {
            return i;
        }
    }
    return -1;
}

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
    size_t faces = guiMesh.n_faces();
    //0 for Lab, 1 for RGB.
    short colormode = 1;

    //Metadata
    tempString.append(to_string(vertices) + " " + to_string(faces) + " " + to_string(colormode) + "\n");

    for(OpnMesh::VertexIter ite = guiMesh.vertices_sbegin();
        ite != guiMesh.vertices_end(); ite++ )
    {
        OpnMesh::Point point = guiMesh.point(ite);
        QVector3D color = guiMesh.data(ite).color();
        //x y z
        tempString += to_string(point[0]) + " " + to_string(point[1]) + " " + to_string(point[2]);
        tempString += " ";

        //l a b
        double l, a, b;
        subdivMesh::RGB2LAB(color.x(), color.y(), color.z(), l, a, b);
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
        for(unsigned int i = 0; i < valence; i++)
        {
            tempString += to_string(guiMesh.data(ite).weight());
            tempString += " ";
        }

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

    //Faces:
    tempString += "\n";
    for(int i = 0; i < faceHandlers.size(); i++)
    {
        int counterValence = 0;
        string vertexInFace;
        for(OpnMesh::FaceVertexIter fv_ite = guiMesh.fv_begin(faceHandlers[i]);
            fv_ite != guiMesh.fv_end(faceHandlers[i]); fv_ite++)
        {
            counterValence++;
            vertexInFace += to_string(fv_ite->idx()) + " ";
        }
        tempString += to_string(counterValence) + " ";
        tempString += vertexInFace;

        tempString += "\n";
    }


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
    //TODO: If there alleready are vertices,edges and faces in guiMesh
    if ( ! OpenMesh::IO::read_mesh(guiMesh, location.toStdString()) )
      {
        qDebug() << "Error: Cannot read mesh from " << location;
        return false;
      }
    qDebug() << "ImportGuiMesh";
    for(OpnMesh::VertexIter v_ite = guiMesh.vertices_sbegin (); v_ite != guiMesh.vertices_end(); v_ite++)
    {
        vertexHandlers.push_back(*v_ite);
    }
    for(OpnMesh::FaceIter f_ite = guiMesh.faces_sbegin (); f_ite != guiMesh.faces_end(); f_ite++)
    {
        faceHandlers.push_back(*f_ite);
        qDebug() << "Added face";
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
    subdMesh->loadV2(TEMPFILEPATH2);
    subdMesh->build(); // must be called after load
    subdivide();
}
