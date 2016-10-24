#include "GUILogic/meshhandler.h"
#include <QDebug>
#include <QPointF>
#include <QVector4D>
#include <QColor>
#include "subdivMesh/utils.h"
#include <QLineF>
#include "subdivMesh/mesh.h"
#include <QPolygonF>

#include <OpenMesh/Core/IO/MeshIO.hh>


using namespace GUILogic;

typedef subdivMesh::Mesh SbdvMesh;


int MeshHandler::subdivisionSteps_ = 3;

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
}

vector<vector<QVector4D>> MeshHandler::vertices()
{
    vector<vector<QVector4D>> result;

    for (OpnMesh::VertexIter v_it = guiMesh.vertices_sbegin();
           v_it != guiMesh.vertices_end(); ++v_it)
    {
        OpnMesh::Point vertPoint = guiMesh.point(v_it);
        OpnMesh::Color vertColor = guiMesh.color(v_it);
        QVector4D point(vertPoint[0], vertPoint[1], vertPoint[2], v_it->idx());
        QVector4D color(vertColor[0],vertColor[1], vertColor[2], .0f);

        vector<QVector4D> vert;
        vert.push_back(point);
        vert.push_back(color);
        result.push_back(vert);

    }
    return result;
}

int MeshHandler::addVertex(const QPointF &position, const QColor color)
{
    float x = static_cast <float> (position.x());
    float y = static_cast <float> (position.y());
    vertexHandle handler = guiMesh.add_vertex(OpnMesh::Point(x, y, .0f));
    setVertexColor(handler.idx(), color);
    return handler.idx();
}

void MeshHandler::deleteVertex(int idx)
{
    vertexHandle handle = guiMesh.vertex_handle(idx);
    guiMesh.delete_vertex(handle);
}

QVector3D MeshHandler::vertexPoint(int idx)
{
    vertexHandle vhandle = guiMesh.vertex_handle(idx);
    OpnMesh::Point point = guiMesh.point(vhandle);
    return QVector3D(point[0], point[1],point[2]);
}

void MeshHandler::setVertexPoint(int idx, const QPointF &position)
{
    float x = static_cast <float> (position.x());
    float y = static_cast <float> (position.y());

    guiMesh.set_point(guiMesh.vertex_handle(idx), OpnMesh::Point(x, y, .0f));
}

QVector3D MeshHandler::vertexColor(int idx)
{
    OpnMesh::Color color = guiMesh.color(guiMesh.vertex_handle(idx));
    return QVector3D(color[0],color[1], color[2]);
}

void MeshHandler::setVertexColor(int idx, QColor color)
{
    OpnMesh::Color colr(color.red(), color.green(), color.blue());
    guiMesh.set_color(guiMesh.vertex_handle(idx), colr);
}

uint MeshHandler::vertexValence(int idx)
{
    return guiMesh.valence(guiMesh.vertex_handle(idx));
}

bool MeshHandler::isBoundaryVertex(int idx)
{
    vertexHandle vh = guiMesh.vertex_handle((idx));
    return guiMesh.is_boundary(vh);
}

bool MeshHandler::isValidVertex(int idx)
{
    return guiMesh.is_valid_handle(guiMesh.vertex_handle(idx));
}

bool MeshHandler::isBoundaryEdge(int startIdx, int endIdx)
{
    vertexHandle startHandle = guiMesh.vertex_handle(startIdx);
    vertexHandle endHandle = guiMesh.vertex_handle(endIdx);

    for(OpnMesh::ConstVertexEdgeIter ve_ite = guiMesh.cve_iter(startHandle); ve_ite != guiMesh.cve_end(startHandle); ve_ite++)
    {
        if(guiMesh.to_vertex_handle(guiMesh.halfedge_handle(ve_ite,0)) == endHandle
                || guiMesh.to_vertex_handle(guiMesh.halfedge_handle(ve_ite,1)) == endHandle)
        {
            return guiMesh.is_boundary(ve_ite);
        }
    }
    return true;
}

