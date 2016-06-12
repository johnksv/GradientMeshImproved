/*
 * These classes has been created by Jiri Kosinka for personal use
 * (and research on subdivision surfaces). It was stolen, with
 * permission, by Henrik Lieng for personal use (and research on
 * applications in subdivision surfaces).
*/

#ifndef MESH_H
#define MESH_H

using namespace std;

#include <vector>
#include <list>
#include <string>
#include <QOpenGLFunctions_1_0> // replace with opengl headers if not qt is used
#include "point_3d.h"
#include "spline.h"

// I ASSUME CLOCK-WISE ORIENTATION OF VERTICES WITHIN FACETS!

/*! Namespace of the in-house subdivision library.
 */
namespace subdivMesh {

#define CPSIZE 6.0f
#define SHOW_DERIVATIVE 1

enum FileType
{
    OFF, PLY, OFFW
};

class MeshFacet;

class MeshVertex
{
public:
    MeshVertex() {snapped=false;  }
   ~MeshVertex() {  }

    Point_3D 		my_point, my_visPoint,
                    my_Vpoi, my_VpoiColour, my_newW13, my_newW31, my_newW33,
                    my_W13, my_W31, my_W33, // wavelets
                    my_CClimit, // CC limit position
                    my_colour; //RGB colour

	unsigned int 	my_index,
                    my_valency;

    int             my_W13vid0, my_W13vid1, // wavelet vertex IDs
                    my_W31vid0, my_W31vid1,
                    my_newW13vid0, my_newW13vid1, // new wavelet vertex IDs
                    my_newW31vid0, my_newW31vid1;

	unsigned int 	my_i, // for mesh to rays conversion
					my_j,
                    my_k,
                    my_W13sharpness, my_W31sharpness, my_W33sharpness,
                    my_newW13sharpness, my_newW31sharpness, my_newW33sharpness;

    bool			isOnBoundary,
                    isFeature,
                    isSelected,
                    isVertP1,
                    isStable,
                    snapped;

    QList<float>    weights;
    QList<Point_3D> weights_vec;
    QList<int>      weight_ids, sharpness;

	std::vector <unsigned int> my_faceIndices;
	std::vector <MeshFacet*>   my_facets;

    std::vector<Point_3D>      my_fakeD, my_fakeDColour; // fake Ds for Int. C-C
    std::vector<unsigned int>  my_n;     // incident facet valencies - 3

	PointPrec		my_curvM,
					my_curvG,
					my_curvMsmooth,
					my_curvGsmooth;

	GLfloat			my_normalFlat[3],
					my_normalSmooth[3];

	void coutV(void);
};

class MeshCorner
{
public:
    MeshCorner() { my_alreadyIn = false; }
   ~MeshCorner() {  }

    unsigned int	 my_index;
    unsigned int	 my_vIndex;
    unsigned int	 my_nIndex;
    MeshVertex 		*my_vertex;
    MeshCorner		*my_nextCorner, *my_pairedCorner;
    MeshFacet		*my_facet;
    MeshFacet		*my_nextFacet;
    unsigned int     my_nextFshift; // index shift in next face to get the same-1 vertex

    Point_3D		 my_Epoi, my_EpoiColour, my_newW13, my_newW31, my_newW33,
                     my_edgeContr, my_edgeContrColour, // for Int. C-C
                     my_Epoi0, my_Epoi1, my_Epoi0Colour, my_Epoi1Colour, my_Fpoi, my_FpoiColour; // for ternary

    bool			my_alreadyIn;
    unsigned int	my_newEindex, //index of the new edge-vertex
                    my_newWvid0, my_newWvid1,
                    my_newE0index, my_newE1index, my_newFindex; // for ternary

    unsigned int    my_posi, my_posj, my_posa, my_posb,
                    my_W13sharpness, my_W31sharpness, my_W33sharpness;

    int             my_sharpness, my_newID;
};

class MeshFacet
{
public:
    MeshFacet() { drawMe = true; }
   ~MeshFacet() { ; }

	unsigned int 	my_index,
                    my_valency,
                    my_lifeVal;

    std::vector <unsigned int> my_vertIndices;
	std::vector <MeshCorner>   my_corners;
    std::vector <MeshFacet* >  my_lifeFacets;

    std::vector<Point_3D>      my_faceContr, my_faceContrColour; // facet contributions from vertices

	PointPrec		my_curvM,
					my_curvG,
					my_curvMsmooth,
					my_curvGsmooth;

	GLfloat			my_normalFlat[3],
					my_normalSmooth[3];

    Point_3D		my_Fpoi, my_FpoiColour;

    bool			drawMe;

	void coutF(void);
};

class MeshRay
{
public:
    MeshRay() { ; }
   ~MeshRay() { ; }