void MeshHandler::setConstraints(int halfedgeFromVertIdx, int halfedgeToVertIdx, subdivMesh::Point_3D constraints)
{
    setConstraints(halfedgeFromVertIdx, halfedgeToVertIdx, QVector2D(constraints.getX(), constraints.getY()));
}

void MeshHandler::setConstraints(int halfedgeFromVertIdx, int halfedgeToVertIdx, QVector2D constraints)
{
    vertexHandle outgoingVert = guiMesh.vertex_handle(halfedgeFromVertIdx);
    vertexHandle endVert = guiMesh.vertex_handle(halfedgeToVertIdx);
    for(OpnMesh::VertexOHalfedgeIter voh_ite = guiMesh.voh_begin(outgoingVert); voh_ite != guiMesh.voh_end(outgoingVert); voh_ite++)
    {
        if(guiMesh.to_vertex_handle(voh_ite) == endVert)
        {
            guiMesh.data(voh_ite).setConstraint(constraints);
            return;
        }
    }
}

QVector2D MeshHandler::constraints(int halfedgeFromVertIdx, int halfedgeToVertIdx)
{
    vertexHandle outgoingVert = guiMesh.vertex_handle(halfedgeFromVertIdx);
    vertexHandle endVert = guiMesh.vertex_handle(halfedgeToVertIdx);
    for(OpnMesh::VertexOHalfedgeIter voh_ite = guiMesh.voh_begin(outgoingVert); voh_ite != guiMesh.voh_end(outgoingVert); voh_ite++)
    {
        if(guiMesh.to_vertex_handle(voh_ite) == endVert)
        {
            return guiMesh.data(voh_ite).constraint();
        }
    }
}

vector<QVector2D> MeshHandler::constraints(int edgeIdx)
{
    vector<QVector2D> result;
	OpnMesh::EdgeHandle eHandle = guiMesh.edge_handle(edgeIdx);
	OpnMesh::HalfedgeHandle firstHandle = guiMesh.halfedge_handle(eHandle, 0);
	OpnMesh::HalfedgeHandle secondHandle = guiMesh.halfedge_handle(eHandle, 1);
    result.push_back(guiMesh.data(firstHandle).constraint());
    result.push_back(guiMesh.data(secondHandle).constraint());
	
    return result;
}