	unsigned int			my_index;
	std::vector <SplineC>	my_crossCurves;
};

class Mesh
{
public:
    Mesh() {lifeIsOn = false; withWavelets = true; my_numV = my_numF = 0; interpFlat = false; useRGB = false;v3=false;}
   ~Mesh() { DeleteData(); }
	void DeleteData	(void);
    void newNC		(unsigned int size, unsigned int deg, unsigned int val);
    bool load(const char *fileName, bool _xyWeights=false);
    bool loadV2(const char *fileName, bool _xyWeights=false);
    bool loadV3(const char *fileName, bool _xyWeights=false);
    bool loadJ(const char *fileName, bool _xyWeights, float height, float width, const int X, const int Y);
    bool loadJiri(const char *fileName, float width, float height, const int X, const int Y);
    void load       (vector<vector<float> > *vertices, vector<vector<int> > *faces, bool _xyWeights);
    void addWeight  (QList<float> *weights,QList<int> *ids,int index);
    void save		(const char *fileName, FileType ftype);
	void transf		(void);
	void build		(void);
    bool removeLowVal (void);
	void compCurv	(void);
	void compCurvSmooth(unsigned int rings);
    void compCurvG	(void);
    void flipColourSpace();
    void convert2v3();
    void spreadOrgCol1();
    void spreadOrgCol2();
    void freeSelection();
    void increaseWeights();
    void decreaseWeights();
    void increaseLocalWeights();
    void decreaseLocalWeights();
    void snapSelected();
    void changeSharpness(int change);
    void draw(QOpenGLFunctions_1_0 *context);
 //   void drawCircle(float cx, float cy, float r, int num_segments, float *colour);
 //   void drawGrid();
//    void drawVertices(QList<int> *vertices);
    bool insideMesh(int x,int y);
    bool insideFace(unsigned int i, int x,int y);
    void addEdge(vector<int> *edge, int id1, int id2);
    void getRightCoords(vector<double> *gtCoords, vector<double> *gnCoords, vector<int> *edge,
                        double *gtRight, double *gnRight, int index, int target);
    void getLeftCoords(vector<double> *gtCoords, vector<double> *gnCoords, vector<int> *edge,
                              double *gtLeft, double *gnLeft, int index, int target);

	Point_3D getPoi(int i, int j, int k);

	void generateRays(Mesh *mesh);
    void generateCtrlMeshFromRays();
	void raysToMesh(void);

	void triangulate(void); 	//split each face into triangles wrt. one vertex
	void triangulate2(void);	//split each face into triangles wrt. facet centroid

	void computeNormalsFlat(void);
	void computeNormalsSmooth(unsigned int rings);

	void calculateNormal(GLfloat *norm, GLfloat p0[3],
						 GLfloat p1[3], GLfloat p2[3]);

	void getRingsF	(MeshFacet *fac, unsigned int r,
					 std::vector< std::vector <unsigned int> > *ring_facets,
					 std::vector< std::vector <unsigned int> > *ring_vertices);

	void getRingsV	(MeshVertex *vert, unsigned int r,
					 std::vector< std::vector <unsigned int> > *ring_facets,
					 std::vector< std::vector <unsigned int> > *ring_vertices);

    void CatmullClark(Mesh *mesh);
    void CatmullClarkColour(Mesh *mesh);
    void CatmullClarkIntFlat(Mesh *mesh);
    void CatmullClarkLimit(void);
    void CatmullClarkBounded(Mesh *mesh);
    void CatmullClarkInt(Mesh *mesh);
    void CatmullClarkIntColour(Mesh *mesh);
    void Conformal(Mesh *mesh);
    void LaplacianSubdiv(Mesh *smesh, int lapSmvalue);
    void WaveletSubdiv(Mesh *mesh);
    float getWeight(int vIndex, int target);
    Point_3D* getWeightVec(int vIndex, int target);
    int getSharpness(int vIndex, int target);
    void setWeightId(int vIndex, int target, int value);
    void LinearTernarySubdiv(Mesh *smesh);

    void LaplacianSmooth(int lapSmvalue);

    void LinearSubdiv(Mesh *mesh);
    void computeWeights(PointPrec x[], PointPrec y[], PointPrec weights[]);

    void inverseLimitMesh(Mesh *mesh);
    void inverseLimitMeshColour(Mesh *mesh);

    void lifeInit(void);
    void lifeStep(void);

    void times(const char *which);

    bool isLshape(MeshVertex *v);

    unsigned int decideSharpness(unsigned int s1, unsigned int s2, unsigned int s3, unsigned int s4);
    unsigned int decideSharpness(unsigned int s1, unsigned int s2);

    MeshCorner* getMainCrn(MeshCorner *crn);

    MeshCorner* getNextCorner(MeshCorner *corner);
    MeshCorner* getSameCorner(MeshCorner *corner);
    MeshCorner* getPrevCorner(MeshCorner *corner);
    MeshCorner* findCorner(MeshVertex *vertex, MeshFacet *facet);

    unsigned int my_numV, my_numF, my_numE, my_numW;

	int my_degree;

    bool xyWeights;

	unsigned int 	my_NCdegree;
	unsigned int 	my_NCvalency;
	Mults			my_NCmults, my_NCsizes;
	unsigned int	my_maxMult;

	unsigned int	my_level;
	std::string		my_s,
					my_save;

	unsigned int	my_rand;

	const char 		*my_file;

	PointPrec		my_scale;
	Point_3D		my_centre;

	Point_3D		my_EV,
					my_newEV;

	CtrlPlg			my_EF,
					my_newEF;

	PointPrec		my_minM,
					my_maxM,
					my_minG,
                    my_maxG,
                    my_minz,
                    my_maxz;

	bool			rever,
                    transform,
                    lifeIsOn,
                    interpFlat, v3;

    bool            quadMesh, hasBoundary;

	std::vector< std::vector< std::vector < unsigned int > > >	map;

	std::vector <MeshVertex>	my_vertices;
	std::vector <MeshFacet>  	my_facets;
	std::vector <MeshRay>		my_rays;
	std::vector <MeshRay>		my_insRays;
	std::vector <MeshRay>		my_insMaxRays;

    std::vector <MeshCorner*>	my_boundaryCorners, my_creaseCorners;

    std::vector<unsigned int>   my_sFacets, my_sVertices, my_sValues, my_vertP1; // for sharp creases

    static const float          ccW[], ccA[], ccB[], ccC[],
                                cfA[], cfB[], cfC[];
    bool        withWavelets;
    bool        isFootBall;
    bool        useRGB;

};

} // end of namespace subdivMesh

#endif