void MeshHandler::deleteDiscontinuedFace(vector<int> &vertexHandlersIdx)
{
    assert(vertexHandlersIdx.size() == 4);

    //TODO: Ugly function. Make it prettier.
    for(int i = 1; i <=2; i++)
    {
        vertexHandle mainVertex;
        vertexHandle toCollapse;
        if(i == 1)
        {
            int startVertIdx = vertexHandlersIdx.front();
            mainVertex = guiMesh.vertex_handle((startVertIdx));

            int startDiscontinuedIdx =  vertexHandlersIdx.at(1);
            toCollapse = guiMesh.vertex_handle((startDiscontinuedIdx));
        }
        else
        {
            int endVertIdx = vertexHandlersIdx.back();
            mainVertex = guiMesh.vertex_handle((endVertIdx));

            int endDiscontinuedIdx =  vertexHandlersIdx.at(2);
            toCollapse = guiMesh.vertex_handle((endDiscontinuedIdx));
        }

        for (OpnMesh::VertexIHalfedgeIter vih_ite = guiMesh.vih_begin(mainVertex); vih_ite != guiMesh.vih_end(mainVertex); vih_ite++)
        {
            if(guiMesh.from_vertex_handle(vih_ite) == toCollapse)
            {
                if(guiMesh.is_collapse_ok(vih_ite))
                {
                    qDebug() << "Collapse is OKAY!";
                    guiMesh.collapse(vih_ite);
                }
                break;
            }
        }
    }
    guiMesh.garbage_collection();
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

bool MeshHandler::collapseEdge(int startVertIdx, int endVertIdx)
{
    OpnMesh::VertexHandle startVert = guiMesh.vertex_handle(startVertIdx);
    OpnMesh::VertexHandle endVert = guiMesh.vertex_handle(endVertIdx);
    for (OpnMesh::VertexOHalfedgeIter voh_ite = guiMesh.voh_begin(startVert); voh_ite != guiMesh.voh_end(startVert); voh_ite++)
    {
		if (guiMesh.to_vertex_handle(voh_ite) == endVert)
		{
			if (guiMesh.is_collapse_ok(voh_ite))
			{
				guiMesh.collapse(voh_ite);
				guiMesh.garbage_collection();
				return true;
			}
			else
			{
				return false;
			}
		}
    }
	throw - 1;
}

int GUILogic::MeshHandler::insertVertexOnEdge(int edgeStartVertIdx, int edgeEndVertIdx, const QPointF &position, const QColor &color)
{
    int newVertIdx = addVertex(position, color);
    OpnMesh::VertexHandle splitVert = guiMesh.vertex_handle(newVertIdx);
    OpnMesh::VertexHandle startVert = guiMesh.vertex_handle(edgeStartVertIdx);
    OpnMesh::VertexHandle endVert = guiMesh.vertex_handle(edgeEndVertIdx);
    OpnMesh::HalfedgeHandle outHEdge;


    for (OpnMesh::VertexOHalfedgeIter voh_ite = guiMesh.voh_begin(startVert); voh_ite != guiMesh.voh_end(startVert); voh_ite++)
    {
        if(guiMesh.to_vertex_handle(voh_ite) == endVert)
        {
            guiMesh.split_edge(guiMesh.edge_handle(voh_ite), splitVert);
			outHEdge = voh_ite;
            break;
        }
    }

	QVector2D outConstraints = guiMesh.data(outHEdge).constraint();
	guiMesh.data(outHEdge).setConstraint(QVector2D(0.1f ,0.1f));

	guiMesh.data(guiMesh.halfedge_handle(guiMesh.n_halfedges() - 2)).setConstraint(outConstraints);
    return newVertIdx;
}

bool MeshHandler::makeFace(vector<int>& vertexHandlersIdx, bool faceInsideFace, bool sameStartAndEnd)
{
    if(sameStartAndEnd) return addFaceWIthSameStartAndEnd(vertexHandlersIdx, faceInsideFace);

    vector<vertexHandle> vHandlersToBeFace, orginalvHandlersFace;
    for(int i = 0; i < vertexHandlersIdx.size(); i++)
    {
        int index = vertexHandlersIdx.at(i);
        vHandlersToBeFace.push_back(guiMesh.vertex_handle(index));
    }
    //Used if rotation of added vertices are wrong
    orginalvHandlersFace = vHandlersToBeFace;

    //Variable to hold the face that is added by the vertices.
    OpnMesh::FaceHandle newFace;

    //Variable to hold if loop must be run again to fix orientation
    bool loopToFixOrientation = false;

    do
    {
        qDebug() << "Enter loop";
        vertexHandle lastVertex = vHandlersToBeFace.back();
        OpnMesh::Point lastPoint = guiMesh.point(lastVertex);

        vertexHandle secondLastVertex = vHandlersToBeFace.at(vHandlersToBeFace.size()-2);
        //Used to find the halfedges that is leading to the closest vertices that should be a part of the face.
        QLineF toBeNewEdge(lastPoint[0],lastPoint[1],
                            guiMesh.point(secondLastVertex)[0],guiMesh.point(secondLastVertex)[1]);


        //To be the halfedge that is outgoing from the last vertex the user clicked.
        OpnMesh::HalfedgeHandle outgoingHalfedge;

        double angle = 361;
        //Outgoing halfedge always point towards the start vertex. Assumes that the faces always are added in same orientation (CW or CCW).
        for(OpnMesh::VertexOHalfedgeIter vOutH_ite = guiMesh.voh_begin(lastVertex); vOutH_ite != guiMesh.voh_end(lastVertex); vOutH_ite++)
        {
            if(guiMesh.is_boundary(vOutH_ite) || faceInsideFace)
            {
                //Calculate angle relative to the new edge
                OpnMesh::Point point = guiMesh.point(guiMesh.to_vertex_handle(vOutH_ite));
                QLineF vertVertEdge(lastPoint[0],lastPoint[1],point[0],point[1]);

                double angleTo = toBeNewEdge.angleTo(vertVertEdge);
                qDebug() << "angleTo" << angleTo;
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
            qDebug() << "Following edge from vertex " << guiMesh.from_vertex_handle(outgoingHalfedge).idx()
                << ", to vertex " << guiMesh.to_vertex_handle(outgoingHalfedge).idx();
            int i = 0;
            while(true)
            {
                vertexHandle nextVertex = guiMesh.to_vertex_handle(outgoingHalfedge);
                outgoingHalfedge = guiMesh.next_halfedge_handle(outgoingHalfedge);

                if(nextVertex != vHandlersToBeFace.front())
                {
                    vHandlersToBeFace.push_back(nextVertex);
                }
                else
                {
                    break;
                }
                i++;
                if(i > guiMesh.n_halfedges())
                {
                    //TODO: Fix. Not good practice.
                    throw -1;
                }
            }
        }

        //If faceInsideFace, or if vertesToAddFace_.size() == 2 (canvas.cpp) (outgoingHalfedge is then invalid)
        if (faceInsideFace)
        {
            //if the handlers are neighbours (Which means no face can be made by following edges).
			if (vHandlersToBeFace.size() <= 2) return false;

            if (guiMesh.face_handle(outgoingHalfedge).is_valid())
            {
                OpnMesh::FaceHandle &oldFace = guiMesh.face_handle(outgoingHalfedge);
                vector<vertexHandle> verticesOldFace;
                //Iterate over vertices in oldFace
                for (OpnMesh::FaceVertexIter fv_ite = guiMesh.fv_begin(oldFace); fv_ite != guiMesh.fv_end(oldFace); fv_ite++)
                {
                    verticesOldFace.push_back(fv_ite);
                }
                //delete old face
                guiMesh.delete_face(oldFace, false);

                newFace = guiMesh.add_face(vHandlersToBeFace);

                //Special case. If you are adding a face inside the first face. That means the first face is deleted and invalid ( guiMesh.delete_face(oldFace, false) above)
                if (guiMesh.n_faces() > 1)
                {
                    //Check if newFace has correct orientation, else, make correct face
                    if (faceOrientation(orginalvHandlersFace, newFace, vHandlersToBeFace))
                    {
                        qDebug() << "Wrong rotation.. Recursive call to makeFace";
                        if (!makeFace(vertexHandlersIdx)) {
                            qDebug() << "Did not make face. TODO: Check for error";
                        }
                    }
                }

                //Calculate which vertices that should not be included in boundaryFace (because they were added in newFace instead).
                int startIndex = -1, endIndex =-1;
                for (int i = 0; i < verticesOldFace.size(); i++)
                {
                    if (verticesOldFace.at(i).idx() == vertexHandlersIdx.front())
                    {
                        startIndex = i;
                        break;
                    }
                }
                for (int i = 0; i < verticesOldFace.size(); i++)
                {
                    if (verticesOldFace.at(i).idx() == vertexHandlersIdx.back())
                    {
                        endIndex = i;
                        break;
                    }
                }

                /*	The only vertices left after erase are vertices not "touched" by the new face
                *	Eg: you have face ABCD and add AEFD inside ABCD,
                *	after erase oldFace/boundaryFace-vector will only contain BC.
                *	DEFA is added in next for-loop so the new face and the boundaryFace dosn't collide.
                *	Resulting faces will be:
                *		New Face (already added by the time this if-statement hits): AEFD
                *		Boundary face (to be added): BCDFEA
                */
                if (startIndex != -1 && endIndex != -1 && startIndex > endIndex)
                {
                    vector<vertexHandle> behindVerts;
                    for (auto v_ite = verticesOldFace.begin()+startIndex+1; v_ite != verticesOldFace.end(); v_ite++)
                    {
                        behindVerts.push_back(*v_ite);
                    }
                    verticesOldFace.erase(verticesOldFace.begin() + endIndex, verticesOldFace.end());
                    verticesOldFace.insert(verticesOldFace.begin(), behindVerts.begin(), behindVerts.end());

                }
                else
                {
                    if (startIndex != -1 && endIndex != -1)
                    {
                        verticesOldFace.erase(verticesOldFace.begin(), verticesOldFace.begin() + startIndex + 1);
                        //Line above: Index is shortened by startindex + 1 elements. Adjust endIndex accordingly.
                        endIndex -= startIndex+1;
                        verticesOldFace.erase(verticesOldFace.begin() + endIndex, verticesOldFace.end());
                    }
                    else if (startIndex != -1)
                    {
                        verticesOldFace.erase(verticesOldFace.begin(), verticesOldFace.begin() + startIndex + 1);
                    }
                    else if (endIndex != -1)
                    {
                        verticesOldFace.erase(verticesOldFace.begin() + endIndex, verticesOldFace.end());
                    }

                }

                //Adding the old face, but with the new vertices.
                //Continuation from example above:
                reverse(vertexHandlersIdx.begin(), vertexHandlersIdx.end());
                for (int i = 0; i < vertexHandlersIdx.size(); i++)
                {
                    int index = vertexHandlersIdx.at(i);
                    vertexHandle vertex = guiMesh.vertex_handle(index);
                    verticesOldFace.push_back(vertex);
                }


                OpnMesh::FaceHandle boundaryFace = guiMesh.add_face(verticesOldFace);
                qDebug() << "BoundaryFace made. Checking rotation.";
                //Special case. If you are adding a face inside the first face. That means the first face is deleted and invalid
                if (guiMesh.n_faces() > 1)
                {
                    //Should be OK, but needs testing.
                    if (faceOrientation(orginalvHandlersFace, boundaryFace, vHandlersToBeFace))
                    {
                        qDebug() << "Wrong orientation... TODO: Handle";
                    }
                }

                //Print vertex IDX for debugging.
                qDebug() << "Vertices in boundaryFace:";
                for (OpnMesh::FaceVertexIter fv_ite = guiMesh.fv_begin(boundaryFace); fv_ite != guiMesh.fv_end(boundaryFace); fv_ite++)
                {
                    qDebug() << "\tvertexHandleIdx: " << fv_ite->idx();
                    vertexHandlersIdx.push_back(fv_ite->idx());
                }
            }
            else
            {
                //If no verts are added in the above lines (Typically this is becuase the input verts is neighbours)
                qDebug() << "Size of verts" << QString::number(vHandlersToBeFace.size());

                newFace = guiMesh.add_face(vHandlersToBeFace);
                loopToFixOrientation = faceOrientation(orginalvHandlersFace, newFace, vHandlersToBeFace);
            }
        }
        else
        {
            newFace = guiMesh.add_face(vHandlersToBeFace);
            loopToFixOrientation = faceOrientation(orginalvHandlersFace, newFace, vHandlersToBeFace);
        }
    }while(loopToFixOrientation);

    //Check which vertices are in current face, for debugging
    vertexHandlersIdx.clear();
	qDebug() << "Vertices in the new face:";
    for (OpnMesh::FaceVertexIter fv_ite = guiMesh.fv_begin(newFace); fv_ite != guiMesh.fv_end(newFace); fv_ite++)
    {
        qDebug() << "\tvertexHandleIdx: " << fv_ite->idx();
        vertexHandlersIdx.push_back(fv_ite->idx());
    }
    return true;
}

bool MeshHandler::addFaceWIthSameStartAndEnd(vector<int> &vertexHandlersIdx, bool faceInsideFace)
{
	vector<vertexHandle> vHandlersToBeFace, orginalvHandlersFace;
	for (int i = 0; i < vertexHandlersIdx.size(); i++)
	{
		int index = vertexHandlersIdx.at(i);
		vHandlersToBeFace.push_back(guiMesh.vertex_handle(index));
	}
	//Used if rotation of added vertices are wrong
	orginalvHandlersFace = vHandlersToBeFace;

	//Variable to hold the face that is added by the vertices.
	OpnMesh::FaceHandle newFace;

	if (guiMesh.n_faces() == 0)
	{
		newFace = guiMesh.add_face(vHandlersToBeFace);
		if (faceOrientation(orginalvHandlersFace, newFace, vHandlersToBeFace))
		{
			newFace = guiMesh.add_face(vHandlersToBeFace);
		}
	}
	else
	{
        if (!faceInsideFace)
        {
            newFace = guiMesh.add_face(vHandlersToBeFace);
            if (faceOrientation(orginalvHandlersFace, newFace, vHandlersToBeFace))
            {
                newFace = guiMesh.add_face(vHandlersToBeFace);
            }
        }
        else
        {
           //TODO: Implement
        }
	}


	vertexHandlersIdx.clear();
    qDebug() << "Vertices in the new face (From addFaceWIthSameStartAndEnd):";
	for (OpnMesh::FaceVertexIter fv_ite = guiMesh.fv_begin(newFace); fv_ite != guiMesh.fv_end(newFace); fv_ite++)
	{
		qDebug() << "\tvertexHandleIdx: " << fv_ite->idx();
		vertexHandlersIdx.push_back(fv_ite->idx());
	}

    return true;
}

size_t MeshHandler::numberOfFaces()
{
    return guiMesh.n_faces();
}

bool MeshHandler::vertsOnSameFace(int vertIdx1, int vertIdx2)
{
    vertexHandle vert1 = guiMesh.vertex_handle((vertIdx1));
    vertexHandle vert2 = guiMesh.vertex_handle((vertIdx2));
    OpnMesh::FaceHandle faceHandle1;

    //Chech if same facehandler.
    for(OpnMesh::VertexOHalfedgeIter voh_ite = guiMesh.voh_begin(vert1); voh_ite != guiMesh.voh_end(vert1); voh_ite++)
    {
        faceHandle1 = guiMesh.face_handle(voh_ite);
        for(OpnMesh::VertexOHalfedgeIter voh2_ite = guiMesh.voh_begin(vert2); voh2_ite != guiMesh.voh_end(vert2); voh2_ite++)
        {
            if(faceHandle1 == guiMesh.face_handle(voh2_ite) && faceHandle1.is_valid())
            {
                return true;
            }
        }
    }

    return false;
}

vector<vector<int>> MeshHandler::facesIdx()
{
    vector<vector<int>> result;
    for (OpnMesh::FaceIter f_ite = guiMesh.faces_sbegin(); f_ite != guiMesh.faces_end(); f_ite++)
    {
        vector<int> vectorIdx;
        for (OpnMesh::FaceVertexIter fv_ite = guiMesh.fv_begin(f_ite); fv_ite != guiMesh.fv_end(f_ite); fv_ite++)
        {
            vectorIdx.push_back(fv_ite->idx());
        }
        result.push_back(vectorIdx);
    }
    return result;

}

void MeshHandler::deleteFace(const int &faceIdx, bool delete_isolated_vertices)
{
    guiMesh.delete_face(guiMesh.face_handle(faceIdx), delete_isolated_vertices);
}

bool MeshHandler::isBoundaryFace(const int &idx) const
{
    return guiMesh.is_boundary(guiMesh.face_handle(idx));
}

void MeshHandler::clearAll()
{
    delete subdMesh;
    delete oneStepSubdMesh_;

    subdMesh = nullptr;
    oneStepSubdMesh_ = nullptr;

    guiMesh.clear();
    guiMesh.garbage_collection();
}

/********************************************
 * HELPER METHODS
********************************************/

bool MeshHandler::faceOrientation(vector<vertexHandle> &orginalvHandlersFace, OpnMesh::FaceHandle &newFace, vector<vertexHandle> &vHandlersFace)
{		
    bool isValidFace = newFace.is_valid();
    OpnMesh::Normal newFaceNormal;

    if(isValidFace)
    {
        newFaceNormal = guiMesh.calc_face_normal(newFace);
    }
    //TODO: Implement check for 3D
    qDebug() << "Orientation: " << isValidFace << ", Normals: " << newFaceNormal[2];
    //Orientation is of if face is not valid, or z-component of normal is different (2D).
    if(!isValidFace || (1 - newFaceNormal[2])  > 0.00001)
    {
        vHandlersFace = orginalvHandlersFace;
        reverse(vHandlersFace.begin(), vHandlersFace.end());
        guiMesh.delete_face(newFace, false);
        return true;
    }

    return false;
}

// Subdivide mesh for rendering
// using method of Lieng et al.
void MeshHandler::subdivide()
{
    if(subdivisionSteps_==0) return; // not in function domain

    SbdvMesh *currentMsh = new SbdvMesh();
    SbdvMesh *nextMesh;

    // perform the first step of subdivision (special rules - Lieng et al.)
    subdMesh->LinearTernarySubdiv(currentMsh);
    delete subdMesh; // delete old mesh from heap

    oneStepSubdMesh_ = currentMsh;

    // subdivide steps-1 of CC-subdivision
    for(int i = 0; i < subdivisionSteps_-1; i++) {
        nextMesh = new SbdvMesh();
        currentMsh->CatmullClarkColour(nextMesh);

		if(oneStepSubdMesh_ != currentMsh)
		{
            delete currentMsh;
        }

        // delete old mesh from heap and swap
        currentMsh = nextMesh;
    }

    // set the final mesh
    subdMesh = currentMsh;
}

void MeshHandler::prepareGuiMeshForSubd(bool saveFileOFF, QString location)
{
    guiMesh.garbage_collection();
    string tempString;
    tempString +="OFF\n";

    size_t vertices = guiMesh.n_vertices();
    size_t faces = guiMesh.n_faces();
    if(faces  <= 0) return;

    //0 for Lab, 1 for RGB.
    short colormode = 0;

    //Metadata
    tempString.append(to_string(vertices) + " " + to_string(faces) + " " + to_string(colormode) + "\n");

    //Vertex data
    for(OpnMesh::VertexIter ite = guiMesh.vertices_sbegin(); ite != guiMesh.vertices_end(); ite++ )
    {
        OpnMesh::Point point = guiMesh.point(ite);
        OpnMesh::Color color = guiMesh.color(ite);

        //x y z
        tempString += to_string(point[0]) + " " + to_string(point[1]) + " " + to_string(1.0);
        tempString += " ";

        //l a b
        double l, a, b;
        //r g b
        double r = (double)color[0]/255;
        double g = (double)color[1]/255;
        double bb = (double)color[2]/255;

        subdivMesh::RGB2LAB(r, g, bb, l, a, b);
        tempString += to_string(l) + " " + to_string(a) + " " + to_string(b);
        tempString += " ";

        //valence
        unsigned int valence = guiMesh.valence(ite);
        tempString += to_string(valence);
        tempString += " ";

        //id_to_neighbour..valence
        for(OpnMesh::VertexVertexIter vv_ite = guiMesh.vv_begin(ite);
            vv_ite != guiMesh.vv_end(ite); vv_ite++)
        {
            tempString += to_string(vv_ite->idx());
            tempString += " ";

        }

        //{constraint_vec_x, constraint_vec_y}
        for(OpnMesh::VertexOHalfedgeIter voh_ite = guiMesh.voh_begin(ite); voh_ite != guiMesh.voh_end(ite); voh_ite++)
        {
            QVector3D constraint = guiMesh.data(voh_ite).constraint();
            tempString += to_string(constraint.x()) + " " + to_string(constraint.y()) + " ";
        }


        //0..valency(not in use)
        for(unsigned int i = 0; i < valence; i++)
        {
            tempString += "0";
            tempString += " ";
        }

        //0(not in use): the number 0
        tempString += "0";
        //New point
        tempString += "\n";
    }

    //Faces:
    tempString += "\n";
	
    //Iterate thorugh faces.
	for(OpnMesh::FaceIter face_ite = guiMesh.faces_sbegin(); face_ite != guiMesh.faces_end(); face_ite++)
    {
        //Valence of face
		tempString += to_string(guiMesh.valence(face_ite)) + " ";

        for(OpnMesh::FaceVertexIter fv_ite = guiMesh.fv_begin(face_ite); fv_ite != guiMesh.fv_end(face_ite); fv_ite++)
        {
            tempString += to_string(fv_ite->idx()) + " ";
        }
        tempString += "\n";
    }
	


    // delete current mesh object and insert a new one
    delete subdMesh; // delete from heap
    subdMesh = new SbdvMesh();

    //Make a file, used for export to SubdMesh
	fstream file;
	file.open("test.off", fstream::out);
	file << tempString.c_str();
	file.close();
    qDebug() << "Mesh saved: test.off" << "Sucsess with loadV3?" << subdMesh->loadV3("test.off");
    subdMesh->build(); // build mesh topology from data

    if(saveFileOFF) //Save mesh to OFF before subdividing
    {
        subdMesh->save(qPrintable(location), subdivMesh::FileType::OFF);
    }

    subdivide();
    //std::remove("test.off");

    isQuadMesh();
}

MeshHandler *MeshHandler::oneStepSubdMesh()
{
    if(oneStepSubdMesh_ == nullptr) return nullptr;

    MeshHandler *subdivedMesh = new MeshHandler();
	
	const char* filename = "tempSubdividedMesh.off";
	fstream file;
	file.open(filename, fstream::out);

    oneStepSubdMesh_->build();
	oneStepSubdMesh_->save(filename, subdivMesh::OFF);

	subdivedMesh->importGuiMesh(QString::fromUtf8(filename));
	
    std::remove("tempSubdividedMesh.off");

	return subdivedMesh;

}

void MeshHandler::setSubdivisionSteps(int value)
{
    subdivisionSteps_ = value;
}

bool MeshHandler::isQuadMesh()
{
    for(OpnMesh::FaceIter face_ite = guiMesh.faces_begin(); face_ite != guiMesh.faces_end(); face_ite++)
    {
        if(guiMesh.valence(face_ite) != 4)
        {
            return false;
        }
    }
    return true;
}

bool MeshHandler::importGuiMesh(QString location, bool draw)
{
    //TODO: Discontinuity and gradient constraints.
    //TODO: If there alleready are vertices,edges and faces in guiMesh
	delete subdMesh; // delete from heap
	subdMesh = new SbdvMesh();
	qDebug() << "Sucsess with loadV3?" << subdMesh->loadV3(location.toStdString().c_str());
    subdMesh->build(); // build mesh topology from data

	vector <subdivMesh::MeshVertex>	&vertices = subdMesh->my_vertices;
	vector <subdivMesh::MeshFacet>  &facets = subdMesh->my_facets;

	for (int i = 0; i < vertices.size(); i++)
	{
		subdivMesh::Point_3D point3d = vertices.at(i).my_point;
		subdivMesh::Point_3D color3d = vertices.at(i).my_colour;
		double r, g, b;
		subdivMesh::LAB2RGB(color3d.getX(), color3d.getY(), color3d.getZ(), r, g, b);
		r *= 255; b *= 255; g *= 255;
		if (r > 255) r = 255;
		if (g > 255) g = 255;
		if (b > 255) b = 255;

		QPointF point2d(point3d.getX(), point3d.getY());
		QColor color(r, g, b);
		guiMesh.vertex_handle(addVertex(point2d, color));
	}

	for (int i = 0; i < facets.size(); ++i) {
		vector<vertexHandle> vertexIdx;

		vector<unsigned int> &vertIndices = facets.at(i).my_vertIndices;
		for (int j = 0; j < vertIndices.size(); ++j) {
			vertexIdx.push_back(guiMesh.vertex_handle(vertIndices.at(j)));
		}
		guiMesh.add_face(vertexIdx);
	}
    for (int i = 0; i < vertices.size(); i++)
    {
        QList<int> weights_ids = vertices.at(i).weight_ids;
        QList<subdivMesh::Point_3D> constraint = vertices.at(i).weights_vec;
        for (int j = 0; j < weights_ids.size(); ++j) {
            setConstraints(i, weights_ids.at(j),constraint.at(j));
        }
    }

    if(!draw)
    {
        delete subdMesh;
        subdMesh = nullptr;
    }

    return true;
}

void MeshHandler::garbageCollectOpenMesh()
{
    guiMesh.garbage_collection();
}

