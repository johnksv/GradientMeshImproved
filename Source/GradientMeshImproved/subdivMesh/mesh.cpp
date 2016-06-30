#include <fstream>
#include "mesh.h"
#include "utils.h"
#include <set>
#include <cmath>
#include <complex>
#include <stdlib.h>
#include <assert.h>
#include <algorithm> // for reverse();
#include <string>
#include <omp.h>
#include <time.h>
#include <iostream>
#include <string.h>

using namespace subdivMesh;

const float Mesh::ccW[] = {1.236068,
                           1.000000,
                           0.718502,
                           0.522333,
                           0.391842,
                           0.303196,
                           0.240967,
                           0.195860,
                           0.162219,
                           0.136502};

const float Mesh::ccA[] = {0.170033,
                           0.25,
                           0.344048,
                           0.440062,
                           0.527227,
                           0.601358,
                           0.662508,
                           0.712380,
                           0.753001,
                           0.786215};

const float Mesh::ccB[] = {0.751865,
                           1.0,
                           1.340796,
                           1.677230,
                           1.966112,
                           2.200535,
                           2.387118,
                           2.535252,
                           2.653472,
                           2.748627};

const float Mesh::ccC[] = {0.078102,
                           -0.25,
                           -0.684845,
                           -1.117293,
                           -1.493339,
                           -1.801893,
                           -2.049626,
                           -2.247632,
                           -2.406473,
                           -2.534842};

const float Mesh::cfA[] = {
0.1705915019,
0.2500000000,
0.3311781613,
0.4080232162,
0.4760015040,
0.5341689770,
0.5833204647,
0.6247738911,
0.6598517896,
0.6897079362
};

const float Mesh::cfB[] = {
0.7983274886,
1.000000000,
1.117391355,
1.158270260,
1.153490258,
1.124906440,
1.085157838,
1.041129368,
0.9964663888,
0.9530383823
};

const float Mesh::cfC[] = {
0.03108100976,
-0.2500000000,
-0.4485695173,
-0.5662934757,
-0.6294917607,
-0.6590754169,
-0.6684783030,
-0.6659032597,
-0.6563181787,
-0.6427463185
};

//const float Mesh::sA[] = {

//};

//const float Mesh::sB[] = {

//};

//const float Mesh::sC[] = {

//};

using namespace std;

void MeshVertex::coutV(void)
{
	unsigned int i;

    cout << "-----------------------------" << endl;
	cout << "Index: " << my_index << ", Val: " << my_valency << ": ";
	for (i = 0 ; i < my_valency ; i++)
	{
		cout << my_faceIndices[i] << " ";
	}
    cout << endl;
    cout << "x, y, z: " << my_point;
    cout << "k, i, j: " << my_k << " " << my_i << " " << my_j << endl;
    cout << "CurvMfl: " << my_curvM << ", CurvGfl: " << my_curvG << endl;
    cout << "CurvMsm: " << my_curvMsmooth << ", CurvGsm: " << my_curvGsmooth << endl;
    cout << "W13 indices " << my_W13vid0 << " " << my_W13vid1 << endl;
    cout << "W31 indices " << my_W31vid0 << " " << my_W31vid1 << endl;
}

void MeshFacet::coutF(void)
{
	unsigned int i;

	cout << my_index << " " << my_valency << " ";
	for (i = 0 ; i < my_valency ; i++)
	{
		cout << my_vertIndices[i] << " ";
	}
    cout << endl;
}

void Mesh::DeleteData()
{
    unsigned int    i;

    map.clear();
    my_vertices.clear();

    // Is it necessary to delete corners of facets?
    for (i = 0 ; i < my_facets.size() ; i++)
    {
        my_facets[i].my_corners.clear();
    }

    my_facets.clear();
//    my_subdFacets.clear();
    my_rays.clear();
    my_insRays.clear();
    my_insMaxRays.clear();

    my_boundaryCorners.clear();

    my_numE = my_numF = my_numV = 0;

//    my_EVlist.clear();


//    cout << "Mesh data deleted!" << endl;
}

void Mesh::newNC(unsigned int size, unsigned int deg, unsigned int val)
{
    unsigned int	i;
    std::stringstream 	sstm, sstmsave;

    my_level = 0;
    my_NCdegree  = deg;
    my_NCvalency = val;
    my_NCmults.clear();
    for (i = 0 ; i < my_NCvalency ; i++)
    {
        my_NCmults.push_back(1);

    }
    for (i = 0 ; i < my_NCvalency ; i++)
    {
        my_NCsizes.push_back(size);
    }
    my_maxMult = my_NCdegree + 1;

    sstm << "Deg: " << my_NCdegree << "; Val: " << my_NCvalency << "; EP: " << my_NCmults[0];
    sstmsave << my_NCdegree << "-" << my_NCvalency << "_" << my_NCmults[0];
    for (i = 1 ; i < my_NCvalency ; i++)
    {
        sstm << "*" << my_NCmults[i];
        sstmsave << "-" << my_NCmults[i];
    }
    my_s = sstm.str();
    my_save = sstmsave.str();
}

bool Mesh::load(const char *fileName, bool _xyWeights)
{
    unsigned int	i, j, vn, n, f, v, s;
	PointPrec		x, y, z;
    std::string		file_type, tmp;
	MeshVertex 		vertex;
	MeshFacet 		facet;
	ifstream 		file (fileName);

    my_level = 0;
    xyWeights = _xyWeights;
    v3 = false;

	if(!file.is_open())
	{
        return false;
	}
	else
	{
		my_level = 0;

		file >> file_type;

        if (file_type == "OFF" || file_type == "off" || file_type == "Off")
        {
            file >> my_numV >> my_numF >> my_numE;
            my_numW = 0;

        }
        else if (file_type == "PLY" || file_type == "ply" || file_type == "Ply")
        {
            do
            {
                file >> tmp;
            }
            while (tmp != "format");
            file >> tmp;
            if (tmp != "ascii")
            {
                cout << "Cannot read binary files!!!" << endl;
                assert(false);
            }

            do
            {
                file >> tmp;
            }
            while (tmp != "vertex");

            file >> my_numV;

            do
            {
                file >> tmp;
            }
            while (tmp != "face");

            file >> my_numF;

            do
            {
                file >> tmp;
            }
            while (tmp != "end_header");
            my_numE = 0;
            my_numW = 0;
        }
        else if (file_type == "OFFW" || file_type == "offw" || file_type == "Offw")
        {
            file >> my_numV >> my_numF >> my_numW;
            my_numE = 0;
        }
        else
        {
            return false;
        }

        if (my_numV == 60 && my_numF == 32)
        {
            isFootBall = true;
        }
        else
        {
            isFootBall = false;
        }

        float R, G, B, w;
        int wid,val;
		// read vertex coordinates
		for (i = 0 ; i < my_numV ; i++)
		{
            file >> x >> y >> z >> R >> G >> B >> val;
            if(xyWeights) {
                vertex.my_point.setX(x*z);
                vertex.my_point.setY(y*z);
            } else {
                vertex.my_point.setX(x);
                vertex.my_point.setY(y);
            }
            vertex.my_point.setZ(z);

            vertex.isStable = false;
            vertex.isSelected = false;
            vertex.my_index = i;
            vertex.isFeature = true;
            //set all wavelet neighbourhood indices to -1
            vertex.my_W13vid0 = -1;
            vertex.my_W13vid1 = -1;
            vertex.my_W31vid0 = -1;
            vertex.my_W31vid1 = -1;

            vertex.my_W13sharpness = 0;
            vertex.my_W31sharpness = 0;
            vertex.my_W33sharpness = 0;
            vertex.snapped = false;

            vertex.my_newW13vid0 = -1;
            vertex.my_newW13vid1 = -1;
            vertex.my_newW31vid0 = -1;
            vertex.my_newW31vid1 = -1;

            vertex.my_colour.setX(R);
            vertex.my_colour.setY(G);
            vertex.my_colour.setZ(B);

            vertex.weight_ids.clear();
            vertex.weights.clear();
            for(j = 0; j < val; j++) {
                file >> wid;
                vertex.weight_ids.append(wid);
            }
            for(j = 0; j < val; j++) {
                file >> w;
                vertex.weights.append(w);
            }

			my_vertices.push_back(vertex);
			my_vertices[i].my_faceIndices.clear();
		}

		// read facets
        quadMesh = true;
		for (i = 0 ; i < my_numF ; i++)
		{
			facet.my_index = i;
			facet.my_vertIndices.clear();
			file >> n;
			facet.my_valency = n;
            if (n != 4)
            {
                quadMesh = false;
            }
			for (j = 0 ; j < n ; j++)
			{
				file >> vn;
				facet.my_vertIndices.push_back(vn);
				my_vertices[vn].my_faceIndices.push_back(i);
			}

            my_facets.push_back(facet);
		}

        my_sFacets.clear();
        my_sVertices.clear();
        my_sValues.clear();
        // read sharp edges
        for (i = 0 ; i < my_numE ; i++)
        {
            file >> f >> v >> s;
            my_sFacets.push_back(f);
            my_sVertices.push_back(v);
            my_sValues.push_back(s);
        }

		my_s = fileName;
		my_save = fileName;

        build();

	} //end of else

	file.close();
    return true;
}

bool Mesh::loadJ(const char *fileName, bool _xyWeights, float height, float width, const int X, const int Y)
{
    unsigned int	i, j, vn, n, f, v, s;
    PointPrec		x, y, z;
    std::string		file_type, tmp;
    MeshVertex 		vertex;
    MeshFacet 		facet;
    ifstream 		file (fileName);

    my_level = 0;
    xyWeights = _xyWeights;
    v3 = false;

    if(!file.is_open())
    {
        return false;
    }
    else
    {
        my_level = 0;

        file >> file_type;

        if (file_type == "OFF" || file_type == "off" || file_type == "Off")
        {
            file >> my_numV >> my_numF >> my_numE;
            my_numW = 0;

        }
        else if (file_type == "PLY" || file_type == "ply" || file_type == "Ply")
        {
            do
            {
                file >> tmp;
            }
            while (tmp != "format");
            file >> tmp;
            if (tmp != "ascii")
            {
                cout << "Cannot read binary files!!!" << endl;
                assert(false);
            }

            do
            {
                file >> tmp;
            }
            while (tmp != "vertex");

            file >> my_numV;

            do
            {
                file >> tmp;
            }
            while (tmp != "face");

            file >> my_numF;

            do
            {
                file >> tmp;
            }
            while (tmp != "end_header");
            my_numE = 0;
            my_numW = 0;
        }
        else if (file_type == "OFFW" || file_type == "offw" || file_type == "Offw")
        {
            file >> my_numV >> my_numF >> my_numW;
            my_numE = 0;
        }
        else
        {
            return false;
        }

        if (my_numV == 60 && my_numF == 32)
        {
            isFootBall = true;
        }
        else
        {
            isFootBall = false;
        }

        float R, G, B;
        // read vertex coordinates
        for (i = 0 ; i < my_numV ; i++)
        {
            file >> x >> y >> z;
            x = x*width + (X-width)/2;
            y = y*height + (Y-height)/2;
            R = 0.2; G = 0.8; B = 0.2;
            getline(file, tmp);
            if(xyWeights) {
                vertex.my_point.setX(x*z);
                vertex.my_point.setY(y*z);
            }
            else {
                vertex.my_point.setX(x);
                vertex.my_point.setY(y);
            }
            vertex.my_point.setZ(z);
            vertex.isStable = false;
            vertex.isSelected = false;
            vertex.my_index = i;
            vertex.isFeature = true;
            //set all wavelet neighbourhood indices to -1
            vertex.my_W13vid0 = -1;
            vertex.my_W13vid1 = -1;
            vertex.my_W31vid0 = -1;
            vertex.my_W31vid1 = -1;
            vertex.snapped = false;

            vertex.my_W13sharpness = 0;
            vertex.my_W31sharpness = 0;
            vertex.my_W33sharpness = 0;

            vertex.my_newW13vid0 = -1;
            vertex.my_newW13vid1 = -1;
            vertex.my_newW31vid0 = -1;
            vertex.my_newW31vid1 = -1;

            vertex.my_colour.setX(R);
            vertex.my_colour.setY(G);
            vertex.my_colour.setZ(B);

            my_vertices.push_back(vertex);
            my_vertices[i].my_faceIndices.clear();
        }

        // read facets
        quadMesh = true;
        for (i = 0 ; i < my_numF ; i++)
        {
            facet.my_index = i;
            facet.my_vertIndices.clear();
            file >> n;
            facet.my_valency = n;
            if (n != 4)
            {
                quadMesh = false;
            }
            for (j = 0 ; j < n ; j++)
            {
                file >> vn;
                facet.my_vertIndices.push_back(vn);
                my_vertices[vn].my_faceIndices.push_back(i);
            }
            my_facets.push_back(facet);
        }

        for (i = 0 ; i < my_numF ; i++)
        {
            for (j = 0 ; j < my_facets[i].my_valency; j++)
            {
                if(j<n-1) {
                    addWeight(&my_vertices[my_facets[i].my_vertIndices[j]].weights,
                            &my_vertices[my_facets[i].my_vertIndices[j]].weight_ids,my_facets[i].my_vertIndices[j+1]);
                    addWeight(&my_vertices[my_facets[i].my_vertIndices[j+1]].weights,
                            &my_vertices[my_facets[i].my_vertIndices[j+1]].weight_ids,my_facets[i].my_vertIndices[j]);
                } else {
                    addWeight(&my_vertices[my_facets[i].my_vertIndices[j]].weights,
                            &my_vertices[my_facets[i].my_vertIndices[j]].weight_ids,my_facets[i].my_vertIndices[0]);
                    addWeight(&my_vertices[my_facets[i].my_vertIndices[0]].weights,
                            &my_vertices[my_facets[i].my_vertIndices[0]].weight_ids,my_facets[i].my_vertIndices[j]);
                }
            }
        }

        my_sFacets.clear();
        my_sVertices.clear();
        my_sValues.clear();
        // read sharp edges
        for (i = 0 ; i < my_numE ; i++)
        {
            file >> f >> v >> s;
            my_sFacets.push_back(f);
            my_sVertices.push_back(v);
            my_sValues.push_back(s);
        }

        my_s = fileName;
        my_save = fileName;

        build();

    } //end of else

    file.close();
    return true;
}

bool Mesh::loadJiri(const char *fileName, float width, float height, const int X, const int Y)
{
    unsigned int	i, j, vn, n, f, v, s;
    PointPrec		x, y, z;
    std::string		file_type, tmp;
    MeshVertex 		vertex;
    MeshFacet 		facet;
    ifstream 		file (fileName);

    xyWeights = true; // true: 2D; false: 3D

    my_level = 0;
    v3 = false;

    if(!file.is_open())
    {
        return false;
    }
    else
    {
        my_level = 0;

        file >> file_type;

        if (file_type == "OFF" || file_type == "off" || file_type == "Off")
        {
            file >> my_numV >> my_numF >> my_numE;
            my_numW = 0;

        }
        else if (file_type == "PLY" || file_type == "ply" || file_type == "Ply")
        {
            do
            {
                file >> tmp;
            }
            while (tmp != "format");
            file >> tmp;
            if (tmp != "ascii")
            {
                cout << "Cannot read binary files!!!" << endl;
                assert(false);
            }

            do
            {
                file >> tmp;
            }
            while (tmp != "vertex");

            file >> my_numV;

            do
            {
                file >> tmp;
            }
            while (tmp != "face");

            file >> my_numF;

            do
            {
                file >> tmp;
            }
            while (tmp != "end_header");
            my_numE = 0;
            my_numW = 0;
        }
        else if (file_type == "OFFW" || file_type == "offw" || file_type == "Offw")
        {
            file >> my_numV >> my_numF >> my_numW;
            my_numE = 0;
        }
        else
        {
            return false;
        }

        if (my_numV == 60 && my_numF == 32)
        {
            isFootBall = true;
        }
        else
        {
            isFootBall = false;
        }

        float R, G, B;
        float xmin=X*X,xmax=-X*X,ymin=Y*Y,ymax=-Y*Y;
        // read vertex coordinates
        for (i = 0 ; i < my_numV ; i++)
        {
            file >> x >> y >> z;
            R = 0.2; G = 0.8; B = 0.2;
            getline(file, tmp);
            vertex.my_point.setX(x);
            vertex.my_point.setY(y);
            if(xyWeights)
                vertex.my_point.setZ(1.0f);
            else
                vertex.my_point.setZ(z);

            if(x<xmin) xmin = x;
            if(x>xmax) xmax = x;
            if(y<ymin) ymin = y;
            if(y>ymax) ymax = y;

            vertex.isStable = false;
            vertex.isSelected = false;
            vertex.my_index = i;
            vertex.isFeature = true;

            //set all wavelet neighbourhood indices to -1
            vertex.my_W13vid0 = -1;
            vertex.my_W13vid1 = -1;
            vertex.my_W31vid0 = -1;
            vertex.my_W31vid1 = -1;

            vertex.my_W13sharpness = 0;
            vertex.my_W31sharpness = 0;
            vertex.my_W33sharpness = 0;
            vertex.snapped = false;

            vertex.my_newW13vid0 = -1;
            vertex.my_newW13vid1 = -1;
            vertex.my_newW31vid0 = -1;
            vertex.my_newW31vid1 = -1;

            vertex.my_colour.setX(R);
            vertex.my_colour.setY(G);
            vertex.my_colour.setZ(B);

            my_vertices.push_back(vertex);
            my_vertices[i].my_faceIndices.clear();
        }

        // transform into coordinate system
        for(i = 0; i < my_numV && xyWeights; i++) {
            x = (my_vertices[i].my_point.getX()-xmin)/(xmax-xmin);
            y = (my_vertices[i].my_point.getY()-ymin)/(ymax-ymin);
            x = x*width + (X-width)/2;
            y = y*height + (Y-height)/2;
            my_vertices[i].my_point.setX(x);
            my_vertices[i].my_point.setY(y);
        }

        // read facets
        quadMesh = true;
        for (i = 0 ; i < my_numF ; i++)
        {
            facet.my_index = i;
            facet.my_vertIndices.clear();
            file >> n;
            facet.my_valency = n;
            if (n != 4)
            {
                quadMesh = false;
            }
            for (j = 0 ; j < n ; j++)
            {
                file >> vn;
                facet.my_vertIndices.push_back(vn);
                my_vertices[vn].my_faceIndices.push_back(i);
            }
            my_facets.push_back(facet);
        }

        for (i = 0 ; i < my_numF ; i++)
        {
            for (j = 0 ; j < my_facets[i].my_valency; j++)
            {
                if(j<my_facets[i].my_valency-1) {
                    addWeight(&my_vertices[my_facets[i].my_vertIndices[j]].weights,
                            &my_vertices[my_facets[i].my_vertIndices[j]].weight_ids,my_facets[i].my_vertIndices[j+1]);
                    addWeight(&my_vertices[my_facets[i].my_vertIndices[j+1]].weights,
                            &my_vertices[my_facets[i].my_vertIndices[j+1]].weight_ids,my_facets[i].my_vertIndices[j]);
                } else {
                    addWeight(&my_vertices[my_facets[i].my_vertIndices[j]].weights,
                            &my_vertices[my_facets[i].my_vertIndices[j]].weight_ids,my_facets[i].my_vertIndices[0]);
                    addWeight(&my_vertices[my_facets[i].my_vertIndices[0]].weights,
                            &my_vertices[my_facets[i].my_vertIndices[0]].weight_ids,my_facets[i].my_vertIndices[j]);
                }
            }
        }

        my_sFacets.clear();
        my_sVertices.clear();
        my_sValues.clear();
        // read sharp edges
        for (i = 0 ; i < my_numE ; i++)
        {
            file >> f >> v >> s;
            my_sFacets.push_back(f);
            my_sVertices.push_back(v);
            my_sValues.push_back(s);
        }

        my_s = fileName;
        my_save = fileName;

        build();

    } //end of else

    file.close();
    return true;
}

void Mesh::load(vector<vector<float> > *vertices, vector<vector<int> > *faces, bool _xyWeights)
{
    my_level = 0;
    xyWeights = _xyWeights;
    my_numV = vertices->size();
    double L=0,a=0,b=0;
    v3 = false;
    // read vertex coordinates
    for (unsigned int i = 0 ; i < my_numV ; i++)
    {
        MeshVertex vertex;
        float w = vertices->at(i)[5];
        if(xyWeights) {
            vertex.my_point.setX(vertices->at(i)[0]*w);
            vertex.my_point.setY(vertices->at(i)[1]*w);
        } else {
            vertex.my_point.setX(vertices->at(i)[0]);
            vertex.my_point.setY(vertices->at(i)[1]);
        }
        vertex.my_point.setZ(w);
        vertex.isStable = false;
        vertex.isSelected = false;
        vertex.my_index = i;
        vertex.isFeature = true;
        //set all wavelet neighbourhood indices to -1
        vertex.my_W13vid0 = -1;
        vertex.my_W13vid1 = -1;
        vertex.my_W31vid0 = -1;
        vertex.my_W31vid1 = -1;
        vertex.snapped = false;

        vertex.my_W13sharpness = 0;
        vertex.my_W31sharpness = 0;
        vertex.my_W33sharpness = 0;

        vertex.my_newW13vid0 = -1;
        vertex.my_newW13vid1 = -1;
        vertex.my_newW31vid0 = -1;
        vertex.my_newW31vid1 = -1;

        vertex.my_colour.setX(vertices->at(i)[2]);
        vertex.my_colour.setY(vertices->at(i)[3]);
        vertex.my_colour.setZ(vertices->at(i)[4]);

        RGB2LAB(vertex.my_colour.getX(),vertex.my_colour.getY(),vertex.my_colour.getZ(),L,a,b);
        vertex.my_colour.setX(L);
        vertex.my_colour.setY(a);
        vertex.my_colour.setZ(b);

        my_vertices.push_back(vertex);
        my_vertices[i].my_faceIndices.clear();
    }

    // read facets and set up weights
    quadMesh = true;
    my_numF = faces->size();
    for (unsigned int i = 0 ; i < my_numF ; i++)
    {
        MeshFacet facet;
        facet.my_index = i;
        facet.my_vertIndices.clear();
        int n = faces->at(i).size();
        facet.my_valency = n;
        if (n != 4)
        {
            quadMesh = false;
        }
        for (int j = 0 ; j < n ; j++)
        {
            facet.my_vertIndices.push_back(faces->at(i)[j]);
            my_vertices[faces->at(i)[j]].my_faceIndices.push_back(i);
            if(j<n-1) {
                addWeight(&my_vertices[faces->at(i)[j]].weights,
                        &my_vertices[faces->at(i)[j]].weight_ids,faces->at(i)[j+1]);
                addWeight(&my_vertices[faces->at(i)[j+1]].weights,
                        &my_vertices[faces->at(i)[j+1]].weight_ids,faces->at(i)[j]);
            } else {
                addWeight(&my_vertices[faces->at(i)[j]].weights,
                        &my_vertices[faces->at(i)[j]].weight_ids,faces->at(i)[0]);
                addWeight(&my_vertices[faces->at(i)[0]].weights,
                        &my_vertices[faces->at(i)[0]].weight_ids,faces->at(i)[j]);
            }
        }
        my_facets.push_back(facet);
    }
}

bool Mesh::loadV2(const char *fileName, bool _xyWeights)
{
    unsigned int	i, j, vn, n, f, v, s;
    PointPrec		x, y, z;
    std::string		file_type, tmp;
    MeshVertex 		vertex;
    MeshFacet 		facet;
    ifstream 		file (fileName);

    my_level = 0;
    xyWeights = _xyWeights;
    v3 = false;

    if(!file.is_open())
    {
        return false;
    }
    else
    {
        my_level = 0;

        file >> file_type;

        if (file_type == "OFF" || file_type == "off" || file_type == "Off")
        {
            file >> my_numV >> my_numF >> my_numE;
            my_numW = 0;

        }
        else if (file_type == "PLY" || file_type == "ply" || file_type == "Ply")
        {
            do
            {
                file >> tmp;
            }
            while (tmp != "format");
            file >> tmp;
            if (tmp != "ascii")
            {
                cout << "Cannot read binary files!!!" << endl;
                assert(false);
            }

            do
            {
                file >> tmp;
            }
            while (tmp != "vertex");

            file >> my_numV;

            do
            {
                file >> tmp;
            }
            while (tmp != "face");

            file >> my_numF;

            do
            {
                file >> tmp;
            }
            while (tmp != "end_header");
            my_numE = 0;
            my_numW = 0;
        }
        else if (file_type == "OFFW" || file_type == "offw" || file_type == "Offw")
        {
            file >> my_numV >> my_numF >> my_numW;
            my_numE = 0;
        }
        else
        {
            return false;
        }

        if (my_numV == 60 && my_numF == 32)
        {
            isFootBall = true;
        }
        else
        {
            isFootBall = false;
        }

        float R, G, B, w;
        int wid,sharp;
        unsigned int val;
        bool snapped;
        // read vertex coordinates
        for (i = 0 ; i < my_numV ; i++)
        {
            file >> x >> y >> z >> R >> G >> B >> val;
            if(xyWeights) {
                vertex.my_point.setX(x*z);
                vertex.my_point.setY(y*z);
            } else {
                vertex.my_point.setX(x);
                vertex.my_point.setY(y);
            }
            vertex.my_point.setZ(z);

            vertex.isStable = false;
            vertex.isSelected = false;
            vertex.my_index = i;
            vertex.isFeature = true;
            //set all wavelet neighbourhood indices to -1
            vertex.my_W13vid0 = -1;
            vertex.my_W13vid1 = -1;
            vertex.my_W31vid0 = -1;
            vertex.my_W31vid1 = -1;

            vertex.my_W13sharpness = 0;
            vertex.my_W31sharpness = 0;
            vertex.my_W33sharpness = 0;

            vertex.my_newW13vid0 = -1;
            vertex.my_newW13vid1 = -1;
            vertex.my_newW31vid0 = -1;
            vertex.my_newW31vid1 = -1;

            vertex.my_colour.setX(R);
            vertex.my_colour.setY(G);
            vertex.my_colour.setZ(B);

            vertex.weight_ids.clear();
            vertex.weights.clear();
            vertex.sharpness.clear();
            for(j = 0; j < val; j++) {
                file >> wid;
                vertex.weight_ids.append(wid);
            }
            for(j = 0; j < val; j++) {
                file >> w;
                vertex.weights.append(w);
            }
            for(j = 0; j < val; j++) {
                file >> sharp;
                vertex.sharpness.append(sharp);
            }
            file >> snapped;
            vertex.snapped = snapped;
            if(snapped) {
                file >> x >> y;
                vertex.my_visPoint.setX(x);
                vertex.my_visPoint.setY(y);
            }

            my_vertices.push_back(vertex);
            my_vertices[i].my_faceIndices.clear();
        }

        // read facets
        quadMesh = true;
        for (i = 0 ; i < my_numF ; i++)
        {
            facet.my_index = i;
            facet.my_vertIndices.clear();
            file >> n;
            facet.my_valency = n;
            if (n != 4)
            {
                quadMesh = false;
            }
            for (j = 0 ; j < n ; j++)
            {
                file >> vn;
                facet.my_vertIndices.push_back(vn);
                my_vertices[vn].my_faceIndices.push_back(i);
            }

            my_facets.push_back(facet);
        }

        my_sFacets.clear();
        my_sVertices.clear();
        my_sValues.clear();
        // read sharp edges
        for (i = 0 ; i < my_numE ; i++)
        {
            file >> f >> v >> s;
            my_sFacets.push_back(f);
            my_sVertices.push_back(v);
            my_sValues.push_back(s);
        }

        my_s = fileName;
        my_save = fileName;

        build();

    } //end of else

    file.close();
    return true;
}

bool Mesh::loadV3(const char *fileName, bool _xyWeights)
{
    ifstream file (fileName);
    stringstream strStream;

    strStream << file.rdbuf();
    loadV3(strStream, _xyWeights);

    file.close();
    return true;
}

bool Mesh::loadV3(stringstream &strStream, bool _xyWeights)
{
    unsigned int	i, j, vn, n, f, v, s;
    PointPrec		x, y, z;
    std::string		file_type, tmp;
    MeshVertex 		vertex;
    MeshFacet 		facet;

    my_level = 0;
    xyWeights = _xyWeights;
    v3 = true;

    if(strStream.bad())
    {
        return false;
    }
    else
    {
        my_level = 0;

        strStream >> file_type;

        if (file_type == "OFF" || file_type == "off" || file_type == "Off")
        {
            strStream >> my_numV >> my_numF >> my_numE;
            useRGB = my_numE;
            my_numW = 0;

        }
        else if (file_type == "PLY" || file_type == "ply" || file_type == "Ply")
        {
            do
            {
                strStream >> tmp;
            }
            while (tmp != "format");
            strStream >> tmp;
            if (tmp != "ascii")
            {
                cout << "Cannot read binary files!!!" << endl;
                assert(false);
            }

            do
            {
                strStream >> tmp;
            }
            while (tmp != "vertex");

            strStream >> my_numV;

            do
            {
                strStream >> tmp;
            }
            while (tmp != "face");

            strStream >> my_numF;

            do
            {
                strStream >> tmp;
            }
            while (tmp != "end_header");
            my_numE = 0;
            my_numW = 0;
        }
        else if (file_type == "OFFW" || file_type == "offw" || file_type == "Offw")
        {
            strStream >> my_numV >> my_numF >> my_numW;
            my_numE = 0;
        }
        else
        {
            return false;
        }

        if (my_numV == 60 && my_numF == 32)
        {
            isFootBall = true;
        }
        else
        {
            isFootBall = false;
        }

        float R, G, B;
        Point_3D W;
        W.setZ(0.0f);
        int wid,sharp;
        unsigned int val;
        bool snapped;
        // read vertex coordinates
        for (i = 0 ; i < my_numV ; i++)
        {
            strStream >> x >> y >> z >> R >> G >> B >> val;
            if(xyWeights) {
                vertex.my_point.setX(x*z);
                vertex.my_point.setY(y*z);
            } else {
                vertex.my_point.setX(x);
                vertex.my_point.setY(y);
            }
            vertex.my_point.setZ(z);

            vertex.isStable = false;
            vertex.isSelected = false;
            vertex.my_index = i;
            vertex.isFeature = true;
            //set all wavelet neighbourhood indices to -1
            vertex.my_W13vid0 = -1;
            vertex.my_W13vid1 = -1;
            vertex.my_W31vid0 = -1;
            vertex.my_W31vid1 = -1;

            vertex.my_W13sharpness = 0;
            vertex.my_W31sharpness = 0;
            vertex.my_W33sharpness = 0;

            vertex.my_newW13vid0 = -1;
            vertex.my_newW13vid1 = -1;
            vertex.my_newW31vid0 = -1;
            vertex.my_newW31vid1 = -1;

            vertex.my_colour.setX(R);
            vertex.my_colour.setY(G);
            vertex.my_colour.setZ(B);

            vertex.weight_ids.clear();
            vertex.weights.clear();
            vertex.sharpness.clear();
            vertex.weights_vec.clear();
            for(j = 0; j < val; j++) {
                strStream >> wid;
                vertex.weight_ids.append(wid);
            }
            for(j = 0; j < val; j++) {
                strStream >> x >> y;
                W.setX(x); W.setY(y);
                vertex.weights_vec.append(W);
            }
            for(j = 0; j < val; j++) {
                strStream >> sharp;
                vertex.sharpness.append(sharp);
            }
            strStream >> snapped;
            vertex.snapped = snapped;
            if(snapped) {
                strStream >> x >> y;
                vertex.my_visPoint.setX(x);
                vertex.my_visPoint.setY(y);
            }

            my_vertices.push_back(vertex);
            my_vertices[i].my_faceIndices.clear();
        }

        // read facets
        quadMesh = true;
        for (i = 0 ; i < my_numF ; i++)
        {
            facet.my_index = i;
            facet.my_vertIndices.clear();
            strStream >> n;
            facet.my_valency = n;
            if (n != 4)
            {
                quadMesh = false;
            }
            for (j = 0 ; j < n ; j++)
            {
                strStream >> vn;
                facet.my_vertIndices.push_back(vn);
                my_vertices[vn].my_faceIndices.push_back(i);
            }

            my_facets.push_back(facet);
        }

        my_sFacets.clear();
        my_sVertices.clear();
        my_sValues.clear();
        // read sharp edges
        for (i = 0 ; i < my_numE ; i++)
        {
            strStream >> f >> v >> s;
            my_sFacets.push_back(f);
            my_sVertices.push_back(v);
            my_sValues.push_back(s);
        }

        build();

    } //end of else

    return true;
}

void Mesh::addWeight(QList<float> *weights,QList<int> *ids,int index) {
    if(!ids->contains(index)) {
        weights->append(0.75f);
        ids->append(index);
    }
}

void Mesh::save(const char *fileName, FileType ftype)
{
    unsigned int	i, j;
    MeshVertex 		*v;
    ofstream 		file (fileName);

    if(!file.is_open())
    {
        cout<< "Failed to find the file:" << fileName << endl;
    }
    else
    {
        if (ftype == PLY)
        {
            file << "ply" << endl;
            file << "format ascii 1.0" << endl;
            file << "element vertex " << my_numV << endl;
            file << "property float32 x" << endl;
            file << "property float32 y" << endl;
            file << "property float32 z" << endl;
            file << "element face " << my_numF << endl;
            file << "property list uint8 int32 vertex_index" << endl;
            file << "end_header" << endl;
        }
        else if (ftype == OFF)
        {
            file << "OFF" << endl;
//            file << my_numV << " " << my_numF << " " << my_creaseCorners.size() << "" << endl;
            file << my_numV << " " << my_numF << " " << 0 << "" << endl;
        }
        else if (ftype == OFFW)
        {
            my_numW = 0;
            //count wavelets
            for (i = 0 ; i < my_numV ; i++)
            {
                v = &my_vertices[i];
                if (v->my_W13.getX() != 0 || v->my_W13.getY() != 0 || v->my_W13.getZ() != 0)
                {
                    my_numW++;
                }
                if (v->my_W31.getX() != 0 || v->my_W31.getY() != 0 || v->my_W31.getZ() != 0)
                {
                    my_numW++;
                }
                if (v->my_W33.getX() != 0 || v->my_W33.getY() != 0 || v->my_W33.getZ() != 0)
                {
                    my_numW++;
                }
            }

            file << "OFFW" << endl;
            file << my_numV << " " << my_numF << " " << my_numW << "" << endl;
        }
        else
        {
            assert(false);
        }

        //write vertices with weights
        for (i = 0 ; i < my_numV ; i++)
        {
            file << my_vertices[i].my_point << " " << my_vertices[i].my_colour << " ";
            file << my_vertices[i].weight_ids.size();
            for(j = 0; j < (unsigned int) my_vertices[i].weight_ids.size(); j++)
                file << " " << my_vertices[i].weight_ids[j];
            if(v3)
                for(j = 0; j < (unsigned int) my_vertices[i].weight_ids.size(); j++)
                    file<<" "<<my_vertices[i].weights_vec[j].getX()<<" "<<my_vertices[i].weights_vec[j].getY();
            else
                for(j = 0; j < (unsigned int) my_vertices[i].weight_ids.size(); j++)
                    file << " " << my_vertices[i].weights[j];
            if(my_vertices[i].sharpness.isEmpty()) {
                for(j = 0; j < (unsigned int) my_vertices[i].weight_ids.size(); j++)
                    file << " 0";
                file << " 0";
            } else { // v2/v3
                for(j = 0; j < (unsigned int) my_vertices[i].weight_ids.size(); j++)
                    file << " " << my_vertices[i].sharpness[j];
                file << " " << my_vertices[i].snapped;
                if(my_vertices[i].snapped)
                    file << " " << my_vertices[i].my_visPoint.getX() << " " << my_vertices[i].my_visPoint.getY();
            }
            file << endl;
        }

        if (ftype != PLY) file << endl;

        // write facets
        for (i = 0 ; i < my_numF ; i++)
        {
            file << my_facets[i].my_valency;

            for (j = 0 ; j < my_facets[i].my_vertIndices.size() ; j++)
            {
                file << " " << my_facets[i].my_vertIndices[j];
            }
            file << endl;
        }

        if (ftype == OFFW)
        {
            file << endl;

            //write wavelets
            for (i = 0 ; i < my_numV ; i++)
            {
                v = &my_vertices[i];
                if (v->my_W13.getX() != 0 || v->my_W13.getY() != 0 || v->my_W13.getZ() != 0)
                {
                    file << 1 << " " << 3 << " " << v->my_W13vid0 << " " << v->my_index << " " << v->my_W13vid1
                         << " " << v->my_W13.getX() << " " << v->my_W13.getY() << " " << v->my_W13.getZ()
                         << " " << v->my_W13sharpness << endl;
                }
                if (v->my_W31.getX() != 0 || v->my_W31.getY() != 0 || v->my_W31.getZ() != 0)
                {
                    file << 3 << " " << 1 << " " << v->my_W31vid0 << " " << v->my_index << " " << v->my_W31vid1
                         << " " << v->my_W31.getX() << " " << v->my_W31.getY() << " " << v->my_W31.getZ()
                         << " " << v->my_W31sharpness << endl;
                }
                if (v->my_W33.getX() != 0 || v->my_W33.getY() != 0 || v->my_W33.getZ() != 0)
                {
                    file << 3 << " " << 3 << " " << v->my_index << " " << v->my_index << " " << v->my_index
                         << " " << v->my_W33.getX() << " " << v->my_W33.getY() << " " << v->my_W33.getZ()
                         << " " << v->my_W33sharpness << endl;
                }
            }
        }

        cout << "Mesh saved" << endl;
    }
    file.close();
}

void Mesh::transf(void)
{
	unsigned int 		i;
	Point_3D 			cen_point, poi;
	PointPrec 			x, y, z, minx, miny, minz, maxx, maxy, maxz;
	PointPrec 			ratio, threshold, rng, scl;

	ratio = 1;
	threshold = 1.0e10;

	minx = threshold;
	maxx = -threshold;
	miny = threshold;
	maxy = -threshold;
	minz = threshold;
	maxz = -threshold;

	for(i = 0 ; i < my_numV ; i++)
	{
		poi = my_vertices[i].my_point;
		x = poi.getX();
		y = poi.getY();
		z = poi.getZ();
		if (x < minx) minx = x;
		if (x > maxx) maxx = x;
		if (y < miny) miny = y;
		if (y > maxy) maxy = y;
		if (z < minz) minz = z;
		if (z > maxz) maxz = z;
	}

	cen_point.setX((maxx + minx) / 2.0);
	cen_point.setY((maxy + miny) / 2.0);
	cen_point.setZ((maxz + minz) / 2.0);

	rng = maxx - minx;
	if (maxy - miny > rng) rng = maxy - miny;
	if (maxz - minz > rng) rng = maxz - minz;
	scl = ratio / rng;

	my_centre = cen_point;
	my_scale = scl;

    my_minz = minz;
    my_maxz = maxz;

//	if (transform)
//	{
//		for(i = 0 ; i < my_numV ; i++)
//		{
//			poi = my_vertices[i].my_point;
//			my_vertices[i].my_point.setX(scl * (poi.getX() - cen_point.getX()));
//			my_vertices[i].my_point.setY(scl * (poi.getY() - cen_point.getY()));
//			my_vertices[i].my_point.setZ(scl * (poi.getZ() - cen_point.getZ()));
//		}
//	}
//	else
//	{
//		cen_point = Point_3D(0,0,0);
//		scl = 1;
//	}

//	if (transform)
//	{
//        makeCurrent();
//		glLoadIdentity();
//		glScalef(0.7 * my_scale, 0.7 * my_scale, 0.7 * my_scale);
//        glTranslatef(-my_centre.getX(), -my_centre.getY(), -my_centre.getZ());
//	}

	cout << "cen: " << my_centre;
    cout << "scale: " << my_scale << endl;
}

void Mesh::build(void)
{
    unsigned int 	i, j, k, l, ind1, ind2, shift;
    MeshCorner		corner, *crn, *crnn;
    MeshFacet		*facet, *adjFacet, *nextFacet;
    bool			search;
    MeshVertex		*vert;

    my_numE = 0;

    hasBoundary = false;

    my_creaseCorners.clear();

    // fill vertex->facet pointers
//    my_EVlist.clear();

    for (i = 0 ; i < my_numV ; i++)
    {
        vert = &my_vertices[i];
        vert->isOnBoundary = false; // init all to false
        vert->my_valency = vert->my_faceIndices.size();
        if (vert->my_valency == 1) // fix for vertices of valency 1
        {
            vert->isOnBoundary = true;
        }

        vert->my_facets.clear();
        for (j = 0 ; j < vert->my_valency ; j++)
        {
            facet = &my_facets[vert->my_faceIndices[j]];
            vert->my_facets.push_back(facet);
        }
    }

    // find corners
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        facet->my_corners.clear();
        my_numE += facet->my_valency;
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            corner.my_vIndex = facet->my_vertIndices[j];
            corner.my_vertex = &(my_vertices[corner.my_vIndex]);
            corner.my_facet = facet;
            corner.my_alreadyIn = false;
            corner.my_nIndex = facet->my_vertIndices[(j + 1) % facet->my_valency];
            corner.my_sharpness = 0;
            corner.my_index = j;
            facet->my_corners.push_back(corner);
        }
    }

    // find next corners
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            facet->my_corners[j].my_nextCorner = &(facet->my_corners[(j + 1) % facet->my_valency]);
        }
    }

    // find next facets
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);
            ind1 = crn->my_vIndex;
            ind2 = crn->my_nIndex;

            // the following search returns only one face, thus may give wrong
            // answer if two-sided faces are present!!!
            k = 0;
            search = true;
            nextFacet = NULL;
            while (search && k < crn->my_vertex->my_valency)
            {
                adjFacet = crn->my_vertex->my_facets[k];
                crnn = &adjFacet->my_corners[0];
                l = 0;
                while (search && l < adjFacet->my_valency)
                {
                    if (crnn->my_vIndex == ind2 && crnn->my_nIndex == ind1)
                    {
                        search = false;
                        nextFacet = adjFacet;
                        shift = l;
                    }
                    crnn = crnn->my_nextCorner;
                    l++;
                }
                k++;
            }
            if (nextFacet == NULL)
            {
                crn->my_vertex->isOnBoundary = true;
                hasBoundary = true;
                crn->my_nextFacet = NULL;
                crn->my_nextFshift = 0;
                crn->my_pairedCorner = NULL;
            }
            else
            {
                crn->my_nextFacet = nextFacet;
                crn->my_nextFshift = shift;
                crn->my_pairedCorner = &(nextFacet->my_corners[shift]);
            }

            if(my_level == 0 && !my_vertices[ind1].sharpness.isEmpty()) { // v2
                int sharp = getSharpness(ind1,ind2);
                crn->my_sharpness = sharp;
                if (nextFacet != NULL)
                    crn->my_pairedCorner->my_sharpness = sharp;
                if(sharp>0)
                    my_creaseCorners.push_back(crn);
            }
        }
    }

    if (my_level > 0)
    {
        my_creaseCorners.clear();
        //create crease info for corners
        for (i = 0 ; i < my_sFacets.size() ; i++ )
        {
            crn = &my_facets[my_sFacets[i]].my_corners[my_sVertices[i]];
            if (crn->my_pairedCorner != NULL)
            {
                crn->my_sharpness = my_sValues[i];
                crn->my_pairedCorner->my_sharpness = my_sValues[i];
                //add crn to crease list
                my_creaseCorners.push_back(crn);
            }
        }
    }

    //comput limit points LP
    CatmullClarkLimit();

    //for zero-sharpness wavelets, set control vectors to CP-LP
    if (withWavelets)
    {
        // for wavelets, assign neighbour indices
        for (i = 0 ; i < my_numV ; i++)
        {
            vert = &my_vertices[i];
            if (vert->my_W13sharpness == 0)
            {
                vert->my_W13 = vert->my_point - vert->my_CClimit;
            }
            if (vert->my_W31sharpness == 0)
            {
                vert->my_W31 = vert->my_point - vert->my_CClimit;
            }
            if (vert->my_W33sharpness == 0)
            {
                vert->my_W33 = vert->my_point - vert->my_CClimit;
            }
        }
    }
}

bool Mesh::removeLowVal(void)
{
    unsigned int                i, j, k, faceI2, val;
    MeshVertex                  *vert, *v1, *v3, *v4;
    MeshFacet                   *face1, *face2, *face;
    MeshCorner                  *crn;
    bool                        cont;

//    std::vector<unsigned int>    iList;
//    std::vector<MeshVertex>     newVlist;


//    iList.clear();
//    newVlist.clear();

    //get a low-valency vertex

    cont = true;
    i = 0;
    while ((i < my_numV) && (cont))
    {
        vert = &(my_vertices[i]);
        if (!vert->isOnBoundary && vert->my_valency == 2)
        {
            cont = false;
        }
        else
        {
            i++;
        }
    }

    if (cont)
    {
        cout << "found nothing" << endl;
        // nothing to do
        return(false);
    }
    else
    {
        cout << "found " << i << endl;
        cout << "numV = " << my_numV << endl;

        // 2 faces to be removed
        face1 = vert->my_facets[0];
        face2 = vert->my_facets[1];
//        faceI1 = face1->my_index;
        faceI2 = face2->my_index;

        // 4 vertices for the new quad
        crn = findCorner(vert, face1);
        v1 = crn->my_nextCorner->my_vertex;
//        v2 = crn->my_nextCorner->my_nextCorner->my_vertex;
        crn = getSameCorner(crn);
        v3 = crn->my_nextCorner->my_vertex;
        v4 = crn->my_nextCorner->my_nextCorner->my_vertex;


        // replace the vertex to be removed with the last one
        my_vertices[i] = my_vertices.back();
        my_vertices[i].my_index = i;
        if (v4->my_index == my_numV - 1) // fix for v4 = last
        {
            v4 = &(my_vertices[i]);
        }
        if (v1->my_index == my_numV - 1) // fix for v1 = last
        {
            v1 = &(my_vertices[i]);
        }
        if (v3->my_index == my_numV - 1) // fix for v3 = last
        {
            v3 = &(my_vertices[i]);
        }
//        if (v2->my_index == my_numV - 1) // fix for v2 = last
//        {
//            v2 = &(my_vertices[i]);
//        }
        my_vertices.pop_back();
        my_numV--;

        // change face1 to the new face
        for (k = 0 ; k < face1->my_valency ; k++)
        {
            if (face1->my_vertIndices[k] == i)
            {
                face1->my_vertIndices[k] = v4->my_index;
            }
        }

        // fix faceIndices for v1
        for (k = 0 ; k < v1->my_valency ; k++)
        {
            if (v1->my_faceIndices[k] == face2->my_index)
            {
                v1->my_faceIndices.erase(v1->my_faceIndices.begin()+k);
            }
        }
        v1->my_valency--;
        // fix faceIndices for v3
        for (k = 0 ; k < v3->my_valency ; k++)
        {
            if (v3->my_faceIndices[k] == face2->my_index)
            {
                v3->my_faceIndices.erase(v3->my_faceIndices.begin()+k);
            }
        }
        v3->my_valency--;
        // fix faceIndices for v4
        for (k = 0 ; k < v4->my_valency ; k++)
        {
            if (v4->my_faceIndices[k] == face2->my_index)
            {
                v4->my_faceIndices[k] = face1->my_index;
            }
        }

        // update vertIndices of faces referring the last vertex
        val = vert->my_valency;
        for (j = 0 ; j < val ; j++)
        {
            face = vert->my_facets[j]; // careful here; cannot use my_facets!
            face = &(my_facets[vert->my_faceIndices[j]]);
            for (k = 0 ; k < face->my_valency ; k++)
            {
                if (face->my_vertIndices[k] == my_numV) // my_numV already --
                {
                    face->my_vertIndices[k] = i;
                }
            }
        }

        //remove face2
        my_facets.erase(my_facets.begin() + face2->my_index);
        my_numF--;

        // shift face indices down by 1
        for (k = faceI2 ; k < my_numF ; k++) // whatch out, face2 changed!, use faceI2 instead!
        {
            my_facets[k].my_index--;
        }

        // fix vertices referencing faces with shifted indices
        for (j = 0 ; j < my_numV ; j++)
        {
            vert = &(my_vertices[j]);
            for (k = 0 ; k < vert->my_valency ; k++)
            {
                if (vert->my_faceIndices[k] > faceI2)
                {
                    vert->my_faceIndices[k]--;
                }
            }
        }

//        // check all vertindices of facets
//        for (j = 0 ; j < my_numF ; j++)
//        {
//            for (k = 0 ; k < my_facets[j].my_valency ; k++)
//            {
//                if (my_facets[j].my_vertIndices[k] > my_numV - 1)
//                {
//                    cout << "PROBLEM" << endl;
//                }
//            }
//        }

        build();
        return(true);
    }
}

// returns the previous corner, but in the 'next' facet
MeshCorner* Mesh::getPrevCorner(MeshCorner *corner)
{
    MeshCorner *next;

    if (corner->my_nextFacet == NULL)
    {
        return NULL;
    }
    else
    {
        next = &corner->my_nextFacet->my_corners[corner->my_nextFshift];
        return next;
    }
}

// returns the same corner, but in the 'next' facet
MeshCorner* Mesh::getSameCorner(MeshCorner *corner)
{
    MeshCorner *next;

    if (corner->my_nextFacet == NULL)
    {
        return NULL;
    }
    else
    {
        next = &corner->my_nextFacet->my_corners[corner->my_nextFshift];
        next = next->my_nextCorner;
        return next;
    }
}

// returns the next corner, but in the 'next' facet
MeshCorner* Mesh::getNextCorner(MeshCorner *corner)
{
    MeshCorner *next;

    if (corner->my_nextFacet == NULL)
    {
        return NULL;
    }
    else
    {
        next = &corner->my_nextFacet->my_corners[corner->my_nextFshift];
        next = next->my_nextCorner->my_nextCorner;
        return next;
    }
}

void Mesh::compCurv(void)
{
	unsigned int 		i, j, ind, ind1, ind2, k, k1, k2, val1, val2;
	MeshVertex			*vertex;
	MeshFacet			*facet1, *facet2, *facet;
	Point_3D			poi11, poi12, poi13, poi21, poi22, poi23, vec;
	PointPrec			side11, side12, side13, m1, angle1,
						side21, side22, side23, m2, angle2,
                        sum, sumM, sumG, vor, minM, maxM, minG, maxG, mm, ang, angle;

	PointPrec PI = 4 * atan(1);

//cout << "Computing Mean curvature... " << endl;

	for (i = 0 ; i < my_numV ; i++)
	{
		sum = 0;
		vec = Point_3D(0,0,0);
		vor = 0;
//		mn = 0;
		vertex = &(my_vertices[i]);
		if (!(vertex->isOnBoundary))
		{
			ind = vertex->my_index;
			for (j = 0 ; j < vertex->my_valency ; j++)
			{
				facet1 = vertex->my_facets[j];
				facet2 = vertex->my_facets[(j + 1) % vertex->my_valency];

				val1 = facet1->my_valency;
				k1 = 0;
				ind1 = facet1->my_vertIndices[k1];
				while (ind1 != ind)
				{
					k1++;
					ind1 = facet1->my_vertIndices[k1];
				}
				val2 = facet2->my_valency;
				k2 = 0;

				ind2 = facet2->my_vertIndices[k2];
				while (ind2 != ind)
				{
					k2++;
					ind2 = facet2->my_vertIndices[k2];
				}

				// three consequtive points
				poi13 = my_vertices[facet1->my_vertIndices[(k1 + val1 - 1) % val1]].my_point;
				poi12 = vertex->my_point;
//				poi12 = my_vertices[facet1->my_vertIndices[k1]].my_point;
				poi11 = my_vertices[facet1->my_vertIndices[(k1 + 1) % val1]].my_point;

				poi23 = my_vertices[facet2->my_vertIndices[(k2 + val2 - 1) % val2]].my_point;
				poi22 = vertex->my_point;
//				poi22 = my_vertices[facet2->my_vertIndices[k2]].my_point;
				poi21 = my_vertices[facet2->my_vertIndices[(k2 + 1) % val2]].my_point;

				side11 = poi11.dist(poi12);
				side12 = poi12.dist(poi13);
				side13 = poi11.dist(poi13);

				side21 = poi21.dist(poi22);
				side22 = poi22.dist(poi23);
				side23 = poi21.dist(poi23);

//cout << side11 - side22 << endl;

				m1 = ((side11 * side11 - side12 * side12 - side13 * side13) / (-2.0 * side12 * side13));

//if (m1 < 0) cout << "m1 is negative" << endl;

				angle1 = acos(m1);

				m2 = ((side22 * side22 - side21 * side21 - side23 * side23) / (-2.0 * side21 * side23));
				angle2 = acos(m2);


				mm = ((side12 * side12 - side11 * side11 - side13 * side13) / (-2.0 * side11 * side13));
				ang = acos(mm);
//cout << mm << " " << ang << endl;

				angle = (PI - angle1 - ang) * (180 / PI);


				sum += angle;

//cout << sum << endl;

				vec = (1.0 / tan(angle1) + 1.0 / tan(angle2)) * poi12 + vec;
				vec = (- 1.0 / tan(angle1) - 1.0 / tan(angle2)) * poi11 + vec;

//if (tan(angle1) < 0) cout << "tan(angle1) is negative" << endl;

//				mn = (1.0 / tan(angle1) + 1.0 / tan(angle2)) * side11;

//cout << (1.0 / tan(angle1) + 1.0 / tan(angle2)) << endl;

//cout << angle1 * 180 / PI << " " << ang * 180 / PI << endl;

				if (angle1 * 180 / PI >= 90 || ang * 180 / PI >= 90 || angle >= 90)
				{
					if 	(angle >= 90)
					{
						vor += 1.0 / 2.0 * side12 * side13 * sin(angle1) / 2;
					}
					else
					{
						vor += 1.0 / 2.0 * side12 * side13 * sin(angle1) / 4;
					}
				}
				else
				{
					vor += 1.0 / 8.0 * (side12 * side12 / tan(ang) + side11 * side11 / tan(angle1));
				}
			}
//            vertex->my_curvM = vec.dist(Point_3D(0,0,0));// / 4.0 / vor;
//            vertex->my_curvM = mn / 2.0 / vor;
//            vertex->my_curvM = mn / vor;

			vertex->my_curvM = vor;

			// correction for boundary corners and edges

//			vor = 1.0;
//			if (vertex->my_valency == 1)
//			{
//				vertex->my_curvG = (90 - sum) / vor;
//			}
//			else if (vertex->my_valency == 2)
//			{
//				vertex->my_curvG = (180 - sum) / vor;
//			}
//			else
//			{
//				vertex->my_curvG = (360 - sum) / vor;
//			}

//            vertex->my_curvG = vor;
			vertex->my_curvG = (360 - sum) / vor;
		}
		else
		{
//cout << "Boundary " << i << endl;
			vertex->my_curvM = 0; // or other value for boundary points?
			vertex->my_curvG = 0;

//			for (j = 0 ; j < vertex->my_faceIndices.size() ; j++)
//			{
//				cout << vertex->my_faceIndices[j] << " ";
//			}
//			cout << endl;

		}

//cout << "G: " << vertex->my_curvG << endl;
//cout << "v: " << 10000 * vor << endl;

	}

	minM = 100000000000;
	maxM = -minM;
	minG = 100000000000;
	maxG = -minG;

	// average vertex curvatures and assign to facets
	for (i = 0 ; i < my_numF ; i++)
	{
		sumM = 0;
		sumG = 0;
		k = 0;
		facet = &(my_facets[i]);
		for (j = 0 ; j < facet->my_valency ; j++)
		{
			if (!my_vertices[facet->my_vertIndices[j]].isOnBoundary)
			{
				sumM += my_vertices[facet->my_vertIndices[j]].my_curvM;
				sumG += my_vertices[facet->my_vertIndices[j]].my_curvG;
				k++;
			}
		}
		if (k == 0 ) k = 1;
		sumM = sumM / k;
		sumG = sumG / k;
		if (sumM > maxM)
		{
			maxM = sumM;
		}
		if (sumM < minM)
		{
			minM = sumM;
		}
		if (sumG > maxG)
		{
			maxG = sumG;
		}
		if (sumG < minG)
		{
			minG = sumG;
		}
		facet->my_curvM = sumM;
		facet->my_curvG = sumG;
	}
	my_minM = minM;
	my_maxM = maxM;
	my_minG = minG;
	my_maxG = maxG;

cout << "curvM: " << minM << " -- " << maxM << " curvG: " << minG << " -- " << maxG << endl;

//cout << "DONE Computing curvature... " << endl;
}

void Mesh::compCurvSmooth(unsigned int rings)
{
	unsigned int 	i, j, k, count;
	MeshFacet		*facet;
	MeshVertex		*vertex, *ver;
	PointPrec		m, g;

	std::vector< std::vector <unsigned int> > ring_facets;
	std::vector< std::vector <unsigned int> > ring_vertices;

	// smooth curvature for vertices
	for (i = 0 ; i < my_numV ; i++)
	{
		vertex = &(my_vertices[i]);

		if (rings == 0)
		{
			vertex->my_curvMsmooth = vertex->my_curvM;
			vertex->my_curvGsmooth = vertex->my_curvG;
		}
		else
		{
			m = 0;
			g = 0;
			count = 0;
			getRingsV(vertex, rings, &ring_facets, &ring_vertices);
			for (j = 0 ; j < ring_vertices.size() ; j++)
			{
				for (k = 0 ; k < ring_vertices[j].size() ; k++)
				{
					ver = &(my_vertices[ring_vertices[j][k]]);
					m += ver->my_curvM;
					g += ver->my_curvG;
					count++;
				}
			}
			vertex->my_curvMsmooth = m / count;
			vertex->my_curvGsmooth = g / count;
		}
	}

	// smooth curvature for facets
	for (i = 0 ; i < my_numF ; i++)
	{
		facet = &(my_facets[i]);
		m = 0;
		g = 0;
		count = facet->my_valency;
		for (j = 0 ; j < count ; j++)
		{
			vertex = &(my_vertices[facet->my_vertIndices[j]]);
			m += vertex->my_curvMsmooth;
			g += vertex->my_curvGsmooth;
		}
		facet->my_curvMsmooth = m / count;
		facet->my_curvGsmooth = g / count;
	}
}

void Mesh::compCurvG(void)
{
    unsigned int 		i, j, ind, ind2, k, val;
    MeshVertex			*vertex;
    MeshFacet			*facet;
    Point_3D			poi1, poi2, poi3;
    PointPrec			side1, side2, side3, m, angle, sum, minG, maxG;

    PointPrec PI = 4 * atan(1);

//cout << "Computing Gaussian curvature... " << endl;

    for (i = 0 ; i < my_numV ; i++)
    {
        sum = 0;
        vertex = &(my_vertices[i]);
        ind = vertex->my_index;
        for (j = 0 ; j < vertex->my_valency ; j++)
        {
//cout << "val: " << vertex->my_valency << endl;
            facet = vertex->my_facets[j];
            val = facet->my_valency;
            k = 0;
            ind2 = facet->my_vertIndices[k];
            while (ind2 != ind)
            {
                k++;
                ind2 = facet->my_vertIndices[k];
            }
            // three consequtive points
            poi1 = my_vertices[facet->my_vertIndices[(k + val - 1) % val]].my_point;
            poi2 = vertex->my_point;
            poi3 = my_vertices[facet->my_vertIndices[(k + 1) % val]].my_point;

            side1 = poi1.dist(poi2);
            side2 = poi2.dist(poi3);
            side3 = poi1.dist(poi3);

            m = ((side3 * side3 - side2 * side2 - side1 * side1) / (-2.0 * side1 * side2));
            angle = acos(m);
            angle = angle * (180 / PI);

//cout << "Angle: " << angle << endl;

            sum += angle;
        }
        // correction for boundary corners and edges
        if (vertex->my_valency == 1)
        {
            vertex->my_curvG = 90 - sum;
        }
        else if (vertex->my_valency == 2)
        {
            vertex->my_curvG =180 - sum;
        }
        else
        {
            vertex->my_curvG = 360 - sum;
        }

//cout << vertex->my_valency << " curvG: " << vertex->my_curvG << endl;
    }

    minG = 100000;
    maxG = -minG;

    // average vertex curvatures and assign to facets
    for (i = 0 ; i < my_numF ; i++)
    {
        sum = 0;
        k = 0;
        facet = &(my_facets[i]);
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            if (!my_vertices[facet->my_vertIndices[j]].isOnBoundary)
            {
                sum += my_vertices[facet->my_vertIndices[j]].my_curvG;
                k++;
            }
        }
        sum = sum / k;
        if (sum > maxG)
        {
            maxG = sum;
        }
        if (sum < minG)
        {
            minG = sum;
        }
        facet->my_curvG = sum;
    }
    my_minG = minG;
    my_maxG = maxG;

//cout << "maxG: " << maxG << " minG: " << minG << endl;

//cout << "DONE Computing Gaussian curvature... " << endl;
}

Point_3D Mesh::getPoi(int i, int j, int k)
{
	return(my_vertices[map[i % my_NCvalency][j][k]].my_point);
}

void Mesh::generateRays(Mesh *mesh)
{
	MeshVertex				vertex;
	MeshRay					ray;
	SplineC					curve;
	vector< unsigned int >  					m;
	vector< vector< unsigned int > >			mp;
	unsigned int			i, j, k, count, ii;
	int						jj;
	Point_3D				poi;
	PointPrec				x, y;
	complex<PointPrec>		v, w, z;
	CtrlPlg					plg;
	KnotVec					tau, refTau;
	PointPrec				min, max, rnd;

	PointPrec const 		Pi = 4 * atan(1);

	max = 1.3;
	min = -max;

	my_vertices.clear();
	my_degree = my_NCdegree;

	if (my_NCdegree & 1) // it's odd
	{
        cout << "ODD DEGREE" << endl;

		//generate vertices for basic natural configuration

		// central vertex
		poi = Point_3D(0,0,0);
		vertex.my_point = poi;
		vertex.my_index = 0;
		my_vertices.push_back(vertex);
		my_vertices[0].my_faceIndices.clear();

		// other vertices
		count = 1;
		map.clear();
		for (k = 0 ; k < my_NCvalency ; k++) // index of segment
		{
			mp.clear();
			m.clear();
//			for (i = 1 ; i < my_NCsize ; i++)
			{
				m.push_back(0);
			}
			mp.push_back(m); // just to fix the central vertex
			for (i = 1 ; i < my_NCsizes[k] ; i++)
			{
				m.clear();
				for (j = 0 ; j < my_NCsizes[(k + 1) % my_NCvalency] ; j++)
				{
					v = complex<PointPrec>(0, 2 * Pi / my_NCvalency * k + Pi);
					w = complex<PointPrec>(i, j);
					z = exp(v) * pow(w, (PointPrec)4 / my_NCvalency);
					x = real(z);
					y = imag(z);

					if (my_rand == 1)
					{
						// randomizes the z-coords.
						rnd =  min + (float)rand() / (float)RAND_MAX * (max - min);
					}
					else
					{
						rnd = 0;
					}

					vertex.my_point = Point_3D(x, y, rnd);
					vertex.my_index = count;
					my_vertices.push_back(vertex);
					my_vertices[count].my_faceIndices.clear();
					m.push_back(count);
					count++;
				}
				mp.push_back(m);
			}
			map.push_back(mp);
		}

//        cout << "MAP: " << endl;
//                        for (k = 0 ; k < map.size() ; k++)
//                        {
//                                for (i = 0 ; i < map[k].size() ; i++)
//                                {
//                                        for (j = 0 ; j < map[k][i].size() ; j++)
//                                        {
//                                                cout << k << " " << i << " " << j << " " << map[k][i][j]
//                                 << " " << my_vertices[map[k][i][j]].my_point;
//                                        }
//                                }
//                        }
//        cout << "MAP DONE" << endl;

		// generate rays
		my_rays.clear();
		for (k = 0 ; k < my_NCvalency ; k++)
		{
			for (i = 0 ; i < my_NCsizes[k] ; i++)
			{
				// collect control polygons
				plg.clear();
				for (j = 0 ; j < my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1 ; j++)
				{
					plg.push_back(getPoi(my_NCvalency + k - 1, my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - j - 1, i));
				}
				if (i == 0) // first curve
				{
					plg.push_back(my_vertices[0].my_point);
					for (j = 0 ; j < my_NCsizes[(k + 1) % my_NCvalency] - 1 ; j++)
					{
						plg.push_back(getPoi(k + 1, j + 1, 0));
					}
				}
				else
				{
					for (j = 0 ; j < my_NCsizes[(k + 1) % my_NCvalency] ; j++)
					{
						plg.push_back(getPoi(k, i, j));
					}
				}

				// generate knot vectors
				tau.clear();
				refTau.clear();
				for (jj = (- my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] + 1) - (my_NCdegree + 1) / 2 ; jj < 0 ; jj++)
				{
					tau.push_back(jj);
					refTau.push_back(jj);
				}
				tau.push_back(0);
				for (j = 0 ; j < my_NCmults[k] ; j++)
				{
					refTau.push_back(0);
				}
				for (j = 1 ; j <= (my_NCsizes[(k + 1) % my_NCvalency] - 1) + (my_NCdegree + 1) / 2 ; j++)
				{
					tau.push_back(j);
					refTau.push_back(j);
				}

				curve = SplineC(my_NCdegree + 1, tau, plg);
				curve.my_refTau = refTau;
//cout << "crossCurve before: " << k << " " << i << endl;
//cout << curve << endl;
				if (i > (my_NCdegree - 1) / 2)
				{
					curve.oslo();				// knot insertion along crossCurves
					curve.refineRay();
				}

				if (my_rand == 2)
				{
					for (j = 0 ; j < curve.my_plg.size() ; j++)
					{
						// randomizes the z-coords.
						rnd =  min + (float)rand() / (float)RAND_MAX * (max - min);
						curve.my_plg[j].setZ(curve.my_plg[j].getZ() + rnd);
					}
				}
				if (mesh != NULL && i != 0)
				{
						my_vertices[0] = mesh->my_vertices[0];

//cout << k << " " << i << " " << mesh->map[k][i].size() << endl;

					for (j = 0 ; j < mesh->map[k][i].size() ; j++)
					{
						curve.my_plg[j] = mesh->getPoi(k, i, j);
//                      curve.my_plg[j] = mesh->my_rays[k].my_crossCurves[i].my_plg[j];
					}
				}

				ray.my_crossCurves.push_back(curve);
//cout << "crossCurve after: " << k << " " << i << endl;
//cout << curve << endl;
			}
			ray.my_index = k;
			my_rays.push_back(ray);
			ray.my_crossCurves.clear();
		}
//cout << "RAYS DONE" << endl;

				// for higher degrees, some moved vertices need to be copied over between crossCurves
				for (k = 0 ; k < my_NCvalency ; k++) // segment
				{
						for (i = 1 ; i < (my_NCdegree - 1) / 2 ; i++)
						{
								for (j = 0 ; j < my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - (my_NCdegree - 1) / 2 - 1 ; j++)
								{
										ii = my_rays[(k + my_NCvalency - 1) % my_NCvalency].my_crossCurves[my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - j - 1].my_plg.size();
										my_rays[k].my_crossCurves[i].my_plg[j] =
												my_rays[(k + my_NCvalency - 1) % my_NCvalency].my_crossCurves[my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - j - 1].my_plg[ii + i - my_NCsizes[(my_NCvalency + k) % my_NCvalency]];
								}
						}
				}

//         copying over all vertices (for rand == 2)
		for (k = 0 ; k < my_NCvalency ; k++) // segment
		{
			for (i = 1 ; i < my_NCsizes[k] ; i++)
			{
				for (j = 0 ; j < my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1 ; j++)
				{
						ii = my_rays[(k + my_NCvalency - 1) % my_NCvalency].my_crossCurves[my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - j - 1].my_plg.size();

//                        cout << "Replacing " << k << " " << i << " " << j << " with " <<
//                                (k + my_NCvalency - 1) % my_NCvalency << " " << my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - j - 1 << " " << ii + i - my_NCsizes[(my_NCvalency + k) % my_NCvalency] << endl;

						my_rays[(k + my_NCvalency - 1) % my_NCvalency].my_crossCurves[my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - j - 1].my_plg[ii + i - my_NCsizes[(my_NCvalency + k) % my_NCvalency]] =
							my_rays[k].my_crossCurves[i].my_plg[j];
				}
			}
		}
	}
	else // it's even
	{
        cout << "EVEN DEGREE" << endl;

		// vertices
		count = 0;
		map.clear();
		for (k = 0 ; k < my_NCvalency ; k++)
		{
			mp.clear();
			m.clear();
			for (i = 0 ; i < my_NCsizes[k] ; i++)
			{
				m.clear();
				for (j = 0 ; j < my_NCsizes[(k + 1) % my_NCvalency] ; j++)
				{
					v = complex<PointPrec>(0, 2 * Pi / my_NCvalency * k + Pi);
					w = complex<PointPrec>(i + 0.5, j + 0.5);
					z = exp(v) * pow(w, (PointPrec)4 / my_NCvalency);
					x = real(z);
					y = imag(z);

					if (my_rand == 1)
					{
						// randomizes the z-coords.
						rnd =  min + (float)rand() / (float)RAND_MAX * (max - min);
					}
					else
					{
						rnd = 0;
					}

					vertex.my_point = Point_3D(x, y, rnd);
					vertex.my_index = count;
					my_vertices.push_back(vertex);
					my_vertices[count].my_faceIndices.clear();
					m.push_back(count);
					count++;
				}
				mp.push_back(m);
			}
			map.push_back(mp);
		}

		// generate rays
		my_rays.clear();
		for (k = 0 ; k < my_NCvalency ; k++)
		{
			for (i = 0 ; i < my_NCsizes[k] ; i++)
			{
				// collect control polygons
				plg.clear();
				for (j = 0 ; j < my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] ; j++)
				{
					plg.push_back(getPoi(my_NCvalency + k - 1, my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - j - 1, i));
				}
				for (j = 0 ; j < my_NCsizes[(k + 1) % my_NCvalency]; j++)
				{
					plg.push_back(getPoi(k, i, j));
				}



				// generate knot vectors
				tau.clear();
				refTau.clear();
				for (jj = - my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - my_NCdegree / 2 ; jj < 0 ; jj++)
				{
					tau.push_back(jj);
					refTau.push_back(jj);
				}
				tau.push_back(0);
				for (j = 0 ; j < my_NCmults[k] ; j++)
				{
					refTau.push_back(0);
				}
				for (j = 1 ; j <= my_NCsizes[(k + 1) % my_NCvalency] + my_NCdegree / 2 ; j++)
				{
					tau.push_back(j);
					refTau.push_back(j);
				}

				curve = SplineC(my_NCdegree + 1, tau, plg);
				curve.my_refTau = refTau;
//cout << "crossCurve before: " << k << " " << i << endl;
//cout << curve << endl;
				if (i > my_NCdegree / 2 - 1)
				{
					curve.oslo();				// knot insertion along crossCurves
					curve.refineRay();
				}

				if (my_rand == 2)
				{
					for (j = 0 ; j < curve.my_plg.size() ; j++)
					{
						// randomizes the z-coords.
						rnd =  min + (float)rand() / (float)RAND_MAX * (max - min);
						curve.my_plg[j].setZ(curve.my_plg[j].getZ() + rnd);
					}
				}
				if (mesh != NULL)
				{
//                	my_vertices[0] = mesh->my_vertices[0];

cout << k << " " << i << " " << mesh->map[k][i].size() << endl;

					for (j = 0 ; j < mesh->map[k][i].size() ; j++)
					{
						curve.my_plg[j] = mesh->getPoi(k, i, j);
//                      curve.my_plg[j] = mesh->my_rays[k].my_crossCurves[i].my_plg[j];
					}
				}

				ray.my_crossCurves.push_back(curve);
//cout << "crossCurve after: " << k << " " << i << endl;
//cout << curve << endl;
			}
			ray.my_index = k;
			my_rays.push_back(ray);
			ray.my_crossCurves.clear();
		}
//cout << "RAYS DONE" << endl;


				// for higher degrees, some moved vertices need to be copied over between crossCurves
				for (k = 0 ; k < my_NCvalency ; k++) // segment
				{
					for (i = 0 ; i < (my_NCdegree / 2) - 1 ; i++)
						{
								for (j = 0 ; j < my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - my_NCdegree / 2 ; j++)
								{
										ii = my_rays[(k + my_NCvalency - 1) % my_NCvalency].my_crossCurves[my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - j - 1].my_plg.size();
										my_rays[k].my_crossCurves[i].my_plg[j] =
												my_rays[(k + my_NCvalency - 1) % my_NCvalency].my_crossCurves[my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - j - 1].my_plg[ii + i - my_NCsizes[(my_NCvalency + k) % my_NCvalency]];
								}
						}
				}

//         copying over all vertices (for rand == 2)
		for (k = 0 ; k < my_NCvalency ; k++) // segment
		{
			for (i = 0 ; i < my_NCsizes[k] ; i++)
			{
				for (j = 0 ; j < my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1 ; j++)
				{
						ii = my_rays[(k + my_NCvalency - 1) % my_NCvalency].my_crossCurves[my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - j - 1].my_plg.size();

//                        cout << "Replacing " << k << " " << i << " " << j << " with " <<
//                                (k + my_NCvalency - 1) % my_NCvalency << " " << my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - j - 1 << " " << ii + i - my_NCsizes[(my_NCvalency + k) % my_NCvalency] << endl;

						my_rays[(k + my_NCvalency - 1) % my_NCvalency].my_crossCurves[my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - j - 1].my_plg[ii + i - my_NCsizes[(my_NCvalency + k) % my_NCvalency]] =
							my_rays[k].my_crossCurves[i].my_plg[j];
				}
			}
		}
	}
}

void Mesh::generateCtrlMeshFromRays()
{
	MeshVertex				vertex;
	MeshFacet				facet;
	vector< unsigned int >  					m;
	vector< vector< unsigned int > >			mp;
//	unsigned int			i, j, k;
	Point_3D				poi;

	vertex = my_vertices[0];

	my_facets.clear();
	my_vertices.clear();
	my_level = 0;

	if (my_NCdegree & 1) // it's odd
	{
		//regenerate vertices from knot-inserted rays for general natural configuration

		// central vertex
//        poi = Point_3D(0,0,0);
//        vertex.my_point = poi;
		vertex.my_k = 0;
		vertex.my_i = 0;
		vertex.my_j = 0;
		vertex.my_index = 0;
        vertex.isFeature = true;
        my_vertices.push_back(vertex);
		my_vertices[0].my_faceIndices.clear();

		raysToMesh();
	}
	else // it's even
	{
		//regenerate vertices from knot-inserted rays for general natural configuration
		raysToMesh();
	}

	my_numV = my_vertices.size();
	my_numF = my_facets.size();
	my_numE = 0;

	transf();
    cout << "V, F, E: " << my_numV << " " << my_numF << " " << my_numE << endl;
	build();

//	// transform rays so that they corresp. to transf. mesh
//	for (k = 0 ; k < my_NCvalency ; k++)
//	{
//		for (i = 0 ; i < my_rays[k].my_crossCurves.size() ; i++)
//		{
//			for (j = 0 ; j < my_rays[k].my_crossCurves[i].my_plg.size(); j++)
//			{
//				poi = my_rays[k].my_crossCurves[i].my_plg[j];
//				my_rays[k].my_crossCurves[i].my_plg[j].setX(my_scale * (poi.getX() - my_centre.getX()));
//				my_rays[k].my_crossCurves[i].my_plg[j].setY(my_scale * (poi.getY() - my_centre.getY()));
//				my_rays[k].my_crossCurves[i].my_plg[j].setZ(my_scale * (poi.getZ() - my_centre.getZ()));
//			}
//		}
//	}
}

void Mesh::raysToMesh(void)
{
    MeshVertex				vertex, *vert;
    MeshFacet				facet;
	vector< unsigned int >  					m;
	vector< vector< unsigned int > >			mp;
	unsigned int			i, j, k, count, maxJ, last1, last2;
//	Point_3D				poi;

	if (my_NCdegree & 1)
	{
		// other vertices - taken along rays
		count = 1;
		map.clear();
		for (k = 0 ; k < my_NCvalency ; k++) // index of segment (and ray)
		{
			mp.clear();
			m.clear();
			m.push_back(0);
			mp.push_back(m);
			for (i = 1 ; i < my_NCsizes[k] ; i++) // crossCurve index
			{
				m.clear();
				// maxJ = number of vertices along a crossCurve
				if (i < (my_NCdegree + 1) / 2)
				{
					if (i == 0) // the central vertex, so that it's not included several times
					{
						maxJ = my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1;
					}
					else
					{
						maxJ = my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency];
					}
				}
				else
				{
					maxJ = my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] + my_NCmults[k] - 1;
				}
//cout << "maxJ for k: " << k << " i: " << i << " is: " << maxJ << endl;
				for (j = 0 ; j < maxJ ; j++) // index of a point along a crossCurve
				{
					vertex.my_point = my_rays[k].my_crossCurves[i].my_plg[j];
					vertex.my_k = k;
					vertex.my_i = i;
					vertex.my_j = j;
					vertex.my_index = count;
                    vertex.isFeature = true;
					my_vertices.push_back(vertex);
					my_vertices[count].my_faceIndices.clear();
					m.push_back(count);
					count++;
				}
				mp.push_back(m);
			}
			map.push_back(mp);
		}

//		cout << "MAP: " << endl;
//						for (k = 0 ; k < map.size() ; k++)
//						{
//								for (i = 0 ; i < map[k].size() ; i++)
//								{
//										for (j = 0 ; j < map[k][i].size() ; j++)
//										{
//												cout << k << " " << i << " " << j << " " << map[k][i][j]
//								 << " " << my_vertices[map[k][i][j]].my_point;
//										}
//								}
//						}
//		cout << "MAP DONE" << endl;

		// create facets
		count = 0;
		for (k = 0 ; k < my_NCvalency ; k++) // segment
		{
//cout << endl;
			for (i = 1 ; i < my_NCsizes[k] - 1 ; i ++) // crossCurve
			{
				for (j = 0 ; j < my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1; j++) // face along cross curve
				{
//cout << "j: " << j << endl;
					facet.my_vertIndices.push_back(map[k][i][j]);
									   my_vertices[map[k][i][j]].my_faceIndices.push_back(count);
					facet.my_vertIndices.push_back(map[k][i + 1][j]);
									   my_vertices[map[k][i + 1][j]].my_faceIndices.push_back(count);
					facet.my_vertIndices.push_back(map[k][i + 1][j + 1]);
									   my_vertices[map[k][i + 1][j + 1]].my_faceIndices.push_back(count);
					facet.my_vertIndices.push_back(map[k][i][j + 1]);
									   my_vertices[map[k][i][j + 1]].my_faceIndices.push_back(count);
					facet.my_valency = 4;
					facet.my_index = count;
					if (j == 0 || i == my_NCsizes[k] - 2)
					{
						facet.drawMe = false;
					}
					else
					{
						facet.drawMe = true;
					}
					my_facets.push_back(facet);
					facet.my_vertIndices.clear();
					count++;
//cout << k << " " << i << " " << j << " HERE" << endl;
				}
				// add remaining facets (depends on i and degree)
				if (i < (my_NCdegree - 1) / 2) // part with mult. = 1
				{
					// just add last facet
					last1 = my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1;
					last2 = last1;
				}
				else if (i == (my_NCdegree - 1) / 2) // part with 'triangles'
				{
					last1 = my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1;
					last2 = my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1 + my_NCmults[k] - 1;
					// add triangles
					for (j = 0 ; j < my_NCmults[k] - 1; j++)
					{
						facet.my_vertIndices.push_back(map[k][i][last1]);
										   my_vertices[map[k][i][last1]].my_faceIndices.push_back(count);
						facet.my_vertIndices.push_back(map[k][i + 1][last2 - my_NCmults[k] + 1 + j]);
										   my_vertices[map[k][i + 1][last2 - my_NCmults[k] + 1 + j]].my_faceIndices.push_back(count);
						facet.my_vertIndices.push_back(map[k][i + 1][last2 - my_NCmults[k] + 1 + j + 1]);
										   my_vertices[map[k][i + 1][last2 - my_NCmults[k] + 1 + j + 1]].my_faceIndices.push_back(count);
						facet.my_valency = 3;
						facet.my_index = count;
						if (i == my_NCsizes[k] - 2)
						{
							facet.drawMe = false;
						}
						else
						{
							facet.drawMe = true;
						}
						my_facets.push_back(facet);
						facet.my_vertIndices.clear();
						count++;

						// skip faces for full multiplicities
						if ((my_NCmults[k] == my_NCdegree + 1) && (j == my_NCmults[k] / 2 - 2))
						{
							j++;
						}

//cout << k << " " << i << " " << j << " HERE3" << endl;
					}
				}
				else // part with mult., but regular
				{
					last1 = my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1 + my_NCmults[k] - 1;
					last2 = last1;
					// add quads
					for (j = 0 ; j < my_NCmults[k] - 1; j++)
					{
						facet.my_vertIndices.push_back(map[k][i][last1 - my_NCmults[k] + 1 + j]);
										   my_vertices[map[k][i][last1 - my_NCmults[k] + 1 + j]].my_faceIndices.push_back(count);
						facet.my_vertIndices.push_back(map[k][i + 1][last1 - my_NCmults[k] + 1 + j]);
										   my_vertices[map[k][i + 1][last1 - my_NCmults[k] + 1 + j]].my_faceIndices.push_back(count);
						facet.my_vertIndices.push_back(map[k][i + 1][last1 - my_NCmults[k] + 1 + j + 1]);
										   my_vertices[map[k][i + 1][last1 - my_NCmults[k] + 1 + j + 1]].my_faceIndices.push_back(count);
						facet.my_vertIndices.push_back(map[k][i][last1 - my_NCmults[k] + 1 + j + 1]);
										   my_vertices[map[k][i][last1 - my_NCmults[k] + 1 + j + 1]].my_faceIndices.push_back(count);
						facet.my_valency = 4;
						facet.my_index = count;
						if (i == my_NCsizes[k] - 2)
						{
							facet.drawMe = false;
						}
						else
						{
							facet.drawMe = true;
						}
						my_facets.push_back(facet);
						facet.my_vertIndices.clear();
						count++;

						// skip faces for full multiplicities
						if ((my_NCmults[k] == my_NCdegree + 1) && (j == my_NCmults[k] / 2 - 2))
						{
							j++;
						}
//cout << k << " " << i << " " << j << " HERE4" << endl;
					}
				}
				// adding last facet
//cout << "last1: " << last1 << " last2: " << last2 << endl;
				facet.my_vertIndices.push_back(map[k][i][last1]);
								   my_vertices[map[k][i][last1]].my_faceIndices.push_back(count);
				facet.my_vertIndices.push_back(map[k][i + 1][last2]);
								   my_vertices[map[k][i + 1][last2]].my_faceIndices.push_back(count);
				facet.my_vertIndices.push_back(map[(k + 1) % my_NCvalency][1][my_NCsizes[k] - i - 2]);
								   my_vertices[map[(k + 1) % my_NCvalency][1][my_NCsizes[k] - i - 2]].my_faceIndices.push_back(count);
				facet.my_vertIndices.push_back(map[(k + 1) % my_NCvalency][1][my_NCsizes[k] - i - 1]);
								   my_vertices[map[(k + 1) % my_NCvalency][1][my_NCsizes[k] - i - 1]].my_faceIndices.push_back(count);
				facet.my_valency = 4;
				facet.my_index = count;
				if (i == my_NCsizes[k] - 2)
				{
					facet.drawMe = false;
				}
				else
				{
					facet.drawMe = true;
				}
				my_facets.push_back(facet);
				facet.my_vertIndices.clear();
				count++;
//cout << k << " " << i << " " << j << " HERE last" << endl;
			}
		}
		// adding central ring
		for (k = 0 ; k < my_NCvalency ; k++)
		{
			facet.my_vertIndices.push_back(0);
							   my_vertices[0].my_faceIndices.push_back(count);
			facet.my_vertIndices.push_back(map[k][1][my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1]);
							   my_vertices[map[k][1][my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1]].my_faceIndices.push_back(count);
			facet.my_vertIndices.push_back(map[(k + 1) % my_NCvalency][1][my_NCsizes[(my_NCvalency + k) % my_NCvalency] - 2]);
							   my_vertices[map[(k + 1) % my_NCvalency][1][my_NCsizes[(my_NCvalency + k) % my_NCvalency] - 2]].my_faceIndices.push_back(count);
			facet.my_vertIndices.push_back(map[(k + 1) % my_NCvalency][1][my_NCsizes[(my_NCvalency + k) % my_NCvalency] - 1]);
							   my_vertices[map[(k + 1) % my_NCvalency][1][my_NCsizes[(my_NCvalency + k) % my_NCvalency] - 1]].my_faceIndices.push_back(count);
			facet.my_valency = 4;
			facet.my_index = count;
			facet.drawMe = true;
			my_facets.push_back(facet);
			facet.my_vertIndices.clear();
			count++;
		}
        cout << "ODD DEGREE DONE" << endl;
	}
	else // it's even
	{
		//regenerate vertices from knot-inserted rays for general natural configuration

		count = 0;
		map.clear();
		for (k = 0 ; k < my_NCvalency ; k++) // index of segment (and ray)
		{
			mp.clear();
			m.clear();
			for (i = 0 ; i < my_NCsizes[k] ; i++) // crossCurve index
			{
				m.clear();
				// maxJ = number of vertices along a crossCurve
				if (i < my_NCdegree / 2)
				{
					maxJ = my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency];
				}
				else
				{
					maxJ = my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] + my_NCmults[k] - 1;
				}
//cout << "maxJ for k: " << k << " i: " << i << " is: " << maxJ << endl;
				for (j = 0 ; j < maxJ ; j++) // index of a point along a crossCurve
				{
					vertex.my_point = my_rays[k].my_crossCurves[i].my_plg[j];
					vertex.my_k = k;
					vertex.my_i = i;
					vertex.my_j = j;
					vertex.my_index = count;
                    vertex.isFeature = true;
					my_vertices.push_back(vertex);
					my_vertices[count].my_faceIndices.clear();
					m.push_back(count);
					count++;
				}
				mp.push_back(m);
			}
			map.push_back(mp);
		}

//cout << "MAP: " << endl;
//		for (k = 0 ; k < map.size() ; k++)
//		{
//			for (i = 0 ; i < map[k].size() ; i++)
//			{
//				for (j = 0 ; j < map[k][i].size() ; j++)
//				{
//					cout << k << " " << i << " " << j << " " << map[k][i][j]
//                         << " " << my_vertices[map[k][i][j]].my_point;
//				}
//			}
//		}
//cout << "MAP DONE" << endl;

		// create facets
		count = 0;
		// central facet
		for (k = 0 ; k < my_NCvalency ; k++)
		{
			facet.my_vertIndices.push_back(map[k][0][my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1]);
							   my_vertices[map[k][0][my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1]].my_faceIndices.push_back(count);
		}
		facet.my_valency = my_NCvalency;
		facet.my_index = count;
		facet.drawMe = true;
		my_facets.push_back(facet);
		facet.my_vertIndices.clear();
		count = 1;

		for (k = 0 ; k < my_NCvalency ; k++) // segment
		{
//cout << endl;
			for (i = 0 ; i < my_NCsizes[k] - 1 ; i ++) // crossCurve
			{
				for (j = 0 ; j < my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1; j++) // face along cross curve
				{
//cout << "j: " << j << endl;
					facet.my_vertIndices.push_back(map[k][i][j]);
									   my_vertices[map[k][i][j]].my_faceIndices.push_back(count);
					facet.my_vertIndices.push_back(map[k][i + 1][j]);
									   my_vertices[map[k][i + 1][j]].my_faceIndices.push_back(count);
					facet.my_vertIndices.push_back(map[k][i + 1][j + 1]);
									   my_vertices[map[k][i + 1][j + 1]].my_faceIndices.push_back(count);
					facet.my_vertIndices.push_back(map[k][i][j + 1]);
									   my_vertices[map[k][i][j + 1]].my_faceIndices.push_back(count);
					facet.my_valency = 4;
					facet.my_index = count;
					if (j == 0 || i == my_NCsizes[k] - 2)
					{
						facet.drawMe = false;
					}
					else
					{
						facet.drawMe = true;
					}
					my_facets.push_back(facet);
					facet.my_vertIndices.clear();
					count++;
//cout << k << " " << i << " " << j << " HERE" << endl;
				}
				// add remaining facets (depends on i and degree)
				if (i < my_NCdegree / 2 - 1) // part with mult. = 1
				{
					// just add last facet
					last1 = my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1;
					last2 = last1;
				}
				else if (i == my_NCdegree / 2 - 1) // part with 'pentagons'
				{
					last1 = my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1;

					facet.my_vertIndices.push_back(map[k][i][last1]);
									   my_vertices[map[k][i][last1]].my_faceIndices.push_back(count);
					for (j = 0 ; j < my_NCmults[k]; j++)
					{
						facet.my_vertIndices.push_back(map[k][i + 1][last1 + j]);
										   my_vertices[map[k][i + 1][last1 + j]].my_faceIndices.push_back(count);
					}
					facet.my_vertIndices.push_back(map[(k + 1) % my_NCvalency][0][my_NCsizes[(k) % my_NCvalency] - i - 2]);
									   my_vertices[map[(k + 1) % my_NCvalency][0][my_NCsizes[(k) % my_NCvalency] - i - 2]].my_faceIndices.push_back(count);
					facet.my_vertIndices.push_back(map[(k + 1) % my_NCvalency][0][my_NCsizes[(k) % my_NCvalency] - i - 1]);
									   my_vertices[map[(k + 1) % my_NCvalency][0][my_NCsizes[(k) % my_NCvalency] - i - 1]].my_faceIndices.push_back(count);

					facet.my_valency = my_NCmults[k] + 3;
					facet.my_index = count;
					if (j == 0 || i == my_NCsizes[k] - 2)
					{
						facet.drawMe = false;
					}
					else
					{
						facet.drawMe = true;
					}
					my_facets.push_back(facet);
					facet.my_vertIndices.clear();
					count++;
//cout << k << " " << i << " " << j << " HERE5" << endl;
				}
				else // part with mult., but regular
				{
					last1 = my_NCsizes[(my_NCvalency + k - 1) % my_NCvalency] - 1 + my_NCmults[k] - 1;
					last2 = last1;
					// add quads
					for (j = 0 ; j < my_NCmults[k] - 1; j++)
					{
						facet.my_vertIndices.push_back(map[k][i][last1 - my_NCmults[k] + 1 + j]);
										   my_vertices[map[k][i][last1 - my_NCmults[k] + 1 + j]].my_faceIndices.push_back(count);
						facet.my_vertIndices.push_back(map[k][i + 1][last1 - my_NCmults[k] + 1 + j]);
										   my_vertices[map[k][i + 1][last1 - my_NCmults[k] + 1 + j]].my_faceIndices.push_back(count);
						facet.my_vertIndices.push_back(map[k][i + 1][last1 - my_NCmults[k] + 1 + j + 1]);
										   my_vertices[map[k][i + 1][last1 - my_NCmults[k] + 1 + j + 1]].my_faceIndices.push_back(count);
						facet.my_vertIndices.push_back(map[k][i][last1 - my_NCmults[k] + 1 + j + 1]);
										   my_vertices[map[k][i][last1 - my_NCmults[k] + 1 + j + 1]].my_faceIndices.push_back(count);
						facet.my_valency = 4;
						facet.my_index = count;
						if (i == my_NCsizes[k] - 2)
						{
							facet.drawMe = false;
						}
						else
						{
							facet.drawMe = true;
						}
						my_facets.push_back(facet);
						facet.my_vertIndices.clear();
						count++;
						// skip faces for full multiplicities
						if ((my_NCmults[k] == my_NCdegree + 1) && (j == (my_NCmults[k] - 1) / 2 - 1))
						{
							j++;
						}
////cout << k << " " << i << " " << j << " HERE4" << endl;
					}
				}
//				// adding last facet
				if (i != my_NCdegree / 2 - 1) // not the part with pentagons
				{
//cout << "last1: " << last1 << " last2: " << last2 << endl;
					facet.my_vertIndices.push_back(map[k][i][last1]);
									   my_vertices[map[k][i][last1]].my_faceIndices.push_back(count);
					facet.my_vertIndices.push_back(map[k][i + 1][last2]);
									   my_vertices[map[k][i + 1][last2]].my_faceIndices.push_back(count);
					facet.my_vertIndices.push_back(map[(k + 1) % my_NCvalency][0][my_NCsizes[(k) % my_NCvalency] - i - 2]);
									   my_vertices[map[(k + 1) % my_NCvalency][0][my_NCsizes[(k) % my_NCvalency] - i - 2]].my_faceIndices.push_back(count);
					facet.my_vertIndices.push_back(map[(k + 1) % my_NCvalency][0][my_NCsizes[(k) % my_NCvalency] - i - 1]);
									   my_vertices[map[(k + 1) % my_NCvalency][0][my_NCsizes[(k) % my_NCvalency] - i - 1]].my_faceIndices.push_back(count);
					facet.my_valency = 4;
					facet.my_index = count;
					if (i == my_NCsizes[k] - 2)
					{
						facet.drawMe = false;
					}
					else
					{
						facet.drawMe = true;
					}
					my_facets.push_back(facet);
					facet.my_vertIndices.clear();
					count++;
				}
//cout << k << " " << i << " " << j << " HERE last" << endl;
			}
		}
        cout << "EVEN DEGREE DONE" << endl;
	}

//	if (rever)
//	{
//	//reverse orientation of facets
//		for (i = 0 ; i < my_facets.size() ; i ++)
//		{
//			fac = &(my_facets[i]);
//			reverse(fac->my_vertIndices.begin(), fac->my_vertIndices.end());
//		}
//	}

    //assign -1 to wavelet index pointers
    for (i = 0 ; i < my_vertices.size() ; i++)
    {
        vert = &my_vertices[i];
        vert->my_W13vid0 = -1;
        vert->my_W13vid1 = -1;
        vert->my_W31vid0 = -1;
        vert->my_W31vid1 = -1;

        vert->my_newW13vid0 = -1;
        vert->my_newW13vid1 = -1;
        vert->my_newW31vid0 = -1;
        vert->my_newW31vid1 = -1;

        vert->my_W13sharpness = 0;
        vert->my_W31sharpness = 0;
        vert->my_W33sharpness = 0;
    }
}

void Mesh::triangulate()
{
	unsigned int		i, j, k, l, val;
	MeshFacet			*facet, fac;
	MeshVertex			*vertex;
	std::vector <unsigned int> faceIndices;
	std::vector <MeshFacet>  	facets;

//    cout << "Triang begin" << endl;

	k = my_numF;

	for (i = 0 ; i < my_numF ; i++)
	{
		facet = &(my_facets[i]);
		val = facet->my_valency;
		if (val > 3)
		{
//cout << i << endl;
			for (j = 0 ; j < val - 3 ; j++)
			{
				my_vertices[facet->my_vertIndices[0]].my_faceIndices.push_back(k);
				my_vertices[facet->my_vertIndices[j + 1]].my_faceIndices.push_back(k);
				my_vertices[facet->my_vertIndices[j + 2]].my_faceIndices.push_back(k);
				fac.my_index = k;
				fac.my_valency = 3;
				fac.my_vertIndices.clear();
				fac.my_vertIndices.push_back(facet->my_vertIndices[0]);
				fac.my_vertIndices.push_back(facet->my_vertIndices[j + 1]);
				fac.my_vertIndices.push_back(facet->my_vertIndices[j + 2]);
				facets.push_back(fac);
				k++;
			}
			// last triangle becomes the old facet
			// but first remove this facet from vertices 1 .. val-3
			for (j = 1 ; j <= val - 3 ; j++)
			{
				vertex = &(my_vertices[facet->my_vertIndices[j]]);
				faceIndices.clear();
				for (l = 0 ; l < vertex->my_faceIndices.size(); l++)
				{
					if (vertex->my_faceIndices[l] != i)
					{
						faceIndices.push_back(vertex->my_faceIndices[l]);
					}
				}
				vertex->my_faceIndices = faceIndices;
			}

			fac.my_index = i;
			fac.my_valency = 3;
			fac.my_vertIndices.clear();
			fac.my_vertIndices.push_back(facet->my_vertIndices[0]);
			fac.my_vertIndices.push_back(facet->my_vertIndices[val - 2]);
			fac.my_vertIndices.push_back(facet->my_vertIndices[val - 1]);
			my_facets[i] = fac;
		}
	}

	for (i = 0 ; i < facets.size() ; i++)
	{
		my_facets.push_back(facets[i]);
	}

	my_numF = my_facets.size();

//cout << "Before build" << endl;

	build();


//	for (i = 0 ; i < newMesh->my_numF ; i++)
//	{
//		facet = &(newMesh->my_facets[i]);
//		val = facet->my_valency;
//		cout << i << " " << val << endl;
//	}

//    cout << "Triang end" << endl;

}

void Mesh::triangulate2()
{
	Point_3D			poi;
	unsigned int		i, j, kF, kV, l, val;
	MeshFacet			*facet, fac;
	MeshVertex			*vertex, centroid;
	std::vector <unsigned int> faceIndices;
	std::vector <MeshFacet>  	facets; // new facets
	std::vector <MeshVertex>  	vertices; // new vertices

//    cout << "Triang begin" << endl;

	kF = my_numF;
	kV = my_numV;

	for (i = 0 ; i < my_numF ; i++)
	{
		facet = &(my_facets[i]);
		val = facet->my_valency;
//        if (val > 3)
		{
//cout << i << endl;
			// compute centroid first
			poi = Point_3D(0,0,0);
			for (j = 0 ; j < val ; j++)
			{
				poi = my_vertices[facet->my_vertIndices[j]].my_point + poi;
			}
			poi = (1.0 / val) * poi;
			centroid = MeshVertex();
			centroid.my_point = poi;
			centroid.my_index = kV;
			vertices.push_back(centroid);
			kV++;

//cout << "i: " << i << endl;

			for (j = 0 ; j < val - 1; j++)
			{
				fac.my_index = kF;
				fac.my_valency = 3;
				fac.my_vertIndices.clear();
				fac.my_vertIndices.push_back(facet->my_vertIndices[j]);
				fac.my_vertIndices.push_back(facet->my_vertIndices[j + 1]);
				fac.my_vertIndices.push_back(centroid.my_index);
				facets.push_back(fac);
				centroid.my_faceIndices.push_back(kF);
				my_vertices[facet->my_vertIndices[j]].my_faceIndices.push_back(kF);
				my_vertices[facet->my_vertIndices[j + 1]].my_faceIndices.push_back(kF);
				kF++;
			}
			// last triangle becomes the old facet
			// but first remove this facet from vertices 1 .. val-2
			// and add it to centroid
			for (j = 1 ; j <= val - 2 ; j++)
			{
				vertex = &(my_vertices[facet->my_vertIndices[j]]);
				faceIndices.clear();
				for (l = 0 ; l < vertex->my_faceIndices.size(); l++)
				{
					if (vertex->my_faceIndices[l] != i)
					{
						faceIndices.push_back(vertex->my_faceIndices[l]);
					}
				}
				vertex->my_faceIndices = faceIndices;
			}

			centroid.my_faceIndices.push_back(i);

			fac.my_index = i;
			fac.my_valency = 3;
			fac.my_vertIndices.clear();
			fac.my_vertIndices.push_back(centroid.my_index);
			fac.my_vertIndices.push_back(facet->my_vertIndices[val - 1]);
			fac.my_vertIndices.push_back(facet->my_vertIndices[0]);
			my_facets[i] = fac;
		}
	}

	for (i = 0 ; i < vertices.size() ; i++)
	{
		my_vertices.push_back(vertices[i]);
	}

	for (i = 0 ; i < facets.size() ; i++)
	{
		my_facets.push_back(facets[i]);
	}

	my_numV = my_vertices.size();
	my_numF = my_facets.size();

//cout << "Before build" << endl;



//	for (i = 0 ; i < my_numF ; i++)
//	{
//		facet = &(my_facets[i]);
//		val = facet->my_valency;
//		cout << i << " " << val << endl;
//	}

	build();

//    cout << "Triang2 end" << endl;

}


void Mesh::computeNormalsFlat(void)
{
	unsigned int 	i, j;
	MeshFacet		*facet;
	Point_3D		p0, p1, p2;
	GLfloat			normal[3], n[3], q0[3], q1[3], q2[3];

	for (i = 0 ; i < my_numF ; i++)
	{
		facet = &(my_facets[i]);
		normal[0] = 0;
		normal[1] = 0;
		normal[2] = 0;
		for (j = 0 ; j < facet->my_valency ; j++) // I should simplify this computation
		{
			p0 = facet->my_corners[j].my_vertex->my_point;
			p1 = facet->my_corners[(j + 1) % facet->my_valency].my_vertex->my_point;
			p2 = facet->my_corners[(j + 2) % facet->my_valency].my_vertex->my_point;
			q0[0] = p0.getX();
			q0[1] = p0.getY();
			q0[2] = p0.getZ();
			q1[0] = p1.getX();
			q1[1] = p1.getY();
			q1[2] = p1.getZ();
			q2[0] = p2.getX();
			q2[1] = p2.getY();
			q2[2] = p2.getZ();
			calculateNormal(n, q0, q1, q2);
			normal[0] += n[0];
			normal[1] += n[1];
			normal[2] += n[2];
		}
		facet->my_normalFlat[0] = normal[0];
		facet->my_normalFlat[1] = normal[1];
		facet->my_normalFlat[2] = normal[2];
	}

//cout << "ComputeNormalsFlat called" << endl;

}

void Mesh::computeNormalsSmooth(unsigned int rings)
{
    unsigned int 	i, j;//, k;
    MeshFacet		*facet;//, *fc;
	MeshVertex		*vertex;
    GLfloat			//normal[3],
                    normal2[3], nextNormal[3], normA, normB, dot, dotThreshold;

    dotThreshold = 0.9;

//	std::vector< std::vector <unsigned int> > ring_facets;
//	std::vector< std::vector <unsigned int> > ring_vertices;

//	// smooth normals for facets
//	for (i = 0 ; i < my_numF ; i++)
//	{
//		facet = &(my_facets[i]);

//		if (rings == 0)
//		{
//			facet->my_normalSmooth[0] = facet->my_normalFlat[0];
//			facet->my_normalSmooth[1] = facet->my_normalFlat[1];
//			facet->my_normalSmooth[2] = facet->my_normalFlat[2];
//		}
//		else
//		{
//			normal[0] = 0;
//			normal[1] = 0;
//			normal[2] = 0;
//            getRingsF(facet, rings, &ring_facets, &ring_vertices);
//			for (j = 0 ; j < ring_facets.size() ; j++)
//			{
//				for (k = 0 ; k < ring_facets[j].size() ; k++)
//				{
//					fc = &(my_facets[ring_facets[j][k]]);
//					normal[0] += fc->my_normalFlat[0];
//					normal[1] += fc->my_normalFlat[1];
//					normal[2] += fc->my_normalFlat[2];
//				}
//			}
////			// change contribution of the original facet
////			normal[0] += 0 * facet->my_normalFlat[0];
////			normal[1] += 0 * facet->my_normalFlat[1];
////			normal[2] += 0 * facet->my_normalFlat[2];

//			facet->my_normalSmooth[0] = normal[0];
//			facet->my_normalSmooth[1] = normal[1];
//			facet->my_normalSmooth[2] = normal[2];

////cout << "Flat: " << facet->my_normalFlat[0] << " " << facet->my_normalFlat[1] << " " << facet->my_normalFlat[2] << endl;
////cout << "Smth: " << facet->my_normalSmooth[0] << " " << facet->my_normalSmooth[1] << " " << facet->my_normalSmooth[2] << endl;

//		}
//	}

	// smooth normals for vertices
	for (i = 0 ; i < my_numV ; i++)
	{
		vertex = &(my_vertices[i]);
//		normal[0] = 0;
//		normal[1] = 0;
//		normal[2] = 0;
		normal2[0] = 0;
		normal2[1] = 0;
		normal2[2] = 0;
		for (j = 0 ; j < vertex->my_valency ; j++)
		{
			facet = vertex->my_facets[j];
//			normal[0] += facet->my_normalSmooth[0];
//			normal[1] += facet->my_normalSmooth[1];
//			normal[2] += facet->my_normalSmooth[2];

            nextNormal[0] = facet->my_normalFlat[0];
            nextNormal[1] = facet->my_normalFlat[1];
            nextNormal[2] = facet->my_normalFlat[2];

//            // check for sharp creases; do not average normals across them
//            normA = sqrt(nextNormal[0] * nextNormal[0] + nextNormal[1] * nextNormal[1] + nextNormal[2] * nextNormal[2]);
//            normB = sqrt(normal2[0] * normal2[0] + normal2[1] * normal2[1] + normal2[2] * normal2[2]);

//            if ((normA == 0) || (normB == 0))
//            {
                normal2[0] += facet->my_normalFlat[0];
                normal2[1] += facet->my_normalFlat[1];
                normal2[2] += facet->my_normalFlat[2];
//            }
//            else
//            {
//                dot = (nextNormal[0] * normal2[0] + normal2[1] * normal2[1] + nextNormal[2] * normal2[2]) / normA / normB;
//                if (dot > dotThreshold)
//                {
//                    normal2[0] += facet->my_normalFlat[0];
//                    normal2[1] += facet->my_normalFlat[1];
//                    normal2[2] += facet->my_normalFlat[2];
//                }

//            }
		}
//		vertex->my_normalSmooth[0] = normal[0];
//		vertex->my_normalSmooth[1] = normal[1];
//		vertex->my_normalSmooth[2] = normal[2];
//		vertex->my_normalFlat[0] = normal2[0];
//		vertex->my_normalFlat[1] = normal2[1];
//		vertex->my_normalFlat[2] = normal2[2];

        vertex->my_normalSmooth[0] = normal2[0];
        vertex->my_normalSmooth[1] = normal2[1];
        vertex->my_normalSmooth[2] = normal2[2];
	}
}

void Mesh::calculateNormal(GLfloat *norm, GLfloat p0[3], GLfloat p1[3], GLfloat p2[3])
{
	unsigned int i;
	GLfloat v1[3], v2[3];

	for(i = 0 ; i < 3 ; i++)
	{
		v1[i] = p1[i] - p0[i];
		v2[i] = p2[i] - p0[i];
	}

	norm[0] = v1[1]*v2[2] - v1[2]*v2[1];
	norm[1] = v1[2]*v2[0] - v1[0]*v2[2];
	norm[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

void Mesh::getRingsF(MeshFacet *fac, unsigned int r,
					std::vector< std::vector <unsigned int> > *ring_facets,
					std::vector< std::vector <unsigned int> > *ring_vertices)
{
	MeshVertex							*vertex;
	MeshFacet							*facet;
	unsigned int 						i, j;
	std::vector<unsigned int>			facets;
	std::vector<unsigned int>			vertices;
	std::set<unsigned int>				set_facets;
	std::set<unsigned int>				set_vertices;
	std::set<unsigned int>::iterator	it;
	std::pair<std::set<unsigned int>::iterator, bool> ret;

	ring_facets->clear();
	ring_vertices->clear();
	facets.clear();
	vertices.clear();
	set_facets.clear();
	set_vertices.clear();

	if (r > 0)
	{
		set_facets.insert(fac->my_index);
		facets.push_back(fac->my_index);
		ring_facets->push_back(facets);
		for (i = 0 ; i < r ; i++)
		{
			vertices.clear();
			facets.clear();
			//insert vertices
			for (it = set_facets.begin() ; it != set_facets.end() ; it++)
			{
				facet = &(my_facets[*it]);
				for (j = 0 ; j < facet->my_valency ; j++)
				{
					ret = set_vertices.insert(facet->my_vertIndices[j]);
					if (ret.second)
					{
						vertices.push_back(facet->my_vertIndices[j]);
					}
				}
			}
			ring_vertices->push_back(vertices);
			// insert facets
			for (it = set_vertices.begin() ; it != set_vertices.end() ; it++)
			{
				vertex = &(my_vertices[*it]);
				for (j = 0 ; j < vertex->my_valency ; j++)
				{
					ret = set_facets.insert(vertex->my_faceIndices[j]);
					if (ret.second)
					{
						facets.push_back(vertex->my_faceIndices[j]);
					}
				}
			}
			ring_facets->push_back(facets);
		}
	}
}

void Mesh::getRingsV(MeshVertex *vert, unsigned int r,
					std::vector< std::vector <unsigned int> > *ring_facets,
					std::vector< std::vector <unsigned int> > *ring_vertices)
{
	MeshVertex							*vertex;
	MeshFacet							*facet;
	unsigned int 						i, j;
	std::vector<unsigned int>			facets;
	std::vector<unsigned int>			vertices;
	std::set<unsigned int>				set_facets;
	std::set<unsigned int>				set_vertices;
	std::set<unsigned int>::iterator	it;
	std::pair<std::set<unsigned int>::iterator, bool> ret;

	ring_facets->clear();
	ring_vertices->clear();
	facets.clear();
	vertices.clear();
	set_facets.clear();
	set_vertices.clear();

	if (r > 0)
	{
		set_vertices.insert(vert->my_index);
		vertices.push_back(vert->my_index);
		ring_vertices->push_back(vertices);
		for (i = 0 ; i < r ; i++)
		{
			vertices.clear();
			facets.clear();
			// insert facets
			for (it = set_vertices.begin() ; it != set_vertices.end() ; it++)
			{
				vertex = &(my_vertices[*it]);
				for (j = 0 ; j < vertex->my_valency ; j++)
				{
					ret = set_facets.insert(vertex->my_faceIndices[j]);
					if (ret.second)
					{
						facets.push_back(vertex->my_faceIndices[j]);
					}
				}
			}
			ring_facets->push_back(facets);
			//insert vertices
			for (it = set_facets.begin() ; it != set_facets.end() ; it++)
			{
				facet = &(my_facets[*it]);
				for (j = 0 ; j < facet->my_valency ; j++)
				{
					ret = set_vertices.insert(facet->my_vertIndices[j]);
					if (ret.second)
					{
						vertices.push_back(facet->my_vertIndices[j]);
					}
				}
			}
			ring_vertices->push_back(vertices);
		}
	}
}

//Catmull-Clark subdivision
void Mesh::CatmullClark(Mesh *smesh)
{
    unsigned int 				i, j, k, cnt, num, numCorners, numCreases;
    MeshFacet 					*facet, face;
    MeshVertex					*vertex, vert, *prev0, *prev1, *next0, *next1;
    MeshCorner					*cencrn, *prevcrn, *crn;
    Point_3D					Fpoi, Epoi, Vpoi, Favrg, Eavrg;
//    PointPrec                   x, y, z;
    vector<MeshCorner*>         vecCrn, vecAllCrn, vecCrsCrn;

    cout << "Catmull-Clark start" << endl;

    // compute new face-points
    times("");

    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        Fpoi = Point_3D();

        for (j = 0 ; j < facet->my_valency ; j++)
        {
            Fpoi += facet->my_corners[j].my_vertex->my_point;
        }
        Fpoi *= (1.0 / facet->my_valency);
        facet->my_Fpoi = Fpoi;
    }
    times("Face average loop");

    // compute new edge-points - each only once!
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);

            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                Epoi = Point_3D();
                Epoi += crn->my_vertex->my_point;
                Epoi += crn->my_nextCorner->my_vertex->my_point;
//                if (crn->my_nextFacet == NULL) // OLD, WITHOUT CREASES
                if (crn->my_nextFacet == NULL || crn->my_sharpness != 0 || crn->my_pairedCorner->my_sharpness != 0)
                {
                    Epoi *= 0.5;
                }
                else
                {
                    Epoi += facet->my_Fpoi;
                    Epoi += facet->my_corners[j].my_nextFacet->my_Fpoi;
                    Epoi *= 0.25;
                }
                crn->my_Epoi = Epoi;
            }
        }
    }
    times("Edge average loop");

    // compute new vertex-points
    for (i = 0 ; i < my_numV ; i++)
    {
        vertex = &my_vertices[i];

        //find number of incoming creases
        numCreases = 0;
        vecCrsCrn.clear();
        for (j = 0 ; j < vertex->my_valency ; j ++)
        {
            crn = findCorner(vertex, vertex->my_facets[j]);
            if (crn->my_sharpness > 0)
            {
                numCreases++;
                vecCrsCrn.push_back(crn);
            }
        }

        Vpoi = Point_3D();

        if (vertex->isOnBoundary || numCreases > 1)
        {
            if (vertex->my_valency == 1 || numCreases > 2)
            {
                Vpoi = vertex->my_point;
            }
            else if (vertex->my_valency == 2 || numCreases == 2)
            {
                Vpoi = 6 * vertex->my_point;
                if (numCreases == 2)
                {
                    Vpoi += vecCrsCrn[0]->my_nextCorner->my_vertex->my_point;
                    Vpoi += vecCrsCrn[1]->my_nextCorner->my_vertex->my_point;
                }
                else
                {
                    crn = findCorner(vertex, vertex->my_facets[0]);
                    next0 = crn->my_nextCorner->my_vertex;
                    for (j = 0 ; j < vertex->my_facets[0]->my_valency - 1 ; j++)
                    {
                        crn = crn->my_nextCorner;
                    }
                    prev0 = crn->my_vertex;

                    crn = findCorner(vertex, vertex->my_facets[1]);
                    next1 = crn->my_nextCorner->my_vertex;
                    for (j = 0 ; j < vertex->my_facets[1]->my_valency - 1 ; j++)
                    {
                        crn = crn->my_nextCorner;
                    }
                    prev1 = crn->my_vertex;

                    if (next0->my_index == prev1->my_index)
                    {
                        Vpoi += prev0->my_point + next1->my_point;
                    }
                    else
                    {
                        Vpoi += prev1->my_point + next0->my_point;
                    }
                }
                Vpoi *= 0.125;
            }
            else
            {
//                cout << "Something is wrong at the boundary!" << endl;
//                assert(false); // something is wrong at the boundary!

                // instead of crashing, interpolate the point
                Vpoi = vertex->my_point;
            }
        }
        else
        {
            Favrg = Point_3D();
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Favrg += vertex->my_facets[j]->my_Fpoi;
            }
            Favrg *= (1.0 / vertex->my_valency);

            Eavrg = Point_3D();

            cencrn = findCorner(vertex, vertex->my_facets[0]);
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Eavrg += cencrn->my_nextCorner->my_vertex->my_point;
                cencrn = getSameCorner(cencrn);
            }
            Eavrg *= (1.0 / vertex->my_valency);
            Eavrg += vertex->my_point;
            Eavrg *= 0.5;

            if (vertex->my_valency < 3)
            {
                Vpoi = Point_3D(0,0,0);
            }
            else
            {
                Vpoi = vertex->my_point * (vertex->my_valency - 3);
            }
            Vpoi = Vpoi + Eavrg + Eavrg;
            Vpoi += Favrg;
            if (vertex->my_valency < 3)
            {
                Vpoi *= (1.0 / 3.0);
            }
            else
            {
                Vpoi *= (1.0 / vertex->my_valency);
            }
        }
        vertex->my_Vpoi = Vpoi;
    }
    times("Vertex average loop");

    // copy over new positions of V-vertices
    smesh->my_vertices.clear();
    smesh->my_vertices.resize(my_numV);

    for (i = 0 ; i < my_numV ; i++)
    {
        vert.my_point = my_vertices[i].my_Vpoi;
        vert.my_index = i;
        vert.my_faceIndices.clear();
        vert.isFeature = true;
        smesh->my_vertices[i] = vert;
    }

    // add F-vertices
    smesh->my_vertices.resize(my_numV + my_numF);

    for (i = 0 ; i < my_numF ; i++)
    {
        vert.my_point = my_facets[i].my_Fpoi;
        vert.my_index = my_numV + i;
        vert.isFeature = false;
        smesh->my_vertices[i + my_numV] = vert;
    }
    times("Copy over V- and F-vertices");

    //index E-vertices and add them to vertex list
    //but first collect corners so that new E-vertices can be added in parallel
    vecCrn.clear();
    vecAllCrn.clear();
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);
            vecAllCrn.push_back(crn);
            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                vecCrn.push_back(crn);
            }
        }
    }

    cnt = smesh->my_vertices.size();
    num = vecCrn.size();
    smesh->my_vertices.resize(cnt + num);

    for (i = cnt ; i < cnt + num ; i++)
    {
        crn = vecCrn[i - cnt];
        vert.my_point = crn->my_Epoi;
        vert.my_index = i;
        crn->my_newEindex = i;
        if (crn->my_vertex->isFeature && crn->my_nextCorner->my_vertex->isFeature)
        {
            vert.isFeature = true;
        }
        else
        {
            vert.isFeature = false;
        }
        smesh->my_vertices[i] = vert;
    }

    //create facets
    smesh->my_facets.clear();
    smesh->quadMesh = true;
    numCorners = vecAllCrn.size();
    smesh->my_facets.resize(numCorners);

    smesh->my_sFacets.clear();
    smesh->my_sVertices.clear();
    smesh->my_sValues.clear();

//    #pragma omp parallel for default(none) shared(smesh,numCorners,vecAllCrn), private(i,k,crn,prevcrn,facet,face)
    // THIS DOES NOT WORK IN PARALLEL
    for (i = 0 ; i < numCorners ; i++)
    {
        crn = vecAllCrn[i];
        facet = crn->my_facet;

        face.my_index = i;
        face.my_vertIndices.clear();
        face.my_valency = 4;
//        face.isBlack = facet->isBlack;

        //inherit sharpness for creases
        if (crn->my_sharpness > 0)
        {
            smesh->my_sFacets.push_back(i);
            smesh->my_sVertices.push_back(0);
            smesh->my_sValues.push_back(crn->my_sharpness - 1);
        }

        //add four vertices
        face.my_vertIndices.push_back(crn->my_vertex->my_index);
        smesh->my_vertices[crn->my_vertex->my_index].my_faceIndices.push_back(i);

        if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
        {
            face.my_vertIndices.push_back(crn->my_newEindex);
            smesh->my_vertices[crn->my_newEindex].my_faceIndices.push_back(i);
        }
        else
        {
            face.my_vertIndices.push_back(crn->my_pairedCorner->my_newEindex);
            smesh->my_vertices[crn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(i);
        }

        face.my_vertIndices.push_back(facet->my_index + my_numV);
        smesh->my_vertices[facet->my_index + my_numV].my_faceIndices.push_back(i);

        prevcrn = crn;
        for (k = 0 ; k < facet->my_valency - 1 ; k++)
        {
            prevcrn = prevcrn->my_nextCorner;
        }

        if (prevcrn->my_nextFacet == NULL || facet->my_index < prevcrn->my_nextFacet->my_index) // we are at the boundary or ...
        {
            face.my_vertIndices.push_back(prevcrn->my_newEindex);
            smesh->my_vertices[prevcrn->my_newEindex].my_faceIndices.push_back(i);
        }
        else
        {
            face.my_vertIndices.push_back(prevcrn->my_pairedCorner->my_newEindex);
            smesh->my_vertices[prevcrn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(i);
        }

        smesh->my_facets[i] = face;
    }
    // old serial code
//    for (i = 0 ; i < my_numF ; i++)
//    {
//        facet = &my_facets[i];
//        for (j = 0 ; j < facet->my_valency ; j++)
//        {
//            crn = &(facet->my_corners[j]);

//            face.my_index = cnt;
//            face.my_vertIndices.clear();
//            face.my_valency = 4;

//            //add four vertices
//            face.my_vertIndices.push_back(crn->my_vertex->my_index);
//            smesh->my_vertices[crn->my_vertex->my_index].my_faceIndices.push_back(cnt);

//            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
//            {
//                face.my_vertIndices.push_back(crn->my_newEindex);
//                smesh->my_vertices[crn->my_newEindex].my_faceIndices.push_back(cnt);
//            }
//            else
//            {
//                face.my_vertIndices.push_back(crn->my_pairedCorner->my_newEindex);
//                smesh->my_vertices[crn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
//            }

//            face.my_vertIndices.push_back(facet->my_index + my_numV);
//            smesh->my_vertices[facet->my_index + my_numV].my_faceIndices.push_back(cnt);

//            prevcrn = crn;
//            for (k = 0 ; k < facet->my_valency - 1 ; k++)
//            {
//                prevcrn = prevcrn->my_nextCorner;
//            }

//            if (prevcrn->my_nextFacet == NULL || facet->my_index < prevcrn->my_nextFacet->my_index) // we are at the boundary or ...
//            {
//                face.my_vertIndices.push_back(prevcrn->my_newEindex);
//                smesh->my_vertices[prevcrn->my_newEindex].my_faceIndices.push_back(cnt);
//            }
//            else
//            {
//                face.my_vertIndices.push_back(prevcrn->my_pairedCorner->my_newEindex);
//                smesh->my_vertices[prevcrn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
//            }

//            smesh->my_facets.push_back(face);
//            cnt++;
//        }
//    }
    times("Create facets");

    smesh->my_numV = smesh->my_vertices.size();
    smesh->my_numF = smesh->my_facets.size();
    smesh->my_level = my_level + 1;

    smesh->my_minz = my_minz;
    smesh->my_maxz = my_maxz;

    smesh->isFootBall = isFootBall;
    smesh->useRGB = useRGB;

    cout << "Catmull-Clark end" << endl;

    smesh->build();
}

//Catmull-Clark subdivision with colours
void Mesh::CatmullClarkColour(Mesh *smesh)
{
    unsigned int 				i, j, k, cnt, num, numCorners, numCreases;
    MeshFacet 					*facet, face;
    MeshVertex					*vertex, vert, *prev0, *prev1, *next0, *next1;
    MeshCorner					*cencrn, *prevcrn, *crn;
    Point_3D					Fpoi, Epoi, Vpoi, Favrg, Eavrg, FpoiColour, EpoiColour, VpoiColour, FavrgColour, EavrgColour;
//    PointPrec                   x, y, z;
    vector<MeshCorner*>         vecCrn, vecAllCrn, vecCrsCrn;


    // compute new face-points
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        Fpoi = Point_3D();
        FpoiColour = Point_3D();

        for (j = 0 ; j < facet->my_valency ; j++)
        {
            Fpoi += facet->my_corners[j].my_vertex->my_point;
            FpoiColour += facet->my_corners[j].my_vertex->my_colour;
        }
        Fpoi *= (1.0 / facet->my_valency);
        FpoiColour *= (1.0 / facet->my_valency);
        facet->my_Fpoi = Fpoi;
        facet->my_FpoiColour = FpoiColour;
    }

    // compute new edge-points - each only once!
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);

            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                Epoi = Point_3D();
                EpoiColour = Point_3D();
                Epoi += crn->my_vertex->my_point;
                Epoi += crn->my_nextCorner->my_vertex->my_point;
                EpoiColour += crn->my_vertex->my_colour;
                EpoiColour += crn->my_nextCorner->my_vertex->my_colour;

//                if (crn->my_nextFacet == NULL) // OLD, WITHOUT CREASES
                if (crn->my_nextFacet == NULL || crn->my_sharpness != 0 || crn->my_pairedCorner->my_sharpness != 0)
                {
                    Epoi *= 0.5;
                    EpoiColour *= 0.5;
                }
                else
                {
                    Epoi += facet->my_Fpoi;
                    Epoi += facet->my_corners[j].my_nextFacet->my_Fpoi;
                    Epoi *= 0.25;

                    EpoiColour += facet->my_FpoiColour;
                    EpoiColour += facet->my_corners[j].my_nextFacet->my_FpoiColour;
                    EpoiColour *= 0.25;
                }
                crn->my_Epoi = Epoi;
                crn->my_EpoiColour = EpoiColour;
            }
        }
    }

    // compute new vertex-points
    for (i = 0 ; i < my_numV ; i++)
    {
        vertex = &my_vertices[i];

        //find number of incoming creases
        numCreases = 0;
        vecCrsCrn.clear();
        for (j = 0 ; j < vertex->my_valency ; j ++)
        {
            crn = findCorner(vertex, vertex->my_facets[j]);
            if (crn->my_sharpness > 0)
            {
                numCreases++;
                vecCrsCrn.push_back(crn);
            }
        }

        Vpoi = Point_3D();
        VpoiColour = Point_3D();

        if(vertex->isOnBoundary && vertex->isFeature)
        {
            Vpoi = vertex->my_point;
            VpoiColour = vertex->my_colour;
        }
        else if (vertex->isOnBoundary || numCreases > 1)
        {
            if (vertex->my_valency == 1 || numCreases > 2)
            {
                Vpoi = vertex->my_point;
                VpoiColour = vertex->my_colour;
            }
            else if (vertex->my_valency == 2 || numCreases == 2)
            {
                Vpoi = 6 * vertex->my_point;
                VpoiColour = 6 * vertex->my_colour;
                if (numCreases == 2)
                {
                    Vpoi += vecCrsCrn[0]->my_nextCorner->my_vertex->my_point;
                    Vpoi += vecCrsCrn[1]->my_nextCorner->my_vertex->my_point;

                    VpoiColour += vecCrsCrn[0]->my_nextCorner->my_vertex->my_colour;
                    VpoiColour += vecCrsCrn[1]->my_nextCorner->my_vertex->my_colour;
                }
                else
                {
                    crn = findCorner(vertex, vertex->my_facets[0]);
                    next0 = crn->my_nextCorner->my_vertex;
                    for (j = 0 ; j < vertex->my_facets[0]->my_valency - 1 ; j++)
                    {
                        crn = crn->my_nextCorner;
                    }
                    prev0 = crn->my_vertex;

                    crn = findCorner(vertex, vertex->my_facets[1]);
                    next1 = crn->my_nextCorner->my_vertex;
                    for (j = 0 ; j < vertex->my_facets[1]->my_valency - 1 ; j++)
                    {
                        crn = crn->my_nextCorner;
                    }
                    prev1 = crn->my_vertex;

                    if (next0->my_index == prev1->my_index)
                    {
                        Vpoi += prev0->my_point + next1->my_point;
                        VpoiColour += prev0->my_colour + next1->my_colour;
                    }
                    else
                    {
                        Vpoi += prev1->my_point + next0->my_point;
                        VpoiColour += prev1->my_colour + next0->my_colour;
                    }
                }
                Vpoi *= 0.125;
                VpoiColour *= 0.125;
            }
            else
            {
//                cout << "Something is wrong at the boundary!" << endl;
//                assert(false); // something is wrong at the boundary!

                // instead of crashing, interpolate the point
                Vpoi = vertex->my_point;
                VpoiColour = vertex->my_colour;
            }
        }
        else
        {
            Favrg = Point_3D();
            FavrgColour = Point_3D();
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Favrg += vertex->my_facets[j]->my_Fpoi;
                FavrgColour += vertex->my_facets[j]->my_FpoiColour;
            }
            Favrg *= (1.0 / vertex->my_valency);
            FavrgColour *= (1.0 / vertex->my_valency);

            Eavrg = Point_3D();
            EavrgColour = Point_3D();

            cencrn = findCorner(vertex, vertex->my_facets[0]);
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Eavrg += cencrn->my_nextCorner->my_vertex->my_point;
                EavrgColour += cencrn->my_nextCorner->my_vertex->my_colour;
                cencrn = getSameCorner(cencrn);
            }
            Eavrg *= (1.0 / vertex->my_valency);
            Eavrg += vertex->my_point;
            Eavrg *= 0.5;

            EavrgColour *= (1.0 / vertex->my_valency);
            EavrgColour += vertex->my_colour;
            EavrgColour *= 0.5;

            if (vertex->my_valency < 3)
            {
                Vpoi = Point_3D(0,0,0);
                VpoiColour = Point_3D(0,0,0);
            }
            else
            {
                Vpoi = vertex->my_point * (vertex->my_valency - 3);
                VpoiColour = vertex->my_colour * (vertex->my_valency - 3);
            }
            Vpoi = Vpoi + Eavrg + Eavrg;
            Vpoi += Favrg;
            VpoiColour = VpoiColour + EavrgColour + EavrgColour;
            VpoiColour += FavrgColour;
            if (vertex->my_valency < 3)
            {
                Vpoi *= (1.0 / 3.0);
                VpoiColour *= (1.0 / 3.0);
            }
            else
            {
                Vpoi *= (1.0 / vertex->my_valency);
                VpoiColour *= (1.0 / vertex->my_valency);
            }
        }
        vertex->my_Vpoi = Vpoi;
        vertex->my_VpoiColour = VpoiColour;

    }

    // copy over new positions of V-vertices
    smesh->my_vertices.clear();
    smesh->my_vertices.resize(my_numV);

    for (i = 0 ; i < my_numV ; i++)
    {
        vert.my_point = my_vertices[i].my_Vpoi;
        vert.my_colour = my_vertices[i].my_VpoiColour;
        vert.isFeature = my_vertices[i].isFeature;
        vert.my_index = i;
        vert.my_faceIndices.clear();            
        smesh->my_vertices[i] = vert;
    }

    // add F-vertices
    smesh->my_vertices.resize(my_numV + my_numF);

    for (i = 0 ; i < my_numF ; i++)
    {
        vert.my_point = my_facets[i].my_Fpoi;
        vert.my_colour = my_facets[i].my_FpoiColour;
        vert.my_index = my_numV + i;
        smesh->my_vertices[i + my_numV] = vert;
    }

    //index E-vertices and add them to vertex list
    //but first collect corners so that new E-vertices can be added in parallel
    vecCrn.clear();
    vecAllCrn.clear();
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);
            vecAllCrn.push_back(crn);
            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                vecCrn.push_back(crn);
            }
        }
    }

    cnt = smesh->my_vertices.size();
    num = vecCrn.size();
    smesh->my_vertices.resize(cnt + num);

    for (i = cnt ; i < cnt + num ; i++)
    {
        crn = vecCrn[i - cnt];
        vert.my_point = crn->my_Epoi;
        vert.my_colour = crn->my_EpoiColour;
        vert.my_index = i;
        crn->my_newEindex = i;
        smesh->my_vertices[i] = vert;
    }

    //create facets
    smesh->my_facets.clear();
    smesh->quadMesh = true;
    numCorners = vecAllCrn.size();
    smesh->my_facets.resize(numCorners);

    smesh->my_sFacets.clear();
    smesh->my_sVertices.clear();
    smesh->my_sValues.clear();

//    #pragma omp parallel for default(none) shared(smesh,numCorners,vecAllCrn), private(i,k,crn,prevcrn,facet,face)
    // THIS DOES NOT WORK IN PARALLEL
    for (i = 0 ; i < numCorners ; i++)
    {
        crn = vecAllCrn[i];
        facet = crn->my_facet;

        face.my_index = i;
        face.my_vertIndices.clear();
        face.my_valency = 4;

        //inherit sharpness for creases
        if (crn->my_sharpness > 0)
        {
            smesh->my_sFacets.push_back(i);
            smesh->my_sVertices.push_back(0);
            smesh->my_sValues.push_back(crn->my_sharpness - 1);
        }

        //add four vertices
        face.my_vertIndices.push_back(crn->my_vertex->my_index);
        smesh->my_vertices[crn->my_vertex->my_index].my_faceIndices.push_back(i);

        if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
        {
            face.my_vertIndices.push_back(crn->my_newEindex);
            smesh->my_vertices[crn->my_newEindex].my_faceIndices.push_back(i);
        }
        else
        {
            face.my_vertIndices.push_back(crn->my_pairedCorner->my_newEindex);
            smesh->my_vertices[crn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(i);
        }

        face.my_vertIndices.push_back(facet->my_index + my_numV);
        smesh->my_vertices[facet->my_index + my_numV].my_faceIndices.push_back(i);

        prevcrn = crn;
        for (k = 0 ; k < facet->my_valency - 1 ; k++)
        {
            prevcrn = prevcrn->my_nextCorner;
        }

        if (prevcrn->my_nextFacet == NULL || facet->my_index < prevcrn->my_nextFacet->my_index) // we are at the boundary or ...
        {
            face.my_vertIndices.push_back(prevcrn->my_newEindex);
            smesh->my_vertices[prevcrn->my_newEindex].my_faceIndices.push_back(i);
        }
        else
        {
            face.my_vertIndices.push_back(prevcrn->my_pairedCorner->my_newEindex);
            smesh->my_vertices[prevcrn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(i);
        }

        smesh->my_facets[i] = face;
    }

    smesh->my_numV = smesh->my_vertices.size();
    smesh->my_numF = smesh->my_facets.size();
    smesh->my_level = my_level + 1;

    smesh->isFootBall = isFootBall;

    smesh->my_minz = my_minz;
    smesh->my_maxz = my_maxz;
    smesh->interpFlat = false;
    smesh->useRGB = useRGB;

    smesh->build();
}

void Mesh::CatmullClarkIntFlat(Mesh *smesh)
{
    unsigned int 				i, j, k, cnt, num, numCorners, numCreases;
    MeshFacet 					*facet, face;
    MeshVertex					*vertex, vert, *prev0, *prev1, *next0, *next1;
    MeshCorner					*cencrn, *prevcrn, *crn, *crn2, *crn3;
    Point_3D					Fpoi, Epoi, Vpoi, Favrg, Eavrg, FpoiColour, EpoiColour, VpoiColour, FavrgColour, EavrgColour;
//    PointPrec                   x, y, z;
    vector<MeshCorner*>         vecCrn, vecAllCrn, vecCrsCrn;
    std::vector <MeshVertex>    vertices_bck;
    if(my_level==0) vertices_bck = my_vertices;

    // compute new face-points
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        Fpoi = Point_3D();
        FpoiColour = Point_3D();

        /****************** INSERT FACE POINTS USING BILINEAR INTERP ****************/
        if(my_level == 1) {
            Point_3D points[4];
            float weights_tmp[2],oldZ=Fpoi.getZ();
            for (j = 0 ; j < facet->my_valency ; j++)
            {
                if(facet->my_corners[j].my_vertex->isFeature) {
                    crn2 = &facet->my_corners[j];
                    points[1] = crn2->my_vertex->my_point;
                    points[2] = crn2->my_nextCorner->my_vertex->my_point;
                    weights_tmp[1] = getWeight(crn2->my_vIndex,crn2->my_nIndex);
                } else if(facet->my_corners[j].my_nextCorner->my_vertex->isFeature) {
                    crn2 = facet->my_corners[j].my_nextCorner;
                    points[0] = facet->my_corners[j].my_vertex->my_point;
                    weights_tmp[0] = getWeight(crn2->my_vIndex,facet->my_corners[j].my_vIndex);
                } else if(facet->my_corners[j].my_nextCorner->my_nextCorner->my_vertex->isFeature)
                    points[3] = facet->my_corners[j].my_vertex->my_point;
                FpoiColour += facet->my_corners[j].my_vertex->my_colour;
            }
            float w1 = weights_tmp[0], w2 = weights_tmp[1];
            Fpoi = w1*w2 * points[1] + w1*(1-w2) * points[2];
            Fpoi += (1-w1)*(1-w2) * points[3];
            Fpoi += w2*(1-w1) * points[0];

            // set Z, the control weight, the "normal" way
            if(xyWeights) {
                Fpoi.setZ(oldZ);
                for (j = 0 ; j < facet->my_valency ; j++)
                    Fpoi.setZ(Fpoi.getZ() + facet->my_corners[j].my_vertex->my_point.getZ());
                Fpoi.setZ(Fpoi.getZ() * (1.0/facet->my_valency));
            }
        } else
        {
            for (j = 0 ; j < facet->my_valency ; j++)
            {
                Fpoi += facet->my_corners[j].my_vertex->my_point;
                FpoiColour += facet->my_corners[j].my_vertex->my_colour;
            }
            Fpoi *= (1.0 / facet->my_valency);
        }
        FpoiColour *= (1.0 / facet->my_valency);
        facet->my_Fpoi = Fpoi;
        facet->my_FpoiColour = FpoiColour;
    }

    //index E-vertices and add them to vertex list
    //but first collect corners so that new E-vertices can be added in parallel
    vecCrn.clear();
    vecAllCrn.clear();
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);
            vecAllCrn.push_back(crn);
            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                vecCrn.push_back(crn);
            }
        }
    }

    // compute new edge-points - each only once!
    for (i = 0 ; i < vecCrn.size() ; i++)
    {
        crn = vecCrn[i];
        facet = crn->my_facet;

        Epoi = Point_3D();
        EpoiColour = Point_3D();
        Epoi += crn->my_vertex->my_point;
        Epoi += crn->my_nextCorner->my_vertex->my_point;
        EpoiColour += crn->my_vertex->my_colour;
        EpoiColour += crn->my_nextCorner->my_vertex->my_colour;

        // ******************** CHANGE: CHANGE WEIGHT IDs ***********************
        if(my_level==0) {
            setWeightId(crn->my_vIndex,crn->my_nIndex,i+my_numV+my_numF);
            if (crn->my_nextFacet == NULL)
                setWeightId(crn->my_nextCorner->my_vIndex,crn->my_vIndex,i+my_numV+my_numF);
            if (crn->my_pairedCorner != NULL)
                setWeightId(crn->my_pairedCorner->my_vIndex,crn->my_vIndex,i+my_numV+my_numF);
        }

                if (crn->my_nextFacet == NULL || crn->my_sharpness != 0 || crn->my_pairedCorner->my_sharpness != 0)
                {
                    //******************** ADD WEIGHT *************************
                    if(my_level==1) {
                        float weight, oldZ = Epoi.getZ();
                        if (!crn->my_vertex->isFeature) {
                            crn2 = crn->my_nextCorner;
                            crn3 = crn;
                        } else {
                            crn2 = crn;
                            crn3 = crn2->my_nextCorner;
                        }
                        weight = getWeight(crn2->my_vIndex,crn3->my_vIndex);
                        Epoi = Point_3D();
                        Epoi += weight*crn2->my_vertex->my_point;
                        Epoi += (1-weight)*crn3->my_vertex->my_point;
                        Epoi.setZ(oldZ*.5);
                    } else
                        Epoi *= 0.5;
                    EpoiColour *= 0.5;
                }
                else
                {
                    EpoiColour += facet->my_FpoiColour;
                    EpoiColour += crn->my_nextFacet->my_FpoiColour;
                    EpoiColour *= 0.25;
                    if(my_level==1) {
                        float w1,w2,oldZ=Epoi.getZ();
                        bool crn_next = false;
                        for(j = 0; j < facet->my_valency; j++) {
                            if(facet->my_corners[j].my_vertex->isFeature) {
                                crn2 = &facet->my_corners[j];
                                int id_other;
                                if(crn2->my_nextCorner->my_vIndex == crn->my_vIndex &&
                                        crn->my_nextCorner->my_vIndex == crn2->my_nextCorner->my_nextCorner->my_vIndex) {
                                    id_other = crn->my_nextCorner->my_nextCorner->my_vIndex;
                                    crn_next = true;
                                } else //if(crn2->my_nextCorner->my_nextCorner->my_vIndex == crn->my_vIndex)
                                    id_other = crn2->my_nextCorner->my_vIndex;
                                w1 = getWeight(crn2->my_vIndex,id_other);
                                break;
                            }
                        }
                        facet = crn->my_nextFacet;

                        for(j = 0; j < facet->my_valency; j++) {
                            if(facet->my_corners[j].my_vertex->isFeature) {
                                unsigned int id_next;
                                int id_other;
                                if(crn_next)
                                    id_next = crn->my_vIndex;
                                else
                                    id_next = crn->my_nIndex;
                                crn2 = &facet->my_corners[j];
                                if(crn2->my_nIndex == id_next)
                                    id_other = crn2->my_nextCorner->my_nextCorner->my_nIndex;
                                else
                                    id_other = crn2->my_nIndex;
                                w2 = getWeight(crn2->my_vIndex,id_other);
                                break;
                            }
                        }

                        float weight = (w1+w2)/2;
                        if(crn_next)
                            Epoi = weight*crn->my_vertex->my_point + (1-weight)*crn->my_nextCorner->my_vertex->my_point;
                        else
                            Epoi = (1-weight)*crn->my_vertex->my_point + weight*crn->my_nextCorner->my_vertex->my_point;

                        if(xyWeights) {
                            Epoi.setZ(oldZ+crn->my_facet->my_Fpoi.getZ()+crn->my_nextFacet->my_Fpoi.getZ());
                            Epoi.setZ(Epoi.getZ()*.25);
                        }
                    } else {
                        Epoi += facet->my_Fpoi;
                        Epoi += crn->my_nextFacet->my_Fpoi;
                        Epoi *= 0.25;
                    }
                }
                crn->my_Epoi = Epoi;
                crn->my_EpoiColour = EpoiColour;
    }

    // compute new vertex-points
    for (i = 0 ; i < my_numV ; i++)
    {
        vertex = &my_vertices[i];

        //find number of incoming creases
        numCreases = 0;
        vecCrsCrn.clear();
        for (j = 0 ; j < vertex->my_valency ; j ++)
        {
            crn = findCorner(vertex, vertex->my_facets[j]);
            if (crn->my_sharpness > 0)
            {
                numCreases++;
                vecCrsCrn.push_back(crn);
            }
        }

        Vpoi = Point_3D();
        VpoiColour = Point_3D();

        if (vertex->isOnBoundary || numCreases > 1)
        {
            if (vertex->my_valency == 1 || numCreases > 2)
            {
                Vpoi = vertex->my_point;
                VpoiColour = vertex->my_colour;
            }
            else if (vertex->my_valency == 2 || numCreases == 2)
            {
                Vpoi = 6 * vertex->my_point;
                VpoiColour = 6 * vertex->my_colour;
                if (numCreases == 2)
                {
                    Vpoi += vecCrsCrn[0]->my_nextCorner->my_vertex->my_point;
                    Vpoi += vecCrsCrn[1]->my_nextCorner->my_vertex->my_point;

                    VpoiColour += vecCrsCrn[0]->my_nextCorner->my_vertex->my_colour;
                    VpoiColour += vecCrsCrn[1]->my_nextCorner->my_vertex->my_colour;
                }
                else
                {
                    crn = findCorner(vertex, vertex->my_facets[0]);
                    next0 = crn->my_nextCorner->my_vertex;
                    for (j = 0 ; j < vertex->my_facets[0]->my_valency - 1 ; j++)
                    {
                        crn = crn->my_nextCorner;
                    }
                    prev0 = crn->my_vertex;

                    crn = findCorner(vertex, vertex->my_facets[1]);
                    next1 = crn->my_nextCorner->my_vertex;
                    for (j = 0 ; j < vertex->my_facets[1]->my_valency - 1 ; j++)
                    {
                        crn = crn->my_nextCorner;
                    }
                    prev1 = crn->my_vertex;

                    if (next0->my_index == prev1->my_index)
                    {
                        Vpoi += prev0->my_point + next1->my_point;
                        VpoiColour += prev0->my_colour + next1->my_colour;
                    }
                    else
                    {
                        Vpoi += prev1->my_point + next0->my_point;
                        VpoiColour += prev1->my_colour + next0->my_colour;
                    }
                }
                Vpoi *= 0.125;
                VpoiColour *= 0.125;
            }
            else
            {
                // instead of crashing, interpolate the point
                Vpoi = vertex->my_point;
                VpoiColour = vertex->my_colour;
            }
        }
        else
        {
            Favrg = Point_3D();
            FavrgColour = Point_3D();
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Favrg += vertex->my_facets[j]->my_Fpoi;
                FavrgColour += vertex->my_facets[j]->my_FpoiColour;
            }
            Favrg *= (1.0 / vertex->my_valency);
            FavrgColour *= (1.0 / vertex->my_valency);

            Eavrg = Point_3D();
            EavrgColour = Point_3D();

            cencrn = findCorner(vertex, vertex->my_facets[0]);
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Eavrg += cencrn->my_nextCorner->my_vertex->my_point;
                EavrgColour += cencrn->my_nextCorner->my_vertex->my_colour;
                cencrn = getSameCorner(cencrn);
            }
            Eavrg *= (1.0 / vertex->my_valency);
            Eavrg += vertex->my_point;
            Eavrg *= 0.5;

            EavrgColour *= (1.0 / vertex->my_valency);
            EavrgColour += vertex->my_colour;
            EavrgColour *= 0.5;

            if (vertex->my_valency < 3)
            {
                Vpoi = Point_3D(0,0,0);
                VpoiColour = Point_3D(0,0,0);
            }
            else
            {
                Vpoi = vertex->my_point * (vertex->my_valency - 3);
                VpoiColour = vertex->my_colour * (vertex->my_valency - 3);
            }
            Vpoi = Vpoi + Eavrg + Eavrg;
            Vpoi += Favrg;
            VpoiColour = VpoiColour + EavrgColour + EavrgColour;
            VpoiColour += FavrgColour;
            if (vertex->my_valency < 3)
            {
                Vpoi *= (1.0 / 3.0);
                VpoiColour *= (1.0 / 3.0);
            }
            else
            {
                Vpoi *= (1.0 / vertex->my_valency);
                VpoiColour *= (1.0 / vertex->my_valency);
            }
        }
        vertex->my_Vpoi = Vpoi;
        vertex->my_VpoiColour = VpoiColour;

        if(my_level <= 1 && vertex->isFeature)
            vertex->my_VpoiColour = vertex->my_colour;
    }

    // copy over new positions of V-vertices
    smesh->my_vertices.clear();
    smesh->my_vertices.resize(my_numV);
    smesh->my_vertP1.clear();

    for (i = 0 ; i < my_numV ; i++)
    {
        vert.my_point = my_vertices[i].my_Vpoi;
        vert.my_colour = my_vertices[i].my_VpoiColour;
        vert.my_index = i;
        vert.my_faceIndices.clear();
        if (my_vertices[i].isFeature) {
            vert.isFeature = true;
            if(my_level<2) {
                vert.weights = my_vertices[i].weights;
                vert.weight_ids = my_vertices[i].weight_ids;
            }
        }
        else vert.isFeature = false;


        // *************** FLATNESS CHANGE ********************
        if (my_level == 1 && my_vertices[i].isVertP1)
            smesh->my_vertP1.push_back(i);

        smesh->my_vertices[i] = vert;
    }

    // add F-vertices
    smesh->my_vertices.resize(my_numV + my_numF);

    for (i = 0 ; i < my_numF ; i++)
    {
        vert.my_point = my_facets[i].my_Fpoi;
        vert.my_colour = my_facets[i].my_FpoiColour;
        vert.my_index = my_numV + i;
        vert.isFeature = false;
        smesh->my_vertices[i + my_numV] = vert;
    }

    cnt = smesh->my_vertices.size();
    num = vecCrn.size();
    smesh->my_vertices.resize(cnt + num);

    for (i = cnt ; i < cnt + num ; i++)
    {
        crn = vecCrn[i - cnt];
        vert.my_point = crn->my_Epoi;
        vert.my_colour = crn->my_EpoiColour;
        vert.my_index = i;

        // *************** FLATNESS CHANGE ********************
        if(my_level == 0)
            vert.isVertP1 = true;
        else
            vert.isVertP1 = false;

        crn->my_newEindex = i;
        if (crn->my_vertex->isFeature && crn->my_nextCorner->my_vertex->isFeature)
        {
           vert.isFeature = false;
        }
        else
        {
            vert.isFeature = false;
        }
        smesh->my_vertices[i] = vert;
    }

    //create facets
    smesh->my_facets.clear();
    smesh->quadMesh = true;
    numCorners = vecAllCrn.size();
    smesh->my_facets.resize(numCorners);

    smesh->my_sFacets.clear();
    smesh->my_sVertices.clear();
    smesh->my_sValues.clear();

//    #pragma omp parallel for default(none) shared(smesh,numCorners,vecAllCrn), private(i,k,crn,prevcrn,facet,face)
    // THIS DOES NOT WORK IN PARALLEL
    for (i = 0 ; i < numCorners ; i++)
    {
        crn = vecAllCrn[i];
        facet = crn->my_facet;

        face.my_index = i;
        face.my_vertIndices.clear();
        face.my_valency = 4;

        //inherit sharpness for creases
        if (crn->my_sharpness > 0)
        {
            smesh->my_sFacets.push_back(i);
            smesh->my_sVertices.push_back(0);
            smesh->my_sValues.push_back(crn->my_sharpness - 1);
        }

        //add four vertices
        face.my_vertIndices.push_back(crn->my_vertex->my_index);
        smesh->my_vertices[crn->my_vertex->my_index].my_faceIndices.push_back(i);

        if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
        {
            face.my_vertIndices.push_back(crn->my_newEindex);
            smesh->my_vertices[crn->my_newEindex].my_faceIndices.push_back(i);
        }
        else
        {
            face.my_vertIndices.push_back(crn->my_pairedCorner->my_newEindex);
            smesh->my_vertices[crn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(i);
        }

        face.my_vertIndices.push_back(facet->my_index + my_numV);
        smesh->my_vertices[facet->my_index + my_numV].my_faceIndices.push_back(i);

        prevcrn = crn;
        for (k = 0 ; k < facet->my_valency - 1 ; k++)
        {
            prevcrn = prevcrn->my_nextCorner;
        }

        if (prevcrn->my_nextFacet == NULL || facet->my_index < prevcrn->my_nextFacet->my_index) // we are at the boundary or ...
        {
            face.my_vertIndices.push_back(prevcrn->my_newEindex);
            smesh->my_vertices[prevcrn->my_newEindex].my_faceIndices.push_back(i);
        }
        else
        {
            face.my_vertIndices.push_back(prevcrn->my_pairedCorner->my_newEindex);
            smesh->my_vertices[prevcrn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(i);
        }

        smesh->my_facets[i] = face;
    }

    smesh->my_numV = smesh->my_vertices.size();
    smesh->my_numF = smesh->my_facets.size();
    smesh->my_level = my_level + 1;

    smesh->isFootBall = isFootBall;

    smesh->my_minz = my_minz;
    smesh->my_maxz = my_maxz;
    smesh->interpFlat = true;
    if(my_level==0) my_vertices = vertices_bck;

    smesh->build();
}

float Mesh::getWeight(int vIndex, int target)
{
    for(int i = 0; i < my_vertices[vIndex].weight_ids.size(); i++)
        if(my_vertices[vIndex].weight_ids[i] == target)
            return my_vertices[vIndex].weights[i];
    return -1; // should not happen
}

Point_3D *Mesh::getWeightVec(int vIndex, int target)
{
    for(int i = 0; i < my_vertices[vIndex].weight_ids.size(); i++)
        if(my_vertices[vIndex].weight_ids[i] == target)
            return &my_vertices[vIndex].weights_vec[i];
    return NULL; // should not happen
}

int Mesh::getSharpness(int vIndex, int target)
{
    for(int i = 0; i < my_vertices[vIndex].weight_ids.size(); i++)
        if(my_vertices[vIndex].weight_ids[i] == target)
            return my_vertices[vIndex].sharpness[i];
    return 0; // should not happen
}

void Mesh::setWeightId(int vIndex, int target, int value)
{
    for(int i = 0; i < my_vertices[vIndex].weight_ids.size(); i++)
        if(my_vertices[vIndex].weight_ids[i] == target) {
            my_vertices[vIndex].weight_ids[i] = value;
            break;
        }
}

void Mesh::LinearTernarySubdiv(Mesh *smesh)
{
    unsigned int                 i, j, cnt, num, val;
    MeshFacet                     *facet, face;
    MeshVertex                    vert;
    MeshCorner                    *crn;
    vector<MeshCorner*>         vecCrn, vecAllCrn;

    //collect corners
    vecCrn.clear();
    vecAllCrn.clear();
    smesh->quadMesh = true;
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        if (facet->my_valency != 4)
        {
            smesh->quadMesh = false;
        }
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);
            vecAllCrn.push_back(crn);
            if (crn->my_nextFacet == NULL || facet->my_index <
                    crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                vecCrn.push_back(crn);
            }
        }
    }

    // compute new points
    for (i = 0 ; i < vecAllCrn.size() ; i++)
    {
        crn = vecAllCrn[i];

        // get centroid
        facet = crn->my_facet;
        Point_3D C;
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            C += facet->my_corners[j].my_vertex->my_point;
        }
        C *= (1.0 / facet->my_valency);

        // get weights
        float d1,d2,dv1LEN,dv2LEN;
        Point_3D *dv1,*dv2;
        MeshCorner *crn2 = crn->my_nextCorner;
        if(v3) {
          dv1 = getWeightVec(crn->my_vIndex,crn->my_nIndex);
          for (j = 0; j < facet->my_valency-2; j++)
              crn2 = crn2->my_nextCorner;
          dv2 = getWeightVec(crn->my_vIndex, crn2->my_vIndex);
          dv1LEN = dv1->dist(Point_3D(0.0,0.0,0.0));
          dv2LEN = dv2->dist(Point_3D(0.0,0.0,0.0));
        } else {
            d1 = getWeight(crn->my_vIndex,crn->my_nIndex);
            for (j = 0; j < facet->my_valency-2; j++)
                crn2 = crn2->my_nextCorner;
            d2 = getWeight(crn->my_vIndex, crn2->my_vIndex);
        }

        // set Fpoi
        Point_3D M1,M2;
        if(v3) {
            Point_3D mid1 = (crn->my_vertex->my_point+crn->my_nextCorner->my_vertex->my_point)*0.5,
                     mid2 = (crn->my_vertex->my_point+crn2->my_vertex->my_point)*0.5;
            Point_3D dv1N = ((*dv1)*(1/dv1LEN));
            Point_3D dv2N = ((*dv2)*(1/dv2LEN));
            M1 = crn->my_vertex->my_point + dv1N * mid1.dist(crn->my_vertex->my_point);
            M2 = crn->my_vertex->my_point + dv2N * mid2.dist(crn->my_vertex->my_point);
            d1 = dv1LEN / mid1.dist(crn->my_vertex->my_point);
            d2 = dv2LEN / mid2.dist(crn->my_vertex->my_point);
        } else {
            M1 = (crn->my_vertex->my_point+crn->my_nextCorner->my_vertex->my_point)*0.5;
            M2 = (crn->my_vertex->my_point+crn2->my_vertex->my_point)*0.5;
        }
        crn->my_Fpoi = d1*d2*C + (1-d1)*(1-d2)*crn->my_vertex->my_point +
                       d1*(1-d2)*M1 + d2*(1-d1)*M2;
        crn->my_FpoiColour = crn->my_vertex->my_colour;

        // set Epois
        crn->my_Epoi0 = (1-d1)*crn->my_vertex->my_point + d1*M1;
        if(v3) {
            dv1 = getWeightVec(crn->my_nIndex,crn->my_vIndex);
            Point_3D mid1 = (crn->my_vertex->my_point+crn->my_nextCorner->my_vertex->my_point)*0.5;
            dv1LEN = dv1->dist(Point_3D(0.0,0.0,0.0));
            Point_3D dv1N = ((*dv1)*(1/dv1LEN));
            M1 = crn->my_nextCorner->my_vertex->my_point + dv1N * mid1.dist(crn->my_vertex->my_point);
            d1 = dv1LEN / mid1.dist(crn->my_nextCorner->my_vertex->my_point);
        } else d1 = getWeight(crn->my_nIndex,crn->my_vIndex);
        crn->my_Epoi1 = (1-d1)*crn->my_nextCorner->my_vertex->my_point + d1*M1;
        crn->my_Epoi0Colour = crn->my_vertex->my_colour;
        crn->my_Epoi1Colour = crn->my_nextCorner->my_vertex->my_colour;
    }

    // copy over positions of V-vertices
    smesh->my_vertices.clear();
    smesh->my_vertices.resize(my_numV);

    for (i = 0 ; i < my_numV ; i++)
    {
        vert.my_point = my_vertices[i].my_point;
        vert.my_colour = my_vertices[i].my_colour;
        vert.my_index = i;
        vert.my_faceIndices.clear();
        if(vert.isOnBoundary)
        {
            vert.isFeature = true;
        }
        smesh->my_vertices[i] = vert;
    }

    // add E- and F-vertices
    num = vecCrn.size();
    cnt = my_numV;
//    #pragma omp parallel for default(none) shared(smesh,num,vecCrn)

    for (i = 0 ; i < num ; i++)
    {
        crn = vecCrn[i];

        // add edge point 0
        vert.my_point = crn->my_Epoi0;
        vert.my_colour = crn->my_Epoi0Colour;
        vert.my_index = cnt;
        vert.my_faceIndices.clear();
        vert.isFeature = false;
        smesh->my_vertices.push_back(vert);
        cnt++;

        //add edge point 1
        vert.my_point = crn->my_Epoi1;
        vert.my_colour = crn->my_Epoi1Colour;
        vert.my_index = cnt;
        vert.my_faceIndices.clear();
        vert.isFeature = false;
        smesh->my_vertices.push_back(vert);
        cnt++;

        //add face point
        vert.my_point = crn->my_Fpoi;
        vert.my_colour = crn->my_FpoiColour;
        vert.my_index = cnt;
        vert.my_faceIndices.clear();
        vert.isFeature = false;
        smesh->my_vertices.push_back(vert);
        cnt++;

        //store new indices for face creation
        crn->my_newE0index = cnt - 3;
        crn->my_newE1index = cnt - 2;
        crn->my_newFindex =  cnt - 1;
        if (crn->my_pairedCorner != NULL)
        {
            crn = crn->my_pairedCorner;
            crn->my_newE0index = cnt - 2; // note that these are swapped
            crn->my_newE1index = cnt - 3;

            //add face point
            vert.my_point = crn->my_Fpoi;
            vert.my_colour = crn->my_FpoiColour;
            vert.my_index = cnt;
            vert.my_faceIndices.clear();
            vert.isFeature = false;
            smesh->my_vertices.push_back(vert);
            crn->my_newFindex = cnt;
            cnt++;
        }
    }

    //create facets
    smesh->my_facets.clear();

    smesh->my_sFacets.clear();
    smesh->my_sVertices.clear();
    smesh->my_sValues.clear();

    cnt = 0;
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);

            // add face0
            face.my_index = cnt;
            face.my_vertIndices.clear();
            face.my_valency = 4;
            //inherit sharpness for creases
            if (crn->my_sharpness > 0)
            {
                smesh->my_sFacets.push_back(cnt);
                smesh->my_sVertices.push_back(0);
                smesh->my_sValues.push_back(crn->my_sharpness - 1);
            }
            //add vertices
            face.my_vertIndices.push_back(crn->my_newE0index);
            smesh->my_vertices[crn->my_newE0index].my_faceIndices.push_back(cnt);

            face.my_vertIndices.push_back(crn->my_newE1index);
            smesh->my_vertices[crn->my_newE1index].my_faceIndices.push_back(cnt);

            face.my_vertIndices.push_back(crn->my_nextCorner->my_newFindex);
            smesh->my_vertices[crn->my_nextCorner->my_newFindex].my_faceIndices.push_back(cnt);

            face.my_vertIndices.push_back(crn->my_newFindex);
            smesh->my_vertices[crn->my_newFindex].my_faceIndices.push_back(cnt);
            smesh->my_facets.push_back(face);
            cnt++;

            // add face1
            face.my_index = cnt;
            face.my_vertIndices.clear();
            face.my_valency = 4;
            //inherit sharpness for creases
            if (crn->my_sharpness > 0)
            {
                smesh->my_sFacets.push_back(cnt);
                smesh->my_sVertices.push_back(0);
                smesh->my_sValues.push_back(crn->my_sharpness - 1);
            }
            //add vertices
            face.my_vertIndices.push_back(crn->my_newE1index);
            smesh->my_vertices[crn->my_newE1index].my_faceIndices.push_back(cnt);

            face.my_vertIndices.push_back(crn->my_nextCorner->my_vertex->my_index);
            smesh->my_vertices[crn->my_nextCorner->my_vertex->my_index].my_faceIndices.push_back(cnt);

            face.my_vertIndices.push_back(crn->my_nextCorner->my_newE0index);
            smesh->my_vertices[crn->my_nextCorner->my_newE0index].my_faceIndices.push_back(cnt);

            face.my_vertIndices.push_back(crn->my_nextCorner->my_newFindex);
            smesh->my_vertices[crn->my_nextCorner->my_newFindex].my_faceIndices.push_back(cnt);
            smesh->my_facets.push_back(face);
            cnt++;
        }

        // add central face
        face.my_index = cnt;
        face.my_vertIndices.clear();
        face.my_valency = facet->my_valency;
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);
            //add vertex
            face.my_vertIndices.push_back(crn->my_newFindex);
            smesh->my_vertices[crn->my_newFindex].my_faceIndices.push_back(cnt);
        }
        smesh->my_facets.push_back(face);
        cnt++;
    }
    smesh->my_numV = smesh->my_vertices.size();
    smesh->my_numF = smesh->my_facets.size();
    smesh->my_level = my_level + 1;

    smesh->my_minz = my_minz;
    smesh->my_maxz = my_maxz;
    smesh->interpFlat = false;

    smesh->isFootBall = isFootBall;
    smesh->useRGB = useRGB;

    smesh->build();
}

//Bounded curvature Catmull-Clark subdivision
void Mesh::CatmullClarkBounded(Mesh *smesh)
{
    unsigned int 				i, j, k, cnt, val;
    MeshFacet 					*facet, face;
    MeshVertex					*vertex, vert, *prev0, *prev1, *next0, *next1;
    MeshCorner					*cencrn, *prevcrn, *crn;
    Point_3D					Fpoi, Epoi, Vpoi, Favrg, Eavrg;
    PointPrec                   sum;

    cout << "Catmull-Clark start" << endl;

    // compute new face-points
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        Fpoi = Point_3D();

        if (facet->my_valency != 4)
        {
            for (j = 0 ; j < facet->my_valency ; j++)
            {
                Fpoi += facet->my_corners[j].my_vertex->my_point;
            }
            Fpoi *= (1.0 / facet->my_valency);
        }
        else // regular valency
        {
            sum = 0;
            for (j = 0 ; j < 4 ; j++)
            {
                val = facet->my_corners[j].my_vertex->my_valency;
                if (val > 12)
                {
                    val = 12;
                }
                if (val < 3)
                {
                    val = 3;
                }
                Fpoi += ccW[val - 3] * facet->my_corners[j].my_vertex->my_point;
                sum += ccW[val - 3];
            }
            Fpoi *= (1.0 / sum);
        }
        facet->my_Fpoi = Fpoi;
    }

    // compute new edge-points - each only once!
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);

            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                Epoi = Point_3D();
                if (crn->my_nextFacet == NULL)
                {
                    Epoi += crn->my_vertex->my_point;
                    Epoi += crn->my_nextCorner->my_vertex->my_point;
                    Epoi *= 0.5;
                }
                else
                {
                    sum = 0;
                    val = crn->my_vertex->my_valency;
                    if (val > 12)
                    {
                        val = 12;
                    }
                    if (val < 3)
                    {
                        val = 3;
                    }
                    Epoi += ccW[val - 3] * crn->my_vertex->my_point;
                    sum += ccW[val - 3];

                    val = crn->my_nextCorner->my_vertex->my_valency;
                    if (val > 12)
                    {
                        val = 12;
                    }
                    if (val < 3)
                    {
                        val = 3;
                    }
                    Epoi += ccW[val - 3] * crn->my_nextCorner->my_vertex->my_point;
                    sum += ccW[val - 3];

                    val = facet->my_valency;
                    if (val > 12)
                    {
                        val = 12;
                    }
                    if (val < 3)
                    {
                        val = 3;
                    }
                    Epoi += ccW[val - 3] * facet->my_Fpoi;
                    sum += ccW[val - 3];

                    val = facet->my_corners[j].my_nextFacet->my_valency;
                    if (val > 12)
                    {
                        val = 12;
                    }
                    if (val < 3)
                    {
                        val = 3;
                    }
                    Epoi += ccW[val - 3] * facet->my_corners[j].my_nextFacet->my_Fpoi;
                    sum += ccW[val - 3];

                    Epoi *= (1.0 / sum);
                }
                crn->my_Epoi = Epoi;
                if (crn->my_pairedCorner != NULL)
                {
                    crn->my_pairedCorner->my_Epoi = Epoi; // used later for Epoi averaging
                }
            }
        }
    }

    // compute new vertex-points
    for (i = 0 ; i < my_numV ; i++)
    {
        vertex = &my_vertices[i];
        Vpoi = Point_3D();

        if (vertex->isOnBoundary)
        {
            if (vertex->my_valency == 1)
            {
                Vpoi = vertex->my_point;
            }
            else if (vertex->my_valency == 2) // use cubic B-spline stencil
            {
                Vpoi = 6 * vertex->my_point;
                crn = findCorner(vertex, vertex->my_facets[0]);
                next0 = crn->my_nextCorner->my_vertex;
                for (j = 0 ; j < vertex->my_facets[0]->my_valency - 1 ; j++)
                {
                    crn = crn->my_nextCorner;
                }
                prev0 = crn->my_vertex;

                crn = findCorner(vertex, vertex->my_facets[1]);
                next1 = crn->my_nextCorner->my_vertex;
                for (j = 0 ; j < vertex->my_facets[1]->my_valency - 1 ; j++)
                {
                    crn = crn->my_nextCorner;
                }
                prev1 = crn->my_vertex;

                if (next0->my_index == prev1->my_index)
                {
                    Vpoi += prev0->my_point + next1->my_point;
                }
                else
                {
                    Vpoi += prev1->my_point + next0->my_point;
                }
                Vpoi *= 0.125;
            }
            else
            {
                cout << "Something is wrong at the boundary!" << endl;
//                assert(false); // something is wrong at the boundary!

                // instead of crashing, interpolate the point
                Vpoi = vertex->my_point;
            }
        }
        else
        {
            Favrg = Point_3D();
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Favrg += vertex->my_facets[j]->my_Fpoi;
            }
            Favrg *= (1.0 / vertex->my_valency);

            Eavrg = Point_3D();
            cencrn = findCorner(vertex, vertex->my_facets[0]);
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Eavrg += cencrn->my_Epoi; // this works only if pairedCorners are assigned Epois as well (see above).
//                Eavrg += cencrn->my_Epoi;
//                Eavrg += cencrn->my_pairedCorner->my_Epoi;
                cencrn = getSameCorner(cencrn);
            }
            Eavrg *= (1.0 / vertex->my_valency);

            val = vertex->my_valency;

            if (val > 12)
            {
                val = 12;
            }
            if (val < 3)
            {
                val = 3;
            }
            Vpoi = ccA[val - 3] * vertex->my_point;
            Vpoi += ccB[val - 3] * Eavrg;
            Vpoi += ccC[val - 3] * Favrg;

//            sum = ccA[val - 3]+ccB[val - 3]+ccC[val - 3];
        }
        vertex->my_Vpoi = Vpoi;
    }

    // copy over new positions of V-vertices
    smesh->my_vertices.clear();
    for (i = 0 ; i < my_numV ; i++)
    {
        vert.my_point = my_vertices[i].my_Vpoi;
        vert.my_index = i;
        vert.my_faceIndices.clear();
        vert.isFeature = true;
        smesh->my_vertices.push_back(vert);
    }
    // add F-vertices
    for (i = 0 ; i < my_numF ; i++)
    {
        vert.my_point = my_facets[i].my_Fpoi;
        vert.my_index = my_numV + i;
        vert.isFeature = false;
        smesh->my_vertices.push_back(vert);
    }

    //index E-vertices and add them to vertex list
    cnt = smesh->my_vertices.size();
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);

            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                vert.my_point = crn->my_Epoi;
                vert.my_index = cnt;
                crn->my_newEindex = cnt;
                if (crn->my_vertex->isFeature && crn->my_nextCorner->my_vertex->isFeature)
                {
                    vert.isFeature = true;
                }
                else
                {
                    vert.isFeature = false;
                }
                smesh->my_vertices.push_back(vert);
                cnt++;
            }
        }
    }

    //create facets
    smesh->my_facets.clear();
    smesh->quadMesh = true;
    cnt = 0;
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);

            face.my_index = cnt;
            face.my_vertIndices.clear();
            face.my_valency = 4;

            //add four vertices
            face.my_vertIndices.push_back(crn->my_vertex->my_index);
            smesh->my_vertices[crn->my_vertex->my_index].my_faceIndices.push_back(cnt);

            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                face.my_vertIndices.push_back(crn->my_newEindex);
                smesh->my_vertices[crn->my_newEindex].my_faceIndices.push_back(cnt);
            }
            else
            {
                face.my_vertIndices.push_back(crn->my_pairedCorner->my_newEindex);
                smesh->my_vertices[crn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
            }

            face.my_vertIndices.push_back(facet->my_index + my_numV);
            smesh->my_vertices[facet->my_index + my_numV].my_faceIndices.push_back(cnt);

            prevcrn = crn;
            for (k = 0 ; k < facet->my_valency - 1 ; k++)
            {
                prevcrn = prevcrn->my_nextCorner;
            }

            if (prevcrn->my_nextFacet == NULL || facet->my_index < prevcrn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                face.my_vertIndices.push_back(prevcrn->my_newEindex);
                smesh->my_vertices[prevcrn->my_newEindex].my_faceIndices.push_back(cnt);
            }
            else
            {
                face.my_vertIndices.push_back(prevcrn->my_pairedCorner->my_newEindex);
                smesh->my_vertices[prevcrn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
            }

            smesh->my_facets.push_back(face);
            cnt++;
        }
    }

    smesh->my_numV = smesh->my_vertices.size();
    smesh->my_numF = smesh->my_facets.size();
    smesh->my_level = my_level + 1;

    smesh->my_minz = my_minz;
    smesh->my_maxz = my_maxz;
    smesh->useRGB = useRGB;

    cout << "Catmull-Clark end" << endl;

    smesh->build();
}

//Interpolatory Catmull-Clark subdivision (based on Li and Zheng, 2012, CAGD: An alternative method for constructing interpolatory subdivision from approximating subdivision
void Mesh::CatmullClarkInt(Mesh *smesh)
{
    unsigned int 				i, j, k, cnt, val;
    MeshFacet 					*facet, face;
    MeshVertex					*vertex, vert;
    MeshCorner					*cencrn, *prevcrn, *crn;
    Point_3D					Fpoi, Epoi, Vpoi, Favrg, Eavrg, Dpoi, Cij, Cijsum, Disum;
    PointPrec                   alpha;
    std::vector<Point_3D>       Cijs;

    cout << "Interpolatory Catmull-Clark start" << endl;

    //test that mesh has no boundary
    if (hasBoundary)
    {
        // boundary -> cannot handle it now
        cout << "The mesh is not closed -> cannot use CCint now!" << endl;
        smesh->my_numV = 0;
    }
    else
    {
        // compute fake vertex-points
        for (i = 0 ; i < my_numV ; i++)
        {
            vertex = &my_vertices[i];
            val = vertex->my_valency;
            Vpoi = Point_3D();
            vertex->my_fakeD.clear();
            vertex->my_n.clear();

            if (vertex->isOnBoundary)
            {
                if (vertex->my_valency == 1)
                {
                    Vpoi = vertex->my_point;
                }
                else if (vertex->my_valency == 2)
                {
                    // DO SOMETHING CLEVER HERE, but do not forget to assign faceContr and edgeContr
                    Vpoi = vertex->my_point;
                }
                else
                {
                    cout << "Something is wrong at the boundary!" << endl;
    //                assert(false); // something is wrong at the boundary!

                    // instead of crashing, interpolate the point
                    Vpoi = vertex->my_point;
                }
            }
            else
            {
                // compute fake D points
                cencrn = findCorner(vertex, vertex->my_facets[0]);
                alpha = 0;
                for (j = 0 ; j < val ; j++)
                {
                    //collect n[i] - facet valencies - 3
                    vertex->my_n.push_back(cencrn->my_facet->my_valency - 3);
                    alpha += 1.0 / (float)(vertex->my_n.back() + 3.0);

                    Dpoi = Point_3D();
                    Dpoi = 1.5 * cencrn->my_nextCorner->my_vertex->my_point;
                    Dpoi += -0.25 * cencrn->my_nextCorner->my_nextCorner->my_vertex->my_point;
                    cencrn = getSameCorner(cencrn);
                    prevcrn = cencrn;
                    for (k = 0; k < cencrn->my_facet->my_valency - 2 ; k++)
                    {
                        prevcrn = prevcrn->my_nextCorner;
                    }
                    Dpoi += -0.25 * prevcrn->my_vertex->my_point;
                    vertex->my_fakeD.push_back(Dpoi);
                }

                //finish computing alpha
                alpha *= 4.0 / (float)(val * (val + 5.0));
                alpha += (float)(val - 1.0) / (float)(val + 5.0);

                // sum C_{i,j}
                Cijsum = Point_3D();
                cencrn = findCorner(vertex, vertex->my_facets[0]);
                Cijs.clear();

                for (j = 0 ; j < val ; j++)
                {
                    Cij = Point_3D();
                    crn = cencrn->my_nextCorner;
                    for (k = 0; k < vertex->my_n[j] ; k++)
                    {
                        crn = crn->my_nextCorner;
                        Cij += crn->my_vertex->my_point;
                    }
                    // remember the Cij value since it's needed below for facets
                    Cijs.push_back(Cij);

                    Cij *= 1.0 / (float)(vertex->my_n[j] + 3.0);
                    cencrn = getSameCorner(cencrn);
                    Cijsum += Cij;
                }

                // sum D_i
                Disum = Point_3D();
                for (j = 0 ; j < val ; j++)
                {
//                    Disum += (1.0 / (float)(vertex->my_n[j] + 3.0) + 1.0 / (float)(vertex->my_n[(j + 1) % val] + 3.0)) * vertex->my_fakeD[j];
                    Disum += (1.0 + 2.0 / (float)(vertex->my_n[j] + 3.0) + 2.0 / (float)(vertex->my_n[(j + 1) % val] + 3.0)) * vertex->my_fakeD[j];
                }

                // final fake vertex
//                Vpoi = (-4.0 / (float)(val * (val + 5.0) * alpha)) * (Disum + Cijsum);
//                Vpoi += (1.0 / alpha) * vertex->my_point;
                Vpoi = Disum + (2 * Cijsum);
                Vpoi *= (-2.0 / (float)(val * (val + 5.0)));
                Vpoi += vertex->my_point;
                Vpoi *= (1.0 / alpha);

                // compute contribution of Vpoi (C) to each incident facet (fi) and edge (ei)
                cencrn = findCorner(vertex, vertex->my_facets[0]);
                for (j = 0 ; j < val ; j++)
                {
                    facet = cencrn->my_facet;
                    facet->my_faceContr.push_back(1.0 / ((float)vertex->my_n[j] + 3.0) * (Vpoi + vertex->my_fakeD[j] + Cijs[j] + vertex->my_fakeD[(j + val - 1) % val]));

                    cencrn->my_edgeContr = 1.0 / 3.0 * Vpoi + 1.0 / 2.0 * cencrn->my_nextCorner->my_vertex->my_point +
                                           1.0 / 12.0 * (vertex->my_fakeD[(j + val - 1) % val] + vertex->my_fakeD[(j + 1) % val]);
                    cencrn = getSameCorner(cencrn);
                }
            }
            vertex->my_Vpoi = Vpoi;
        }

        // compute face-points
        for (i = 0 ; i < my_numF ; i++)
        {
            facet = &my_facets[i];
            Fpoi = Point_3D();
            for (j = 0 ; j < facet->my_valency ; j++)
            {
                Fpoi += facet->my_faceContr[j];
            }
            Fpoi *= (1.0 / facet->my_valency);
            facet->my_Fpoi = Fpoi;
        }

        // compute new edge-points - each only once!
        for (i = 0 ; i < my_numF ; i++)
        {
            facet = &my_facets[i];
            for (j = 0 ; j < facet->my_valency ; j++)
            {
                crn = &(facet->my_corners[j]);

                if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
                {
                    Epoi = Point_3D();
                    if (crn->my_nextFacet == NULL)
                    {
                        // do something clever at the boundary
                    }
                    else
                    {
                        Epoi = crn->my_edgeContr;
                        Epoi += crn->my_pairedCorner->my_edgeContr;
                        Epoi *= 0.5;
                    }
                    crn->my_Epoi = Epoi;
                }
            }
        }

        // copy over OLD positions of V-vertices
        smesh->my_vertices.clear();
        for (i = 0 ; i < my_numV ; i++)
        {
            vert.my_point = my_vertices[i].my_point;
            vert.my_index = i;
            vert.my_faceIndices.clear();
            vert.isFeature = true;
            smesh->my_vertices.push_back(vert);
        }
        // add F-vertices
        for (i = 0 ; i < my_numF ; i++)
        {
            vert.my_point = my_facets[i].my_Fpoi;
            vert.my_index = my_numV + i;
            vert.isFeature = false;
            smesh->my_vertices.push_back(vert);
        }

        //index E-vertices and add them to vertex list
        cnt = smesh->my_vertices.size();
        for (i = 0 ; i < my_numF ; i++)
        {
            facet = &my_facets[i];
            for (j = 0 ; j < facet->my_valency ; j++)
            {
                crn = &(facet->my_corners[j]);

                if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
                {
                    vert.my_point = crn->my_Epoi;
                    vert.my_index = cnt;
                    crn->my_newEindex = cnt;
                    if (crn->my_vertex->isFeature && crn->my_nextCorner->my_vertex->isFeature)
                    {
                        vert.isFeature = true;
                    }
                    else
                    {
                        vert.isFeature = false;
                    }
                    smesh->my_vertices.push_back(vert);
                    cnt++;
                }
            }
        }

        //create facets
        smesh->my_facets.clear();
        smesh->quadMesh = true;
        cnt = 0;
        for (i = 0 ; i < my_numF ; i++)
        {
            facet = &my_facets[i];
            for (j = 0 ; j < facet->my_valency ; j++)
            {
                crn = &(facet->my_corners[j]);

                face.my_index = cnt;
                face.my_vertIndices.clear();
                face.my_valency = 4;

                //add four vertices
                face.my_vertIndices.push_back(crn->my_vertex->my_index);
                smesh->my_vertices[crn->my_vertex->my_index].my_faceIndices.push_back(cnt);

                if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
                {
                    face.my_vertIndices.push_back(crn->my_newEindex);
                    smesh->my_vertices[crn->my_newEindex].my_faceIndices.push_back(cnt);
                }
                else
                {
                    face.my_vertIndices.push_back(crn->my_pairedCorner->my_newEindex);
                    smesh->my_vertices[crn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
                }

                face.my_vertIndices.push_back(facet->my_index + my_numV);
                smesh->my_vertices[facet->my_index + my_numV].my_faceIndices.push_back(cnt);

                prevcrn = crn;
                for (k = 0 ; k < facet->my_valency - 1 ; k++)
                {
                    prevcrn = prevcrn->my_nextCorner;
                }

                if (prevcrn->my_nextFacet == NULL || facet->my_index < prevcrn->my_nextFacet->my_index) // we are at the boundary or ...
                {
                    face.my_vertIndices.push_back(prevcrn->my_newEindex);
                    smesh->my_vertices[prevcrn->my_newEindex].my_faceIndices.push_back(cnt);
                }
                else
                {
                    face.my_vertIndices.push_back(prevcrn->my_pairedCorner->my_newEindex);
                    smesh->my_vertices[prevcrn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
                }

                smesh->my_facets.push_back(face);
                cnt++;
            }
        }

        smesh->my_numV = smesh->my_vertices.size();
        smesh->my_numF = smesh->my_facets.size();
        smesh->my_level = my_level + 1;

        smesh->isFootBall = isFootBall;

        smesh->my_minz = my_minz;
        smesh->my_maxz = my_maxz;

        cout << "Catmull-Clark end" << endl;

        smesh->build();
    }
}

//Interpolatory Catmull-Clark subdivision with colour (based on Li and Zheng, 2012, CAGD: An alternative method for constructing interpolatory subdivision from approximating subdivision
void Mesh::CatmullClarkIntColour(Mesh *smesh)
{
    unsigned int 				i, j, k, cnt, val;
    MeshFacet 					*facet, face;
    MeshVertex					*vertex, vert;
    MeshCorner					*cencrn, *prevcrn, *crn;
    Point_3D					Fpoi, Epoi, Vpoi, Dpoi, Cij, Cijsum, Disum, FpoiColour, EpoiColour, VpoiColour, DpoiColour, CijColour, CijsumColour, DisumColour;
    PointPrec                   alpha;
    std::vector<Point_3D>       Cijs, CijsColour;

    cout << "Interpolatory Catmull-Clark COLOUR start" << endl;

    //test that mesh has no boundary
    if (hasBoundary)
    {
        // boundary -> cannot handle it now
        cout << "The mesh is not closed -> cannot use CCint now!" << endl;
        smesh->my_numV = 0;
    }
    else
    {
        // compute fake vertex-points
        for (i = 0 ; i < my_numV ; i++)
        {
            vertex = &my_vertices[i];
            val = vertex->my_valency;
            Vpoi = Point_3D();
            VpoiColour = Point_3D();
            vertex->my_fakeD.clear();
            vertex->my_fakeDColour.clear();
            vertex->my_n.clear();

            if (vertex->isOnBoundary)
            {
                if (vertex->my_valency == 1)
                {
                    Vpoi = vertex->my_point;
                    VpoiColour = vertex->my_colour;
                }
                else if (vertex->my_valency == 2)
                {
                    // DO SOMETHING CLEVER HERE, but do not forget to assign faceContr and edgeContr
                    Vpoi = vertex->my_point;
                    VpoiColour = vertex->my_colour;
                }
                else
                {
                    cout << "Something is wrong at the boundary!" << endl;
    //                assert(false); // something is wrong at the boundary!

                    // instead of crashing, interpolate the point
                    Vpoi = vertex->my_point;
                    VpoiColour = vertex->my_colour;
                }
            }
            else
            {
                // compute fake D points
                cencrn = findCorner(vertex, vertex->my_facets[0]);
                alpha = 0;
                for (j = 0 ; j < val ; j++)
                {
                    //collect n[i] - facet valencies - 3
                    vertex->my_n.push_back(cencrn->my_facet->my_valency - 3);
                    alpha += 1.0 / (float)(vertex->my_n.back() + 3.0);

                    Dpoi = Point_3D();
                    Dpoi = 1.5 * cencrn->my_nextCorner->my_vertex->my_point;
                    Dpoi += -0.25 * cencrn->my_nextCorner->my_nextCorner->my_vertex->my_point;

                    DpoiColour = Point_3D();
                    DpoiColour = 1.5 * cencrn->my_nextCorner->my_vertex->my_colour;
                    DpoiColour += -0.25 * cencrn->my_nextCorner->my_nextCorner->my_vertex->my_colour;

                    cencrn = getSameCorner(cencrn);
                    prevcrn = cencrn;
                    for (k = 0; k < cencrn->my_facet->my_valency - 2 ; k++)
                    {
                        prevcrn = prevcrn->my_nextCorner;
                    }
                    Dpoi += -0.25 * prevcrn->my_vertex->my_point;
                    DpoiColour += -0.25 * prevcrn->my_vertex->my_colour;
                    vertex->my_fakeD.push_back(Dpoi);
                    vertex->my_fakeDColour.push_back(DpoiColour);
                }

                //finish computing alpha
                alpha *= 4.0 / (float)(val * (val + 5.0));
                alpha += (float)(val - 1.0) / (float)(val + 5.0);

                // sum C_{i,j}
                Cijsum = Point_3D();
                CijsumColour = Point_3D();
                cencrn = findCorner(vertex, vertex->my_facets[0]);
                Cijs.clear();
                CijsColour.clear();

                for (j = 0 ; j < val ; j++)
                {
                    Cij = Point_3D();
                    CijColour = Point_3D();
                    crn = cencrn->my_nextCorner;
                    for (k = 0; k < vertex->my_n[j] ; k++)
                    {
                        crn = crn->my_nextCorner;
                        Cij += crn->my_vertex->my_point;
                        CijColour += crn->my_vertex->my_colour;
                    }
                    // remember the Cij value since it's needed below for facets
                    Cijs.push_back(Cij);
                    CijsColour.push_back(CijColour);

                    Cij *= 1.0 / (float)(vertex->my_n[j] + 3.0);
                    CijColour *= 1.0 / (float)(vertex->my_n[j] + 3.0);
                    cencrn = getSameCorner(cencrn);
                    Cijsum += Cij;
                    CijsumColour += CijColour;
                }

                // sum D_i
                Disum = Point_3D();
                DisumColour = Point_3D();
                for (j = 0 ; j < val ; j++)
                {
//                    Disum += (1.0 / (float)(vertex->my_n[j] + 3.0) + 1.0 / (float)(vertex->my_n[(j + 1) % val] + 3.0)) * vertex->my_fakeD[j];
                    Disum += (1.0 + 2.0 / (float)(vertex->my_n[j] + 3.0) + 2.0 / (float)(vertex->my_n[(j + 1) % val] + 3.0)) * vertex->my_fakeD[j];
                    DisumColour += (1.0 + 2.0 / (float)(vertex->my_n[j] + 3.0) + 2.0 / (float)(vertex->my_n[(j + 1) % val] + 3.0)) * vertex->my_fakeDColour[j];
                }

                // final fake vertex
//                Vpoi = (-4.0 / (float)(val * (val + 5.0) * alpha)) * (Disum + Cijsum);
//                Vpoi += (1.0 / alpha) * vertex->my_point;
                Vpoi = Disum + (2 * Cijsum);
                Vpoi *= (-2.0 / (float)(val * (val + 5.0)));
                Vpoi += vertex->my_point;
                Vpoi *= (1.0 / alpha);

                VpoiColour = DisumColour + (2 * CijsumColour);
                VpoiColour *= (-2.0 / (float)(val * (val + 5.0)));
                VpoiColour += vertex->my_colour;
                VpoiColour *= (1.0 / alpha);

                // compute contribution of Vpoi (C) to each incident facet (fi) and edge (ei)
                cencrn = findCorner(vertex, vertex->my_facets[0]);
                for (j = 0 ; j < val ; j++)
                {
                    facet = cencrn->my_facet;
                    facet->my_faceContr.push_back(1.0 / ((float)vertex->my_n[j] + 3.0) * (Vpoi + vertex->my_fakeD[j] + Cijs[j] + vertex->my_fakeD[(j + val - 1) % val]));
                    facet->my_faceContrColour.push_back(1.0 / ((float)vertex->my_n[j] + 3.0) * (VpoiColour + vertex->my_fakeDColour[j] + CijsColour[j] + vertex->my_fakeDColour[(j + val - 1) % val]));

                    cencrn->my_edgeContr = 1.0 / 3.0 * Vpoi + 1.0 / 2.0 * cencrn->my_nextCorner->my_vertex->my_point +
                                           1.0 / 12.0 * (vertex->my_fakeD[(j + val - 1) % val] + vertex->my_fakeD[(j + 1) % val]);
                    cencrn->my_edgeContrColour = 1.0 / 3.0 * VpoiColour + 1.0 / 2.0 * cencrn->my_nextCorner->my_vertex->my_colour +
                                           1.0 / 12.0 * (vertex->my_fakeDColour[(j + val - 1) % val] + vertex->my_fakeDColour[(j + 1) % val]);
                    cencrn = getSameCorner(cencrn);
                }
            }
            vertex->my_Vpoi = Vpoi;
            vertex->my_VpoiColour = VpoiColour;
        }

        // compute face-points
        for (i = 0 ; i < my_numF ; i++)
        {
            facet = &my_facets[i];
            Fpoi = Point_3D();
            FpoiColour = Point_3D();
            for (j = 0 ; j < facet->my_valency ; j++)
            {
                Fpoi += facet->my_faceContr[j];
                FpoiColour += facet->my_faceContrColour[j];
            }
            Fpoi *= (1.0 / facet->my_valency);
            FpoiColour *= (1.0 / facet->my_valency);
            facet->my_Fpoi = Fpoi;
            facet->my_FpoiColour = FpoiColour;
        }

        // compute new edge-points - each only once!
        for (i = 0 ; i < my_numF ; i++)
        {
            facet = &my_facets[i];
            for (j = 0 ; j < facet->my_valency ; j++)
            {
                crn = &(facet->my_corners[j]);

                if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
                {
                    Epoi = Point_3D();
                    EpoiColour = Point_3D();
                    if (crn->my_nextFacet == NULL)
                    {
                        // do something clever at the boundary
                    }
                    else
                    {
                        Epoi = crn->my_edgeContr;
                        Epoi += crn->my_pairedCorner->my_edgeContr;
                        Epoi *= 0.5;

                        EpoiColour = crn->my_edgeContrColour;
                        EpoiColour += crn->my_pairedCorner->my_edgeContrColour;
                        EpoiColour *= 0.5;
                    }
                    crn->my_Epoi = Epoi;
                    crn->my_EpoiColour = EpoiColour;
                }
            }
        }

        // copy over OLD positions of V-vertices
        smesh->my_vertices.clear();
        for (i = 0 ; i < my_numV ; i++)
        {
            vert.my_point = my_vertices[i].my_point;
            vert.my_colour = my_vertices[i].my_colour;
            vert.my_index = i;
            vert.my_faceIndices.clear();
            vert.isFeature = true;
            smesh->my_vertices.push_back(vert);
        }
        // add F-vertices
        for (i = 0 ; i < my_numF ; i++)
        {
            vert.my_point = my_facets[i].my_Fpoi;
            vert.my_colour = my_facets[i].my_FpoiColour;
            vert.my_index = my_numV + i;
            vert.isFeature = false;
            smesh->my_vertices.push_back(vert);
        }

        //index E-vertices and add them to vertex list
        cnt = smesh->my_vertices.size();
        for (i = 0 ; i < my_numF ; i++)
        {
            facet = &my_facets[i];
            for (j = 0 ; j < facet->my_valency ; j++)
            {
                crn = &(facet->my_corners[j]);

                if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
                {
                    vert.my_point = crn->my_Epoi;
                    vert.my_colour = crn->my_EpoiColour;
                    vert.my_index = cnt;
                    crn->my_newEindex = cnt;
                    if (crn->my_vertex->isFeature && crn->my_nextCorner->my_vertex->isFeature)
                    {
                        vert.isFeature = true;
                    }
                    else
                    {
                        vert.isFeature = false;
                    }
                    smesh->my_vertices.push_back(vert);
                    cnt++;
                }
            }
        }

        //create facets
        smesh->my_facets.clear();
        smesh->quadMesh = true;
        cnt = 0;
        for (i = 0 ; i < my_numF ; i++)
        {
            facet = &my_facets[i];
            for (j = 0 ; j < facet->my_valency ; j++)
            {
                crn = &(facet->my_corners[j]);

                face.my_index = cnt;
                face.my_vertIndices.clear();
                face.my_valency = 4;

                //add four vertices
                face.my_vertIndices.push_back(crn->my_vertex->my_index);
                smesh->my_vertices[crn->my_vertex->my_index].my_faceIndices.push_back(cnt);

                if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
                {
                    face.my_vertIndices.push_back(crn->my_newEindex);
                    smesh->my_vertices[crn->my_newEindex].my_faceIndices.push_back(cnt);
                }
                else
                {
                    face.my_vertIndices.push_back(crn->my_pairedCorner->my_newEindex);
                    smesh->my_vertices[crn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
                }

                face.my_vertIndices.push_back(facet->my_index + my_numV);
                smesh->my_vertices[facet->my_index + my_numV].my_faceIndices.push_back(cnt);

                prevcrn = crn;
                for (k = 0 ; k < facet->my_valency - 1 ; k++)
                {
                    prevcrn = prevcrn->my_nextCorner;
                }

                if (prevcrn->my_nextFacet == NULL || facet->my_index < prevcrn->my_nextFacet->my_index) // we are at the boundary or ...
                {
                    face.my_vertIndices.push_back(prevcrn->my_newEindex);
                    smesh->my_vertices[prevcrn->my_newEindex].my_faceIndices.push_back(cnt);
                }
                else
                {
                    face.my_vertIndices.push_back(prevcrn->my_pairedCorner->my_newEindex);
                    smesh->my_vertices[prevcrn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
                }

                smesh->my_facets.push_back(face);
                cnt++;
            }
        }

        smesh->my_numV = smesh->my_vertices.size();
        smesh->my_numF = smesh->my_facets.size();
        smesh->my_level = my_level + 1;

        smesh->isFootBall = isFootBall;

        smesh->my_minz = my_minz;
        smesh->my_maxz = my_maxz;

        cout << "Catmull-Clark Int COLOUR end" << endl;

        smesh->build();
    }
}

//Conformal subdivision
void Mesh::Conformal(Mesh *smesh)
{
//    unsigned int 				i, j, k, cnt, val, valEV, posi, posj, xx, yy, aa, bb;
//    MeshFacet 					*facet, face;
//    MeshVertex					*vertex, vert, *EV, *prev0, *prev1, *next0, *next1;
//    MeshCorner					*cencrn, *prevcrn, *crn, *inCrn, *crnj;
//    Point_3D					Fpoi, Epoi, Vpoi, Favrg, Eavrg, tmp;
////    SubdivFacet					subdF, *subF;
//    PointPrec                   pi, pj, piavrg, pjavrg, x[5], y[5], weights[4], a[5], b[5];
//    complex<PointPrec>          w, z;

//    //test that we have one EV only
//    if (my_EVlist.size() != 1)
//    {
//        // more than one EV -> cannot handle it now
//        cout << "More than one EV -> cannot use Conformal subdivision!" << endl;
//        smesh->my_numV = 0;
//    }
//    else
//    {
//        EV = my_EVlist[0];
//        valEV = EV->my_valency;

//        //assign posi and posj, coords for conf. stuff, to every corner
//        // ASSUMES QUADS ONLY!!!
//        cencrn = findCorner(EV, EV->my_facets[0]);
//        for (i = 0 ; i < valEV ; i++)
//        {
//            posj = 0;
//            crnj = cencrn;
//            while (crnj != NULL)
//            {
//                posi = 0;
//                crn = crnj;
//                while (crn->my_nextFacet != NULL)
//                {
//                    inCrn = getSameCorner(crn);
//                    inCrn->my_posi = posi;
//                    inCrn->my_posj = posj;
//                    inCrn->my_nextCorner->my_posi = posi + 1;
//                    inCrn->my_nextCorner->my_posj = posj;
//                    inCrn->my_nextCorner->my_nextCorner->my_posi = posi + 1;
//                    inCrn->my_nextCorner->my_nextCorner->my_posj = posj + 1;
//                    inCrn->my_nextCorner->my_nextCorner->my_nextCorner->my_posi = posi;
//                    inCrn->my_nextCorner->my_nextCorner->my_nextCorner->my_posj = posj + 1;

//                    crn = getNextCorner(crn);
//                    posi++;
//                }
//                crnj = getSameCorner(crnj->my_nextCorner);
//                posj++;
//            }
//            cencrn = getSameCorner(cencrn);
//        }

//        //assign coords at each corner for edge-points ('sqrt(2)/2 grid')
//        for (i = 0 ; i < my_numF ; i++)
//        {
//            facet = &my_facets[i];
//            for (j = 0 ; j < facet->my_valency ; j++)
//            {
//                crn = &facet->my_corners[j];
//                crn->my_alreadyIn = false; // resetting alreadyIn

//                xx = crn->my_posi;
//                yy = crn->my_posj;
//    //            xn = crn->my_nextCorner->my_posi;
//    //            yn = crn->my_nextCorner->my_posj;

//                if (xx > yy)
//                {
//                    aa = xx - yy;
//                    bb = xx + yy;
//                }
//                else if (xx < yy)
//                {
//                    aa = xx + yy;
//                    bb = -xx + yy;
//                }
//                else // vertex on 'diagonal' - needs separate treatment later!
//                {
//                    aa = xx + yy;
//                    bb = xx + yy;
//                }
//                crn->my_posa = aa;
//                crn->my_posb = bb;
//            }
//        }

//        // compute new face-points
//        for (i = 0 ; i < my_numF ; i++)
//        {
//            facet = &my_facets[i];
//            Fpoi = Point_3D();
//            piavrg = 0;
//            pjavrg = 0;
//            for (j = 0 ; j < facet->my_valency ; j++)
//            {
//                pi = facet->my_corners[j].my_posi;
//                pj = facet->my_corners[j].my_posj;
//                w = complex<PointPrec>(1.0*pi, 1.0*pj);
//                z = pow(w, (PointPrec)4.0 / valEV);
//                x[j] = real(z);
//                y[j] = imag(z);

//                piavrg = piavrg + pi;
//                pjavrg = pjavrg + pj;
//            }
//            w = complex<PointPrec>(piavrg * 0.25, pjavrg * 0.25);
//            z = pow(w, (PointPrec)4.0 / valEV);
//            x[4] = real(z);
//            y[4] = imag(z);

//            if (w.real()*w.real() + w.imag()*w.imag() > 64)
//            {
////                computeWeights(x, y, weights);
//                weights[0] = 0.25;
//                weights[1] = 0.25;
//                weights[2] = 0.25;
//                weights[3] = 0.25;
//            }
//            else
//            {
//                computeWeights(x, y, weights);
//            }

//            for (j = 0 ; j < facet->my_valency ; j++)
//            {
//                tmp = facet->my_corners[j].my_vertex->my_point * weights[j];
//                Fpoi = Fpoi + tmp;
//            }
//            facet->my_Fpoi = Fpoi;
//        }

//        // compute new edge-points - each only once!
//        for (i = 0 ; i < my_numF ; i++)
//        {
//            facet = &my_facets[i];
//            for (j = 0 ; j < facet->my_valency ; j++)
//            {
//                crn = &facet->my_corners[j];

//                if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
//                {
//                    Epoi = Point_3D();
//                    if (crn->my_nextFacet == NULL)
//                    {
//                        Epoi += crn->my_vertex->my_point;
//                        Epoi += crn->my_nextCorner->my_vertex->my_point;
//                        Epoi *= 0.5;
//                    }
//                    else
//                    {
//                        a[0] = crn->my_posa;
//                        b[0] = crn->my_posb;
//                        a[2] = crn->my_nextCorner->my_posa;
//                        b[2] = crn->my_nextCorner->my_posb;

//                        //separate treatment for 'diagonal' vertices
//                        if (crn->my_posi == crn->my_posj)
//                        {
//                            if (a[2] == 1)
//                            {
//                                a[0] = 0;
//                            }
//                            else
//                            {
//                                b[0] = 0;
//                            }
//                        }
//                        if (crn->my_nextCorner->my_posi == crn->my_nextCorner->my_posj)
//                        {
//                            if (a[0] == 1)
//                            {
//                                a[2] = 0;
//                            }
//                            else
//                            {
//                                b[2] = 0;
//                            }
//                        }

//                        //fixing orientation
//                        if ((a[0] - a[2]) * (b[0] - b[2]) < 0)
//                        {
//                            a[1] = a[0];
//                            b[1] = b[2];
//                            a[3] = a[2];
//                            b[3] = b[0];
//                        }
//                        else
//                        {
//                            a[1] = a[2];
//                            b[1] = b[0];
//                            a[3] = a[0];
//                            b[3] = b[2];
//                        }
//                        a[4] = (a[0] + a[2]) * 0.5;
//                        b[4] = (b[0] + b[2]) * 0.5;

//                        for (k = 0 ; k < 5 ; k++)
//                        {
//                            w = complex<PointPrec>(1.0 * a[k], 1.0 * b[k]);
//                            z = pow(w, (PointPrec)4.0 / valEV);
//                            x[k] = real(z);
//                            y[k] = imag(z);
//                        }

//                        if (w.real()*w.real() + w.imag()*w.imag() > 64)
//                        {
////                            computeWeights(x, y, weights);
//                            weights[0] = 0.25;
//                            weights[1] = 0.25;
//                            weights[2] = 0.25;
//                            weights[3] = 0.25;
//                        }
//                        else
//                        {
//                            computeWeights(x, y, weights);
//                        }

//                        Epoi = crn->my_vertex->my_point * weights[0];
//                        tmp = crn->my_nextFacet->my_Fpoi * weights[1];
//                        Epoi = Epoi + tmp;
//                        tmp = crn->my_nextCorner->my_vertex->my_point * weights[2];
//                        Epoi = Epoi + tmp;
//                        tmp = facet->my_Fpoi * weights[3];
//                        Epoi = Epoi + tmp;
//                    }
//                    crn->my_Epoi = Epoi;
//                    if (crn->my_pairedCorner != NULL)
//                    {
//                        crn->my_pairedCorner->my_Epoi = Epoi; // used later for Epoi averaging (really?)
//                    }
//                }
//            }
//        }

////                if (crn->my_vertex->isOnBoundary && crn->my_nextCorner->my_vertex->isOnBoundary)
////                {
////                    Epoi = crn->my_vertex->my_point + crn->my_nextCorner->my_vertex->my_point;
////                    Epoi = Epoi * 0.5;
////                }
////                else
////                {
////                    a[0] = crn->my_posa;
////                    b[0] = crn->my_posb;
////                    a[2] = crn->my_nextCorner->my_posa;
////                    b[2] = crn->my_nextCorner->my_posb;

////                    //separate treatment for 'diagonal' vertices
////                    if (crn->my_posi == crn->my_posj)
////                    {
////                        if (a[2] == 1)
////                        {
////                            a[0] = 0;
////                        }
////                        else
////                        {
////                            b[0] = 0;
////                        }
////                    }
////                    if (crn->my_nextCorner->my_posi == crn->my_nextCorner->my_posj)
////                    {
////                        if (a[0] == 1)
////                        {
////                            a[2] = 0;
////                        }
////                        else
////                        {
////                            b[2] = 0;
////                        }
////                    }

////                    //fixing orientation
////                    if ((a[0] - a[2]) * (b[0] - b[2]) < 0)
////                    {
////                        a[1] = a[0];
////                        b[1] = b[2];
////                        a[3] = a[2];
////                        b[3] = b[0];
////                    }
////                    else
////                    {
////                        a[1] = a[2];
////                        b[1] = b[0];
////                        a[3] = a[0];
////                        b[3] = b[2];
////                    }
////                    a[4] = (a[0] + a[2]) * 0.5;
////                    b[4] = (b[0] + b[2]) * 0.5;

////                    for (k = 0 ; k < 5 ; k++)
////                    {
////                        w = complex<PointPrec>(1.0 * a[k], 1.0 * b[k]);
////                        z = pow(w, (PointPrec)4.0 / valEV);
////                        x[k] = real(z);
////                        y[k] = imag(z);
////                    }
////                    computeWeights(x, y, weights);

////                    Epoi = crn->my_vertex->my_point * weights[0];
////                    tmp = crn->my_nextFacet->my_Fpoi * weights[1];
////                    Epoi = Epoi + tmp;
////                    tmp = crn->my_nextCorner->my_vertex->my_point * weights[2];
////                    Epoi = Epoi + tmp;
////                    tmp = facet->my_Fpoi * weights[3];
////                    Epoi = Epoi + tmp;
////                }
////                crn->my_Epoi = Epoi;
////            }
////        }

//        // compute new vertex-points
//        for (i = 0 ; i < my_numV ; i++)
//        {
//            vertex = &my_vertices[i];
//            Vpoi = Point_3D();

//            if (vertex->isOnBoundary)
//            {
//                if (vertex->my_valency == 1)
//                {
//                    Vpoi = vertex->my_point;
//                }
//                else if (vertex->my_valency == 2) // use cubic B-spline stencil
//                {
//                    Vpoi = 6 * vertex->my_point;
//                    crn = findCorner(vertex, vertex->my_facets[0]);
//                    next0 = crn->my_nextCorner->my_vertex;
//                    for (j = 0 ; j < vertex->my_facets[0]->my_valency - 1 ; j++)
//                    {
//                        crn = crn->my_nextCorner;
//                    }
//                    prev0 = crn->my_vertex;

//                    crn = findCorner(vertex, vertex->my_facets[1]);
//                    next1 = crn->my_nextCorner->my_vertex;
//                    for (j = 0 ; j < vertex->my_facets[1]->my_valency - 1 ; j++)
//                    {
//                        crn = crn->my_nextCorner;
//                    }
//                    prev1 = crn->my_vertex;

//                    if (next0->my_index == prev1->my_index)
//                    {
//                        Vpoi += prev0->my_point + next1->my_point;
//                    }
//                    else
//                    {
//                        Vpoi += prev1->my_point + next0->my_point;
//                    }
//                    Vpoi *= 0.125;
//                }
//                else
//                {
////                    assert(false); // something is wrong at the boundary!

//                    // instead of crashing, interpolate the point
//                    Vpoi = vertex->my_point;
//                }
//            }
//            else
//            {
//                Favrg = Point_3D();
//                for (j = 0 ; j < vertex->my_valency ; j++)
//                {
//                    Favrg = Favrg + vertex->my_facets[j]->my_Fpoi;
//                }
//                Favrg = Favrg * (1.0 / vertex->my_valency);

//                Eavrg = Point_3D();
//                cencrn = findCorner(vertex, vertex->my_facets[0]);
//                for (j = 0 ; j < vertex->my_valency ; j++)
//                {
//                    Eavrg += cencrn->my_Epoi; // this works only if pairedCorners are assigned Epois as well (see above).
//    //                Eavrg += cencrn->my_Epoi;
//    //                Eavrg += cencrn->my_pairedCorner->my_Epoi;
//                    cencrn = getSameCorner(cencrn);
//                }
//                Eavrg *= (1.0 / vertex->my_valency);

//                val = vertex->my_valency;

//                if (val > 12)
//                {
//                    val = 12;
//                }
//                if (val < 3)
//                {
//                    val = 3;
//                }
//                Vpoi = cfA[val - 3] * vertex->my_point;
//                Vpoi += cfB[val - 3] * Eavrg;
//                Vpoi += cfC[val - 3] * Favrg;

//    //            sum = ccA[val - 3]+ccB[val - 3]+ccC[val - 3];
//            }
//            vertex->my_Vpoi = Vpoi;
//        }

////            else
////            {
////                Favrg = Point_3D();
////                for (j = 0 ; j < vertex->my_valency ; j++)
////                {
////                    Favrg = Favrg + vertex->my_facets[j]->my_Fpoi;
////                }
////                Favrg = Favrg * (1.0 / vertex->my_valency);

////                Eavrg = Point_3D();
////                cencrn = findCorner(vertex, vertex->my_facets[0]);
////                for (j = 0 ; j < vertex->my_valency ; j++)
////                {
////                    Eavrg = Eavrg + cencrn->my_nextCorner->my_vertex->my_point;
////                    cencrn = getSameCorner(cencrn);
////                }
////                Eavrg = Eavrg * (1.0 / vertex->my_valency);
////                Eavrg = Eavrg + vertex->my_point;
////                Eavrg = Eavrg * 0.5;

////                if (vertex->my_valency < 3)
////                {
////                    Vpoi = Point_3D(0,0,0);
////                }
////                else
////                {
////                    Vpoi = vertex->my_point * (vertex->my_valency - 3);
////                }
////                Vpoi = Vpoi + Eavrg + Eavrg;
////                Vpoi = Vpoi + Favrg;
////                if (vertex->my_valency < 3)
////                {
////                    Vpoi = Vpoi * (1.0 / 3.0);
////                }
////                else
////                {
////                    Vpoi = Vpoi * (1.0 / vertex->my_valency);
////                }
////            }
////            vertex->my_Vpoi = Vpoi;
////        }

//        // copy over new positions of V-vertices
//        smesh->my_vertices.clear();
//        for (i = 0 ; i < my_numV ; i++)
//        {
//            vert.my_point = my_vertices[i].my_Vpoi;
//            vert.my_index = i;
//            vert.my_faceIndices.clear();
//            vert.isFeature = true;
//            smesh->my_vertices.push_back(vert);
//        }
//        // add F-vertices
//        for (i = 0 ; i < my_numF ; i++)
//        {
//            vert.my_point = my_facets[i].my_Fpoi;
//            vert.my_index = my_numV + i;
//            vert.isFeature = false;
//            smesh->my_vertices.push_back(vert);
//        }

//        //index E-vertices and add them to vertex list
//        cnt = smesh->my_vertices.size();
//        for (i = 0 ; i < my_numF ; i++)
//        {
//            facet = &my_facets[i];
//            for (j = 0 ; j < facet->my_valency ; j++)
//            {
//                crn = &(facet->my_corners[j]);

//                if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
//                {
//                    vert.my_point = crn->my_Epoi;
//                    vert.my_index = cnt;
//                    crn->my_newEindex = cnt;
//                    if (crn->my_vertex->isFeature && crn->my_nextCorner->my_vertex->isFeature)
//                    {
//                        vert.isFeature = true;
//                    }
//                    else
//                    {
//                        vert.isFeature = false;
//                    }
//                    smesh->my_vertices.push_back(vert);
//                    cnt++;
//                }
//            }
//        }

//        //create facets
//        smesh->my_facets.clear();
//        smesh->quadMesh = true;
//        cnt = 0;
//        for (i = 0 ; i < my_numF ; i++)
//        {
//            facet = &my_facets[i];
//            for (j = 0 ; j < facet->my_valency ; j++)
//            {
//                crn = &(facet->my_corners[j]);

//                face.my_index = cnt;
//                face.my_vertIndices.clear();
//                face.my_valency = 4;

//                //add four vertices
//                face.my_vertIndices.push_back(crn->my_vertex->my_index);
//                smesh->my_vertices[crn->my_vertex->my_index].my_faceIndices.push_back(cnt);

//                if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
//                {
//                    face.my_vertIndices.push_back(crn->my_newEindex);
//                    smesh->my_vertices[crn->my_newEindex].my_faceIndices.push_back(cnt);
//                }
//                else
//                {
//                    face.my_vertIndices.push_back(crn->my_pairedCorner->my_newEindex);
//                    smesh->my_vertices[crn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
//                }

//                face.my_vertIndices.push_back(facet->my_index + my_numV);
//                smesh->my_vertices[facet->my_index + my_numV].my_faceIndices.push_back(cnt);

//                prevcrn = crn;
//                for (k = 0 ; k < facet->my_valency - 1 ; k++)
//                {
//                    prevcrn = prevcrn->my_nextCorner;
//                }

//                if (prevcrn->my_nextFacet == NULL || facet->my_index < prevcrn->my_nextFacet->my_index) // we are at the boundary or ...
//                {
//                    face.my_vertIndices.push_back(prevcrn->my_newEindex);
//                    smesh->my_vertices[prevcrn->my_newEindex].my_faceIndices.push_back(cnt);
//                }
//                else
//                {
//                    face.my_vertIndices.push_back(prevcrn->my_pairedCorner->my_newEindex);
//                    smesh->my_vertices[prevcrn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
//                }

//                smesh->my_facets.push_back(face);
//                cnt++;
//            }
//        }

//        smesh->my_numV = smesh->my_vertices.size();
//        smesh->my_numF = smesh->my_facets.size();
//        smesh->my_level = my_level + 1;

//        smesh->my_minz = my_minz;
//        smesh->my_maxz = my_maxz;

//        cout << "Conformal end" << endl;

//        smesh->build();
//    }
}

//Catmull-Clark + Laplacian at EVs
void Mesh::LaplacianSubdiv(Mesh *smesh, int lapSmvalue)
{
    unsigned int 				i, j, k, cnt;
    MeshFacet 					*facet, face;
    MeshVertex					*vertex, vert, *prev0, *prev1, *next0, *next1;
    MeshCorner					*cencrn, *prevcrn, *crn;
    Point_3D					Fpoi, Epoi, Vpoi, Favrg, Eavrg;
    double                      lam;

    cout << "Laplacian Catmull-Clark start" << endl;

    lam = (double)lapSmvalue / 10.0;

    // compute new face-points
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        Fpoi = Point_3D();
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            Fpoi += facet->my_corners[j].my_vertex->my_point;
        }
        Fpoi *= (1.0 / facet->my_valency);
        facet->my_Fpoi = Fpoi;
    }

    // compute new edge-points - each only once!
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);

            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                Epoi = Point_3D();
                Epoi += crn->my_vertex->my_point;
                Epoi += crn->my_nextCorner->my_vertex->my_point;
                if (crn->my_nextFacet == NULL)
                {
                    Epoi *= 0.5;
                }
                else
                {
                    Epoi += facet->my_Fpoi;
                    Epoi += facet->my_corners[j].my_nextFacet->my_Fpoi;
                    Epoi *= 0.25;
                }
                facet->my_corners[j].my_Epoi = Epoi;
            }
        }
    }

    // compute new vertex-points
    for (i = 0 ; i < my_numV ; i++)
    {
        vertex = &my_vertices[i];
        Vpoi = Point_3D();

        if (vertex->isOnBoundary)
        {
            if (vertex->my_valency == 1)
            {
                Vpoi = vertex->my_point;
            }
            else if (vertex->my_valency == 2)
            {
                crn = findCorner(vertex, vertex->my_facets[0]);
                next0 = crn->my_nextCorner->my_vertex;
                for (j = 0 ; j < vertex->my_facets[0]->my_valency - 1 ; j++)
                {
                    crn = crn->my_nextCorner;
                }
                prev0 = crn->my_vertex;

                crn = findCorner(vertex, vertex->my_facets[1]);
                next1 = crn->my_nextCorner->my_vertex;
                for (j = 0 ; j < vertex->my_facets[1]->my_valency - 1 ; j++)
                {
                    crn = crn->my_nextCorner;
                }
                prev1 = crn->my_vertex;

                if (next0->my_index == prev1->my_index)
                {
                    Vpoi += prev0->my_point + next1->my_point;
                }
                else
                {
                    Vpoi += prev1->my_point + next0->my_point;
                }
                Vpoi *= lam / 2.0; // Laplacian weight
                Vpoi += (1.0 - lam) * vertex->my_point;
            }
            else
            {
                cout << "Something is wrong at the boundary!" << endl;
//                assert(false); // something is wrong at the boundary!

                // instead of crashing, interpolate the point
                Vpoi = vertex->my_point;
            }
        }
        else
        {
            cencrn = findCorner(vertex, vertex->my_facets[0]);
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Vpoi += cencrn->my_nextCorner->my_vertex->my_point;
                cencrn = getSameCorner(cencrn);
            }
            Vpoi *= (lam / vertex->my_valency); // lam in Laplacian smoothing
            Vpoi += (1.0 - lam) * vertex->my_point;
        }
        vertex->my_Vpoi = Vpoi;
    }

    // copy over new positions of V-vertices
    smesh->my_vertices.clear();
    for (i = 0 ; i < my_numV ; i++)
    {
        vert.my_point = my_vertices[i].my_Vpoi;
        vert.my_index = i;
        vert.my_faceIndices.clear();
        vert.isFeature = true;
        smesh->my_vertices.push_back(vert);
    }
    // add F-vertices
    for (i = 0 ; i < my_numF ; i++)
    {
        vert.my_point = my_facets[i].my_Fpoi;
        vert.my_index = my_numV + i;
        vert.isFeature = false;
        smesh->my_vertices.push_back(vert);
    }

    //index E-vertices and add them to vertex list
    cnt = smesh->my_vertices.size();
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);

            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                vert.my_point = crn->my_Epoi;
                vert.my_index = cnt;
                crn->my_newEindex = cnt;
                if (crn->my_vertex->isFeature && crn->my_nextCorner->my_vertex->isFeature)
                {
                    vert.isFeature = true;
                }
                else
                {
                    vert.isFeature = false;
                }
                smesh->my_vertices.push_back(vert);
                cnt++;
            }
        }
    }

    //create facets
    smesh->my_facets.clear();
    smesh->quadMesh = true;
    cnt = 0;
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);

            face.my_index = cnt;
            face.my_vertIndices.clear();
            face.my_valency = 4;

            //add four vertices
            face.my_vertIndices.push_back(crn->my_vertex->my_index);
            smesh->my_vertices[crn->my_vertex->my_index].my_faceIndices.push_back(cnt);

            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                face.my_vertIndices.push_back(crn->my_newEindex);
                smesh->my_vertices[crn->my_newEindex].my_faceIndices.push_back(cnt);
            }
            else
            {
                face.my_vertIndices.push_back(crn->my_pairedCorner->my_newEindex);
                smesh->my_vertices[crn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
            }

            face.my_vertIndices.push_back(facet->my_index + my_numV);
            smesh->my_vertices[facet->my_index + my_numV].my_faceIndices.push_back(cnt);

            prevcrn = crn;
            for (k = 0 ; k < facet->my_valency - 1 ; k++)
            {
                prevcrn = prevcrn->my_nextCorner;
            }

            if (prevcrn->my_nextFacet == NULL || facet->my_index < prevcrn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                face.my_vertIndices.push_back(prevcrn->my_newEindex);
                smesh->my_vertices[prevcrn->my_newEindex].my_faceIndices.push_back(cnt);
            }
            else
            {
                face.my_vertIndices.push_back(prevcrn->my_pairedCorner->my_newEindex);
                smesh->my_vertices[prevcrn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
            }

            smesh->my_facets.push_back(face);
            cnt++;
        }
    }

    smesh->my_numV = smesh->my_vertices.size();
    smesh->my_numF = smesh->my_facets.size();
    smesh->my_level = my_level + 1;

    smesh->my_minz = my_minz;
    smesh->my_maxz = my_maxz;

    cout << "Laplacian Catmull-Clark end" << endl;

    smesh->build();
}

//Wavelet subdivision
void Mesh::WaveletSubdiv(Mesh *smesh)
{
    unsigned int 				i, j, k, cnt, num, numCorners, numCreases, ind0,
                                edgeS1, edgeS2, W13s1, W13s2, W31s1, W31s2, undef; // sharpnesses
    MeshFacet 					*facet, face;
    MeshVertex					*vertex, vert, *prev0, *prev1, *next0, *next1, *v;
    MeshCorner					*cencrn, *prevcrn, *crn;
    Point_3D					Fpoi, Epoi, Vpoi, Favrg, Eavrg, W13, W31, W33;
//    PointPrec                   x, y, z;
    vector<MeshCorner*>         vecCrn, vecAllCrn, vecCrsCrn;

    cout << "Wavelet Subdivision start" << endl;

    times("");

    // compute new face-points
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        Fpoi = Point_3D();

        for (j = 0 ; j < facet->my_valency ; j++)
        {
            Fpoi += facet->my_corners[j].my_vertex->my_point;
        }
        Fpoi *= (1.0 / facet->my_valency);
        facet->my_Fpoi = Fpoi;
    }
    times("Face average loop");

    //collect corners so that new E-vertices can be added in parallel
    vecCrn.clear();
    vecAllCrn.clear();
    numCorners = 0;
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);
            vecAllCrn.push_back(crn);
            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                vecCrn.push_back(crn);
                crn->my_newID = my_numV + my_numF + numCorners;
                numCorners++;
            }
        }
    }

    cnt = my_numV + my_numF;
    num = vecCrn.size();

    // compute new edge-points - each only once!
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);

            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                Epoi = Point_3D();
                Epoi += crn->my_vertex->my_point;
                Epoi += crn->my_nextCorner->my_vertex->my_point;

                W13 = Point_3D();
                W31 = Point_3D();
                W33 = Point_3D();

                edgeS1 = 0;
                edgeS2 = 0;

                if (crn->my_nextFacet == NULL)
                {
                    Epoi *= 0.5;
                }
                else
                {
                    if (crn->my_sharpness != 0 || crn->my_pairedCorner->my_sharpness != 0)
                    {
                        Epoi *= 0.5;
                    }
                    else
                    {
                        Epoi += facet->my_Fpoi;
                        Epoi += facet->my_corners[j].my_nextFacet->my_Fpoi;
                        Epoi *= 0.25;
                    }

                    //add wavelet contribution
                    if (crn->my_vertex->my_W13vid0 == crn->my_nextCorner->my_vertex->my_index ||
                        crn->my_vertex->my_W13vid1 == crn->my_nextCorner->my_vertex->my_index)
                    {
                        edgeS1 = crn->my_vertex->my_W13sharpness;
                        if (edgeS1 > 0)
                        {
                            Epoi += 0.375 * crn->my_vertex->my_W13;
                            W13 += 0.25 * crn->my_vertex->my_W13;
                        }
                        //fix new ID wavelet pointer for contributing vertex
                        if (crn->my_vertex->my_W13vid0 == crn->my_nextCorner->my_vertex->my_index)
                        {
                            crn->my_vertex->my_newW13vid0 = crn->my_newID;
                        }
                        else
                        {
                            crn->my_vertex->my_newW13vid1 = crn->my_newID;
                        }
                    }
                    else if (crn->my_vertex->my_W31vid0 == crn->my_nextCorner->my_vertex->my_index ||
                             crn->my_vertex->my_W31vid1 == crn->my_nextCorner->my_vertex->my_index)
                    {
                        edgeS1 = crn->my_vertex->my_W31sharpness;
                        if (edgeS1 > 0)
                        {
                            Epoi += 0.375 * crn->my_vertex->my_W31;
                            W13 += 0.25 * crn->my_vertex->my_W31;
                        }
                        //fix new ID wavelet pointer for contributing vertex
                        if (crn->my_vertex->my_W31vid0 == crn->my_nextCorner->my_vertex->my_index)
                        {
                            crn->my_vertex->my_newW31vid0 = crn->my_newID;
                        }
                        else
                        {
                            crn->my_vertex->my_newW31vid1 = crn->my_newID;
                        }
                    }

                    if (crn->my_vertex->my_index == crn->my_nextCorner->my_vertex->my_W13vid0 ||
                        crn->my_vertex->my_index == crn->my_nextCorner->my_vertex->my_W13vid1)
                    {
                        edgeS2 = crn->my_nextCorner->my_vertex->my_W13sharpness;
                        if (edgeS2 > 0)
                        {
                            Epoi += 0.375 * crn->my_nextCorner->my_vertex->my_W13;
                            W13 += 0.25 * crn->my_nextCorner->my_vertex->my_W13;
                        }
                        //fix new ID wavelet pointer for contributing vertex
                        if (crn->my_vertex->my_index == crn->my_nextCorner->my_vertex->my_W13vid0)
                        {
                            crn->my_nextCorner->my_vertex->my_newW13vid0 = crn->my_newID;
                        }
                        else
                        {
                            crn->my_nextCorner->my_vertex->my_newW13vid1 = crn->my_newID;
                        }
                    }
                    else if (crn->my_vertex->my_index == crn->my_nextCorner->my_vertex->my_W31vid0 ||
                             crn->my_vertex->my_index == crn->my_nextCorner->my_vertex->my_W31vid1)
                    {
                        edgeS2 = crn->my_nextCorner->my_vertex->my_W31sharpness;
                        if (edgeS2 > 0)
                        {
                            Epoi += 0.375 * crn->my_nextCorner->my_vertex->my_W31;
                            W13 += 0.25 * crn->my_nextCorner->my_vertex->my_W31;
                        }
                        //fix new ID wavelet pointer for contributing vertex
                        if (crn->my_vertex->my_index == crn->my_nextCorner->my_vertex->my_W31vid0)
                        {
                            crn->my_nextCorner->my_vertex->my_newW31vid0 = crn->my_newID;
                        }
                        else
                        {
                            crn->my_nextCorner->my_vertex->my_newW31vid1 = crn->my_newID;
                        }
                    }
                }
                crn->my_Epoi = Epoi;
                crn->my_newW13 = W13;
                crn->my_newW31 = W31;
                crn->my_newW33 = W33;
                crn->my_newWvid0 = crn->my_vIndex;
                crn->my_newWvid1 = crn->my_nIndex;

                crn->my_W13sharpness = decideSharpness(edgeS1, edgeS2);

//                if (crn->my_W13sharpness > 10)
//                {
//                    cout << "wrong" << endl;
//                }

                crn->my_W31sharpness = 0;
                crn->my_W33sharpness = 0;
            }
        }
    }
    times("Edge average loop");

    // compute new vertex-points
    for (i = 0 ; i < my_numV ; i++)
    {
        vertex = &my_vertices[i];

        //find number of incoming creases
        numCreases = 0;
        vecCrsCrn.clear();
        for (j = 0 ; j < vertex->my_valency ; j ++)
        {
            crn = findCorner(vertex, vertex->my_facets[j]);
            if (crn->my_sharpness > 0)
            {
                numCreases++;
                vecCrsCrn.push_back(crn);
            }
        }

        Vpoi = Point_3D();
        W13 = Point_3D();
        W31 = Point_3D();
        W33 = Point_3D();

        if (vertex->isOnBoundary || numCreases > 1)
        {
            if (vertex->my_valency == 1 || numCreases > 2)
            {
                Vpoi = vertex->my_point;
            }
            else if (vertex->my_valency == 2 || numCreases == 2)
            {
                Vpoi = 6 * vertex->my_point;
                if (numCreases == 2)
                {
                    Vpoi += vecCrsCrn[0]->my_nextCorner->my_vertex->my_point;
                    Vpoi += vecCrsCrn[1]->my_nextCorner->my_vertex->my_point;
                }
                else
                {
                    crn = findCorner(vertex, vertex->my_facets[0]);
                    next0 = crn->my_nextCorner->my_vertex;
                    for (j = 0 ; j < vertex->my_facets[0]->my_valency - 1 ; j++)
                    {
                        crn = crn->my_nextCorner;
                    }
                    prev0 = crn->my_vertex;

                    crn = findCorner(vertex, vertex->my_facets[1]);
                    next1 = crn->my_nextCorner->my_vertex;
                    for (j = 0 ; j < vertex->my_facets[1]->my_valency - 1 ; j++)
                    {
                        crn = crn->my_nextCorner;
                    }
                    prev1 = crn->my_vertex;

                    if (next0->my_index == prev1->my_index)
                    {
                        Vpoi += prev0->my_point + next1->my_point;
                    }
                    else
                    {
                        Vpoi += prev1->my_point + next0->my_point;
                    }
                }
                Vpoi *= 0.125;
            }
            else
            {
//                cout << "Something is wrong at the boundary!" << endl;
//                assert(false); // something is wrong at the boundary!

                // instead of crashing, interpolate the point
                Vpoi = vertex->my_point;
            }
        }
        else
        {
//            if (isLshape(vertex))
//            {
//                Vpoi = 6 * vertex->my_point;
//                Vpoi += my_vertices[vertex->my_W13vid0].my_point;
//                Vpoi += my_vertices[vertex->my_W13vid1].my_point;
//                Vpoi *= 0.125;
//            }
//            else
//            {
            Favrg = Point_3D();
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Favrg += vertex->my_facets[j]->my_Fpoi;
            }
            Favrg *= (1.0 / vertex->my_valency);

            Eavrg = Point_3D();

            cencrn = findCorner(vertex, vertex->my_facets[0]);
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Eavrg += cencrn->my_nextCorner->my_vertex->my_point;
                cencrn = getSameCorner(cencrn);
            }
            Eavrg *= (1.0 / vertex->my_valency);
            Eavrg += vertex->my_point;
            Eavrg *= 0.5;

            if (vertex->my_valency < 3)
            {
                Vpoi = Point_3D(0,0,0);
            }
            else
            {
                Vpoi = vertex->my_point * (vertex->my_valency - 3);
            }
            Vpoi = Vpoi + Eavrg + Eavrg;
            Vpoi += Favrg;
            if (vertex->my_valency < 3)
            {
                Vpoi *= (1.0 / 3.0);
            }
            else
            {
                Vpoi *= (1.0 / vertex->my_valency);
            }
//            }
        }

        //add wavelet contribution
        if (!vertex->isOnBoundary)
        {
            ind0 = vertex->my_index;

            if (vertex->my_W13sharpness > 0)
            {
                Vpoi += 0.5625 * vertex->my_W13;
                W13 += 0.375 * vertex->my_W13;
            }
            if (vertex->my_W31sharpness > 0)
            {
                Vpoi += 0.5625 * vertex->my_W31;
                W31 += 0.375 * vertex->my_W31;
            }
            if (vertex->my_W33sharpness > 0)
            {
                Vpoi += 0.5625 * vertex->my_W33;
                W13 += 0.375 * vertex->my_W33;
                W31 += 0.375 * vertex->my_W33;
                W33 += 0.25 * vertex->my_W33;
            }

            undef = 111;
            W13s1 = undef;
            W13s2 = undef;
            W31s1 = undef;
            W31s2 = undef;

//                vertex->my_newW13vid0 = -1;
//                vertex->my_newW13vid1 = -1;
//                vertex->my_newW31vid0 = -1;
//                vertex->my_newW31vid0 = -1;

            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                crn = findCorner(vertex, vertex->my_facets[j]);
//                    fID = crn->my_facet->my_index;
//                    nfID = crn->my_nextFacet->my_index;
                v = crn->my_nextCorner->my_vertex;

                if (v->my_W13vid0 == ind0 || v->my_W13vid1 == ind0)
                {
                    if (v->my_W13sharpness > 0)
                    {
                        Vpoi += 0.09375 * v->my_W13;
                    }
                    if (vertex->my_W13vid0 == v->my_index || vertex->my_W13vid1 == v->my_index)
                    {
                        if (v->my_W13sharpness > 0)
                        {
                            W13 += 0.0625 * v->my_W13;
                        }
                        //new ID index
//                            vertex->my_newW13vid0 = getMainCrn(crn)->my_newID;
//                            vertex->my_newW13vid1 = getMainCrn(getSameCorner(getSameCorner(crn)))->my_newID;

                        if (W13s1 == undef)
                        {
                            W13s1 = v->my_W13sharpness;
                        }
                        else
                        {
                            W13s2 = v->my_W13sharpness;
                        }
                    }
                    else if (vertex->my_W31vid0 == v->my_index || vertex->my_W31vid1 == v->my_index)
                    {
                        if (v->my_W13sharpness > 0)
                        {
                            W31 += 0.0625 * v->my_W13;
                        }
                        //new ID index
//                            vertex->my_newW31vid0 = getMainCrn(crn)->my_newID;
//                            vertex->my_newW31vid1 = getMainCrn(getSameCorner(getSameCorner(crn)))->my_newID;

                        if (W31s1 == undef)
                        {
                            W31s1 = v->my_W13sharpness;
                        }
                        else
                        {
                            W31s2 = v->my_W13sharpness;
                        }
                    }
                }
                if (v->my_W31vid0 == ind0 || v->my_W31vid1 == ind0)
                {
                    if (v->my_W31sharpness > 0)
                    {
                        Vpoi += 0.09375 * v->my_W31;
                    }
                    if (vertex->my_W13vid0 == v->my_index || vertex->my_W13vid1 == v->my_index)
                    {
                        if (v->my_W31sharpness > 0)
                        {
                            W13 += 0.0625 * v->my_W31;
                        }
                        //new ID index
//                            vertex->my_newW13vid0 = getMainCrn(crn)->my_newID;
//                            vertex->my_newW13vid1 = getMainCrn(getSameCorner(getSameCorner(crn)))->my_newID;

                        if (W13s1 == undef)
                        {
                            W13s1 = v->my_W31sharpness;
                        }
                        else
                        {
                            W13s2 = v->my_W31sharpness;
                        }
                    }
                    else if (vertex->my_W31vid0 == v->my_index || vertex->my_W31vid1 == v->my_index)
                    {
                        if (v->my_W31sharpness > 0)
                        {
                            W31 += 0.0625 * v->my_W31;
                        }
                        //new ID index
//                            vertex->my_newW31vid0 = getMainCrn(crn)->my_newID;
//                            vertex->my_newW31vid1 = getMainCrn(getSameCorner(getSameCorner(crn)))->my_newID;

                        if (W31s1 == undef)
                        {
                            W31s1 = v->my_W31sharpness;
                        }
                        else
                        {
                            W31s2 = v->my_W31sharpness;
                        }
                    }
                }
            }
        }
        vertex->my_Vpoi = Vpoi;
        vertex->my_newW13 = W13;
        vertex->my_newW31 = W31;
        vertex->my_newW33 = W33;

        if (W13s1 == undef) // probably due to an EV or a boundary vertex
        {
            W13s1 = 0;
        }
        if (W13s2 == undef) // probably due to an EV or a boundary vertex
        {
            W13s2 = 0;
        }
        if (W31s1 == undef) // probably due to an EV or a boundary vertex
        {
            W31s1 = 0;
        }
        if (W31s2 == undef) // probably due to an EV or a boundary vertex
        {
            W31s2 = 0;
        }

        vertex->my_newW13sharpness = decideSharpness(W13s1, W13s2, vertex->my_W13sharpness, vertex->my_W33sharpness);
        vertex->my_newW31sharpness = decideSharpness(W31s1, W31s2, vertex->my_W31sharpness, vertex->my_W33sharpness);
        vertex->my_newW33sharpness = vertex->my_W33sharpness;
    }
    times("Vertex average loop");

    // copy over new positions of V-vertices
    smesh->my_vertices.clear();
    smesh->my_vertices.resize(my_numV);

    for (i = 0 ; i < my_numV ; i++)
    {
        vertex = &my_vertices[i];

        vert.my_point = vertex->my_Vpoi;

        //wavelets
        vert.my_W13 = vertex->my_newW13;
        vert.my_W31 = vertex->my_newW31;
        vert.my_W33 = vertex->my_newW33;
        //and their indices
        vert.my_W13vid0 = vertex->my_newW13vid0;
        vert.my_W13vid1 = vertex->my_newW13vid1;
        vert.my_W31vid0 = vertex->my_newW31vid0;
        vert.my_W31vid1 = vertex->my_newW31vid1;

        if (vertex->my_newW13sharpness > 0)
        {
            vert.my_W13sharpness = vertex->my_newW13sharpness - 1;
        }
        else
        {
            vert.my_W13sharpness = 0;
            vert.my_W13 = Point_3D();
        }

        if (vertex->my_newW31sharpness > 0)
        {
            vert.my_W31sharpness = vertex->my_newW31sharpness - 1;
        }
        else
        {
            vert.my_W31sharpness = 0;
            vert.my_W31 = Point_3D();
        }

        if (vertex->my_newW33sharpness > 0)
        {
            vert.my_W33sharpness = vertex->my_newW33sharpness - 1;
        }
        else
        {
            vert.my_W33sharpness = 0;
            vert.my_W33 = Point_3D();
        }

        vert.my_index = i;
        vert.my_faceIndices.clear();
        vert.isFeature = true;
        smesh->my_vertices[i] = vert;
    }

    // add F-vertices
    smesh->my_vertices.resize(my_numV + my_numF);

    for (i = 0 ; i < my_numF ; i++)
    {
        vert.my_point = my_facets[i].my_Fpoi;
        vert.my_W13 = Point_3D();
        vert.my_W31 = Point_3D();
        vert.my_W33 = Point_3D();

        vert.my_W13vid0 = -1;
        vert.my_W13vid1 = -1;
        vert.my_W31vid0 = -1;
        vert.my_W31vid1 = -1;

        vert.my_W13sharpness = 0;
        vert.my_W31sharpness = 0;
        vert.my_W33sharpness = 0;

        vert.my_index = my_numV + i;
        vert.isFeature = false;
        smesh->my_vertices[i + my_numV] = vert;
    }
    times("Copy over V- and F-vertices");

    //index E-vertices and add them to vertex list
    smesh->my_vertices.resize(cnt + num);
    for (i = cnt ; i < cnt + num ; i++)
    {
        crn = vecCrn[i - cnt];
        vert.my_point = crn->my_Epoi;

        vert.my_W13 = crn->my_newW13;
        vert.my_W31 = crn->my_newW31;
        vert.my_W33 = crn->my_newW33;
        vert.my_W13vid0 = crn->my_newWvid0;
        vert.my_W13vid1 = crn->my_newWvid1;
        vert.my_W31vid0 = -1;
        vert.my_W31vid1 = -1;

        if (crn->my_W13sharpness > 0)
        {
            vert.my_W13sharpness = crn->my_W13sharpness - 1;
        }
        else
        {
            vert.my_W13sharpness = 0;
            vert.my_W13 = Point_3D();
        }
        vert.my_W31sharpness = 0;
        vert.my_W33sharpness = 0;

        vert.my_index = i;
        crn->my_newEindex = i;
        if (crn->my_vertex->isFeature && crn->my_nextCorner->my_vertex->isFeature)
        {
            vert.isFeature = true;
        }
        else
        {
            vert.isFeature = false;
        }
        smesh->my_vertices[i] = vert;
    }
    times("Add E-vertices");

    //create facets
    smesh->my_facets.clear();
    smesh->quadMesh = true;
    numCorners = vecAllCrn.size();
    smesh->my_facets.resize(numCorners);

    smesh->my_sFacets.clear();
    smesh->my_sVertices.clear();
    smesh->my_sValues.clear();

//    #pragma omp parallel for default(none) shared(smesh,numCorners,vecAllCrn), private(i,k,crn,prevcrn,facet,face)
    // THIS DOES NOT WORK IN PARALLEL
    for (i = 0 ; i < numCorners ; i++)
    {
        crn = vecAllCrn[i];
        facet = crn->my_facet;

        face.my_index = i;
        face.my_vertIndices.clear();
        face.my_valency = 4;

        //inherit sharpness for creases
        if (crn->my_sharpness > 0)
        {
            smesh->my_sFacets.push_back(i);
            smesh->my_sVertices.push_back(0);
            smesh->my_sValues.push_back(crn->my_sharpness - 1);
        }

        //add four vertices
        face.my_vertIndices.push_back(crn->my_vertex->my_index);
        smesh->my_vertices[crn->my_vertex->my_index].my_faceIndices.push_back(i);

        if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
        {
            face.my_vertIndices.push_back(crn->my_newEindex);
            smesh->my_vertices[crn->my_newEindex].my_faceIndices.push_back(i);
        }
        else
        {
            face.my_vertIndices.push_back(crn->my_pairedCorner->my_newEindex);
            smesh->my_vertices[crn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(i);
        }

        face.my_vertIndices.push_back(facet->my_index + my_numV);
        smesh->my_vertices[facet->my_index + my_numV].my_faceIndices.push_back(i);

        prevcrn = crn;
        for (k = 0 ; k < facet->my_valency - 1 ; k++)
        {
            prevcrn = prevcrn->my_nextCorner;
        }

        if (prevcrn->my_nextFacet == NULL || facet->my_index < prevcrn->my_nextFacet->my_index) // we are at the boundary or ...
        {
            face.my_vertIndices.push_back(prevcrn->my_newEindex);
            smesh->my_vertices[prevcrn->my_newEindex].my_faceIndices.push_back(i);
        }
        else
        {
            face.my_vertIndices.push_back(prevcrn->my_pairedCorner->my_newEindex);
            smesh->my_vertices[prevcrn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(i);
        }

        smesh->my_facets[i] = face;
    }

    times("Create facets");

    smesh->my_numV = smesh->my_vertices.size();
    smesh->my_numF = smesh->my_facets.size();
    smesh->my_level = my_level + 1;

    smesh->isFootBall = isFootBall;

//    smesh->my_numW = ??

    smesh->my_minz = my_minz;
    smesh->my_maxz = my_maxz;

    cout << "Wavelet Subdivision end" << endl;

    smesh->build();
}

void Mesh::LaplacianSmooth(int lapSmvalue)
{
    unsigned int 				i, j, k;
    MeshVertex					*vertex;
    MeshCorner					*cencrn;
    Point_3D					Vpoi;
    double                      lam;

//    cout << "Laplacian smoothing start" << endl;

    lam = (double)lapSmvalue / 10.0;

    // compute new vertex positions

    for (i = 0 ; i < my_numV ; i++)
    {
        vertex = &my_vertices[i];
        Vpoi = Point_3D();

        //if (vertex->isOnBoundary)
        if (vertex->isStable)
        {
            Vpoi = vertex->my_point;
        }
        else if (vertex->isOnBoundary && vertex->my_valency == 1)
        {
            Vpoi = vertex->my_point;
        }
        else if (vertex->isOnBoundary && vertex->my_valency == 2)
        {
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                cencrn = findCorner(vertex, vertex->my_facets[j]);
                Vpoi += cencrn->my_nextCorner->my_vertex->my_point;
                for (k = 0 ; k < cencrn->my_facet->my_valency - 1 ; k++)
                {
                     cencrn = cencrn->my_nextCorner;
                }
                Vpoi += cencrn->my_vertex->my_point;
            }
            Vpoi *= (lam / 4); // average of edge connected vertices (Bs); the inner one is counted twice
            Vpoi += (1.0 - lam) * vertex->my_point;
        }
        else if (vertex->isOnBoundary && vertex->my_valency > 2)
        {
//            assert(false);
//            cout << "This should not happen for Henrik's images!";
        }
        else
        {
            cencrn = findCorner(vertex, vertex->my_facets[0]);
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Vpoi += cencrn->my_nextCorner->my_vertex->my_point;
                cencrn = getSameCorner(cencrn);
            }
            Vpoi *= (lam / vertex->my_valency); // average of edge connected vertices (Bs)
            Vpoi += (1.0 - lam) * vertex->my_point;
        }
        vertex->my_Vpoi = Vpoi;
    }

    // copy over new (z) positions
    for (i = 0 ; i < my_numV ; i++)
    {
//        my_vertices[i].my_point = my_vertices[i].my_Vpoi;
        my_vertices[i].my_point.setZ(my_vertices[i].my_Vpoi.getZ());
    }

//    cout << "Laplacian Smooth end" << endl;
}

void Mesh::LinearSubdiv(Mesh *smesh)
{
    unsigned int 				i, j, k, cnt;
    MeshFacet 					*facet, face;
    MeshVertex					*vertex, vert;
    MeshCorner					*prevcrn, *crn;
    Point_3D					Fpoi, Epoi, Vpoi, Favrg, Eavrg;

    cout << "LinearSubdiv start" << endl;

    // compute new face-points
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        Fpoi = Point_3D();
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            Fpoi += facet->my_corners[j].my_vertex->my_point;
        }
        Fpoi *= (1.0 / facet->my_valency);
        facet->my_Fpoi = Fpoi;
    }

    // compute new edge-points - each only once!
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);

            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                Epoi = Point_3D();
                Epoi += crn->my_vertex->my_point;
                Epoi += crn->my_nextCorner->my_vertex->my_point;
                Epoi *= 0.5;
                facet->my_corners[j].my_Epoi = Epoi;
            }
        }
    }

    // compute new vertex-points
    for (i = 0 ; i < my_numV ; i++)
    {
        vertex = &my_vertices[i];
        Vpoi = Point_3D();
        Vpoi = vertex->my_point;
        vertex->my_Vpoi = Vpoi;
    }

    // copy over new positions of V-vertices
    smesh->my_vertices.clear();
    for (i = 0 ; i < my_numV ; i++)
    {
        vert.my_point = my_vertices[i].my_Vpoi;
        vert.my_index = i;
        vert.my_faceIndices.clear();
        vert.isFeature = true;
        smesh->my_vertices.push_back(vert);
    }
    // add F-vertices
    for (i = 0 ; i < my_numF ; i++)
    {
        vert.my_point = my_facets[i].my_Fpoi;
        vert.my_index = my_numV + i;
        vert.isFeature = false;
        smesh->my_vertices.push_back(vert);
    }

    //index E-vertices and add them to vertex list
    cnt = smesh->my_vertices.size();
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);

            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                vert.my_point = crn->my_Epoi;
                vert.my_index = cnt;
                crn->my_newEindex = cnt;
                if (crn->my_vertex->isFeature && crn->my_nextCorner->my_vertex->isFeature)
                {
                    vert.isFeature = true;
                }
                else
                {
                    vert.isFeature = false;
                }
                smesh->my_vertices.push_back(vert);
                cnt++;
            }
        }
    }

    //create facets
    smesh->my_facets.clear();
    smesh->quadMesh = true;
    cnt = 0;
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        for (j = 0 ; j < facet->my_valency ; j++)
        {
            crn = &(facet->my_corners[j]);

            face.my_index = cnt;
            face.my_vertIndices.clear();
            face.my_valency = 4;

            //add four vertices
            face.my_vertIndices.push_back(crn->my_vertex->my_index);
            smesh->my_vertices[crn->my_vertex->my_index].my_faceIndices.push_back(cnt);

            if (crn->my_nextFacet == NULL || facet->my_index < crn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                face.my_vertIndices.push_back(crn->my_newEindex);
                smesh->my_vertices[crn->my_newEindex].my_faceIndices.push_back(cnt);
            }
            else
            {
                face.my_vertIndices.push_back(crn->my_pairedCorner->my_newEindex);
                smesh->my_vertices[crn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
            }

            face.my_vertIndices.push_back(facet->my_index + my_numV);
            smesh->my_vertices[facet->my_index + my_numV].my_faceIndices.push_back(cnt);

            prevcrn = crn;
            for (k = 0 ; k < facet->my_valency - 1 ; k++)
            {
                prevcrn = prevcrn->my_nextCorner;
            }

            if (prevcrn->my_nextFacet == NULL || facet->my_index < prevcrn->my_nextFacet->my_index) // we are at the boundary or ...
            {
                face.my_vertIndices.push_back(prevcrn->my_newEindex);
                smesh->my_vertices[prevcrn->my_newEindex].my_faceIndices.push_back(cnt);
            }
            else
            {
                face.my_vertIndices.push_back(prevcrn->my_pairedCorner->my_newEindex);
                smesh->my_vertices[prevcrn->my_pairedCorner->my_newEindex].my_faceIndices.push_back(cnt);
            }

            smesh->my_facets.push_back(face);
            cnt++;
        }
    }

    smesh->my_numV = smesh->my_vertices.size();
    smesh->my_numF = smesh->my_facets.size();
    smesh->my_level = my_level; // NOT +1 FOR LINEAR SUBDIV

    smesh->my_minz = my_minz;
    smesh->my_maxz = my_maxz;

    cout << "LinearSubdiv end" << endl;

    smesh->build();
}

MeshCorner* Mesh::findCorner(MeshVertex *vertex, MeshFacet *facet)
{
    MeshCorner *corner;

    corner = &(facet->my_corners[0]);
    while (corner->my_vIndex != vertex->my_index)
    {
        corner = corner->my_nextCorner;
    }
    return corner;
}

void Mesh::computeWeights(PointPrec x[5], PointPrec y[5], PointPrec weights[4])
{
    PointPrec      a, b, c, D, al1, al2, be1, be2, sum;

    if (x[0] == x[1] && x[2] == x[3])
    {
        cout << "TO DO" << endl; // use y's instead
        assert(false);
    }
    else
    {
        a = -x[1]*y[3]-x[3]*y[2]+x[2]*y[3]+x[1]*y[0]+x[3]*y[1]-x[2]*y[0]-x[0]*y[1]+x[0]*y[2];
        b = -x[0]*y[4]+x[2]*y[0]+x[4]*y[2]+x[1]*y[4]+x[3]*y[4]-x[3]*y[1]-x[4]*y[3]-2.0*x[1]*y[0]+x[1]*y[3]-x[4]*y[1]+2.0*x[0]*y[1]-x[0]*y[2]+x[4]*y[0]-x[2]*y[4];
        c = x[1]*y[0]-x[0]*y[1]+x[0]*y[4]-x[1]*y[4]+x[4]*y[1]-x[4]*y[0];
        D = b * b - 4.0 * a * c;
        if (a == 0)
        {
            if (b == 0)
            {
                cout << "b=0" << endl;
                assert(false);
            }
            else
            {
                be1 = -c / b;
                al1 = (-y[0]+be1*y[0]-be1*y[3]+y[4]) / (-y[0]+y[1]+be1*y[0]-be1*y[1]-be1*y[3]+be1*y[2]);
                if (al1 < 0 || be1 < 0)
                {
                    cout << "al1<0||be<0" << endl;
                    assert(false);
                }
            }
        }
        else
        {
            if (D < 0)
            {
                cout << "D<0" << endl;
                assert(false);
            }
            else
            {
                be1 = (-b + sqrt(D)) / 2.0 / a;
                be2 = (-b - sqrt(D)) / 2.0 / a;
                al1 = (-y[0]+be1*y[0]-be1*y[3]+y[4]) / (-y[0]+y[1]+be1*y[0]-be1*y[1]-be1*y[3]+be1*y[2]);
                al2 = (-y[0]+be2*y[0]-be2*y[3]+y[4]) / (-y[0]+y[1]+be2*y[0]-be2*y[1]-be2*y[3]+be2*y[2]);

                if (be1 < 0)
                {
                    if (be2 < 0)
                    {
                        cout << "be1<0&&be2<0" << endl;
                        assert(false);
                    }
                    else
                    {
                        be1 = be2;
                        al1 = al2;
                    }
                }
            }
        }
    }

    weights[0] = (1 - al1) * (1 - be1);
    weights[1] = al1 * (1 - be1);
    weights[2] = al1 * be1;
    weights[3] = (1 - al1) * be1;

    // testing swapping weights to improve quad shape at EVs
//    weights[2] = (1 - al1) * (1 - be1);
//    weights[1] = al1 * (1 - be1);
//    weights[0] = al1 * be1;
//    weights[3] = (1 - al1) * be1;

    assert(weights[0] >= 0);
    assert(weights[1] >= 0);
    assert(weights[2] >= 0);
    assert(weights[3] >= 0);

    sum =  weights[0] + weights[1] + weights[2] + weights[3];
    assert(abs(1 - sum) < 0.00001);
}

//Compute limit stencil matrix
void Mesh::inverseLimitMesh(Mesh *mesh)
{
    int                         s; // for LU-decomp
    unsigned int 				i, j, val;
    double                      denom, det, detInt, multDenom, lnMultDenom, lnDetInt;
    MeshFacet 					face;
    MeshVertex					*vertex, vert, *prev0, *next0, *prev1, *next1;
    Point_3D                    *poi;
    MeshCorner					*cencrn, *crn;
//    SubdivFacet					subdF;

    // check if the mesh is a pure quad mesh
    if (!quadMesh)
    {
        cout << "The mesh is not a pure quad mesh!" << endl;
        mesh->my_numV = 0;
    }
    else
    {

        // remove low valencies first
        removeLowVal();

        // create mesh
        mesh->my_vertices = my_vertices;
        mesh->my_facets = my_facets;
        mesh->my_level = 0;
        mesh->my_numV = my_numV;
        mesh->my_numF = my_numF;
        mesh->quadMesh = quadMesh;
        mesh->my_centre = my_centre;
        mesh->my_scale = my_scale;
        mesh->isFootBall = isFootBall;

        mesh->build();

    }
}

//Compute limit stencil matrix with colour
void Mesh::inverseLimitMeshColour(Mesh *mesh)
{
    int                         s; // for LU-decomp
    unsigned int 				i, j, val;
    double                      denom, det, detInt, multDenom, lnMultDenom, lnDetInt;
    MeshFacet 					face;
    MeshVertex					*vertex, vert, *prev0, *next0, *prev1, *next1;
    Point_3D                    *poi;
    MeshCorner					*cencrn, *crn;
//    SubdivFacet					subdF;


    // check if the mesh is a pure quad mesh
    if (!quadMesh)
    {
        cout << "The mesh is not a pure quad mesh!" << endl;
        mesh->my_numV = 0;
    }
    else
    {
        // remove low valencies first
        removeLowVal();

        // create mesh
        mesh->my_vertices = my_vertices;
        mesh->my_facets = my_facets;
        mesh->my_level = 0;
        mesh->my_numV = my_numV;
        mesh->my_numF = my_numF;
        mesh->quadMesh = quadMesh;
        mesh->my_centre = my_centre;
        mesh->my_scale = my_scale;
        mesh->isFootBall = isFootBall;

        mesh->build();

    }
}

void Mesh::lifeInit(void)
{
}

void Mesh::lifeStep(void)
{
}

void Mesh::times (const char *which) {
/* If which is not empty, print the times since the previous call. */
/*    static double last_wall = 0.0, last_cpu = 0.0;
    double wall, cpu;
    struct timeval tv;
    clock_t stamp;

    wall = last_wall;
    cpu = last_cpu;
    if (gettimeofday(&tv,NULL) != 0 ||
            (stamp = clock()) == (clock_t)-1)
        cout << "Time failed" << endl;
    last_wall = tv.tv_sec+1.0e-6*tv.tv_usec;
    last_cpu = stamp/(double)CLOCKS_PER_SEC;
    if (strlen(which) > 0) {
        wall = last_wall-wall;
        cpu = last_cpu-cpu;*/
//        cout << which << " time = " << wall << "s , CPU = " << cpu << "s" << endl;
//        printf("%s time = %.2f seconds, CPU = %.2f seconds\n",which,wall,cpu);
//    }
}

MeshCorner *Mesh::getMainCrn(MeshCorner *crn)
{
    if (crn->my_pairedCorner == NULL)
    {
        return(crn);
    }
    else
    {
        if (crn->my_facet->my_index < crn->my_nextFacet->my_index)
        {
            return(crn);
        }
        else
        {
            return(crn->my_pairedCorner);
        }
    }
}

unsigned int Mesh::decideSharpness(unsigned int s1, unsigned int s2)
{
    return qMax(s1, s2);
}

unsigned int Mesh::decideSharpness(unsigned int s1, unsigned int s2, unsigned int s3, unsigned int s4)
{
    return qMax(qMax(s1, s2), qMax(s3, s4));
}

//Catmull-Clark limit
void Mesh::CatmullClarkLimit(void)
{
    unsigned int 				i, j;
    MeshFacet 					*facet;
    MeshVertex					*vertex, *prev0, *prev1, *next0, *next1;
    MeshCorner					*cencrn, *crn;
    Point_3D					Fpoi, Vpoi, Favrg, Eavrg;

    // compute face centroids
    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        Fpoi = Point_3D();

        for (j = 0 ; j < facet->my_valency ; j++)
        {
            Fpoi += facet->my_corners[j].my_vertex->my_point;
        }
        Fpoi *= (1.0 / facet->my_valency);
        facet->my_Fpoi = Fpoi;
    }

    // compute new vertex-points
    for (i = 0 ; i < my_numV ; i++)
    {
        vertex = &my_vertices[i];

        Vpoi = Point_3D();

        if (vertex->isOnBoundary)
        {
            if (vertex->my_valency == 1)
            {
                Vpoi = vertex->my_point;
            }
            else if (vertex->my_valency == 2)
            {
                Vpoi = 4 * vertex->my_point;

                    crn = findCorner(vertex, vertex->my_facets[0]);
                    next0 = crn->my_nextCorner->my_vertex;
                    for (j = 0 ; j < vertex->my_facets[0]->my_valency - 1 ; j++)
                    {
                        crn = crn->my_nextCorner;
                    }
                    prev0 = crn->my_vertex;

                    crn = findCorner(vertex, vertex->my_facets[1]);
                    next1 = crn->my_nextCorner->my_vertex;
                    for (j = 0 ; j < vertex->my_facets[1]->my_valency - 1 ; j++)
                    {
                        crn = crn->my_nextCorner;
                    }
                    prev1 = crn->my_vertex;

                    if (next0->my_index == prev1->my_index)
                    {
                        Vpoi += prev0->my_point + next1->my_point;
                    }
                    else
                    {
                        Vpoi += prev1->my_point + next0->my_point;
                    }

                Vpoi *= 1.0 / 6.0;
            }
            else
            {
//                cout << "Something is wrong at the boundary!" << endl;
//                assert(false); // something is wrong at the boundary!

                // instead of crashing, interpolate the point
                Vpoi = vertex->my_point;
            }
        }
        else
        {
            Favrg = Point_3D();
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Favrg += vertex->my_facets[j]->my_Fpoi;
            }
            Favrg *= (1.0 / vertex->my_valency);

            Eavrg = Point_3D();

            cencrn = findCorner(vertex, vertex->my_facets[0]);
            for (j = 0 ; j < vertex->my_valency ; j++)
            {
                Eavrg += cencrn->my_nextCorner->my_vertex->my_point;
                cencrn = getSameCorner(cencrn);
            }
            Eavrg *= (1.0 / vertex->my_valency);
            Eavrg += vertex->my_point;
            Eavrg *= 0.5;

            Vpoi = (4.0 / (vertex->my_valency + 5.0)) * (Favrg + Eavrg);
            Vpoi += ((vertex->my_valency - 3.0) / (vertex->my_valency + 5.0)) * vertex->my_point;

        }
        vertex->my_CClimit = Vpoi;
    }

//    #pragma omp parallel for default(none) shared(smesh),private(i,vert)
//    for (i = 0 ; i < my_numV ; i++)
//    {
//        my_vertices[i].my_point = my_vertices[i].my_CClimit;
//    }
}

bool Mesh::isLshape(MeshVertex *v)
{
    MeshCorner  *crn;

    if (v->my_W13sharpness < 1 || v->isOnBoundary || v->my_W13vid0 < 0 || v->my_W13vid0 >= my_numV)
    {
        return false;
    }

    if (v->my_valency != 4 &&
        v->my_W13vid0 > -1 && v->my_W13vid0 < my_numV && v->my_W13vid1 > -1 && v->my_W13vid1 < my_numV &&
        v->my_W13sharpness > 0)
    {
        return true;
    }
    else
    {
        crn = findCorner(v, v->my_facets[0]);
        while (crn->my_nIndex != v->my_W13vid0)
        {
            crn = getSameCorner(crn);
        }
        if (getSameCorner(getSameCorner(crn))->my_nIndex == v->my_W13vid1)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
}

void Mesh::convert2v3()
{
    v3 = true;
    for (unsigned int i = 0; i < my_vertices.size(); i++)
        for (int j = 0; j < my_vertices[i].weight_ids.size(); j++) {
            Point_3D midP = (my_vertices[i].my_point+my_vertices[my_vertices[i].weight_ids[j]].my_point)*0.5;
            float w = my_vertices[i].weights[j];
            Point_3D epoi = (1-w)*my_vertices[i].my_point+w*midP;
            my_vertices[i].weights_vec.append(epoi-my_vertices[i].my_point);
        }
}

void Mesh::flipColourSpace()
{
    useRGB = !useRGB;
    double r=0,g=0,b=0,L=0,a=0,b2=0;
    if(useRGB) {
        for(unsigned int i = 0; i < my_vertices.size(); i++) {
            L = my_vertices[i].my_colour.getX();
            a = my_vertices[i].my_colour.getY();
            b2 = my_vertices[i].my_colour.getZ();
            LAB2RGB(L,a,b2,r,g,b);
            my_vertices[i].my_colour.setX(r);
            my_vertices[i].my_colour.setY(g);
            my_vertices[i].my_colour.setZ(b);
        }
    } else {
        for(unsigned int i = 0; i < my_vertices.size(); i++) {
            r = my_vertices[i].my_colour.getX();
            g = my_vertices[i].my_colour.getY();
            b = my_vertices[i].my_colour.getZ();
            RGB2LAB(r,g,b,L,a,b2);
            my_vertices[i].my_colour.setX(L);
            my_vertices[i].my_colour.setY(a);
            my_vertices[i].my_colour.setZ(b2);
        }
    }
}

void Mesh::spreadOrgCol1()
{
    for(unsigned int i = 0; i < my_vertP1.size(); i++) {
        std::vector< std::vector <unsigned int> > ring_facets;
        std::vector< std::vector <unsigned int> > ring_vertices;
        getRingsV(&my_vertices[my_vertP1[i]], 1, &ring_facets, &ring_vertices);
        for(unsigned int j = 0; j < ring_vertices[1].size();j++) {
            my_vertices[ring_vertices[1][j]].my_colour.setX(my_vertices[my_vertP1[i]].my_colour.getX());
            my_vertices[ring_vertices[1][j]].my_colour.setY(my_vertices[my_vertP1[i]].my_colour.getY());
            my_vertices[ring_vertices[1][j]].my_colour.setZ(my_vertices[my_vertP1[i]].my_colour.getZ());
        }
    }
}

void Mesh::spreadOrgCol2()
{
    for(unsigned int i = 0; i < my_vertices.size(); i++) {
        if(my_vertices[i].isFeature) {
            std::vector< std::vector <unsigned int> > ring_facets;
            std::vector< std::vector <unsigned int> > ring_vertices;
            getRingsV(&my_vertices[i], 1, &ring_facets, &ring_vertices);
            for(unsigned int j = 0; j < ring_vertices[1].size();j++) {
                my_vertices[ring_vertices[1][j]].my_colour.setX(my_vertices[i].my_colour.getX());
                my_vertices[ring_vertices[1][j]].my_colour.setY(my_vertices[i].my_colour.getY());
                my_vertices[ring_vertices[1][j]].my_colour.setZ(my_vertices[i].my_colour.getZ());
            }
        }
    }
}

void Mesh::freeSelection()
{
    for(unsigned int i = 0; i < my_vertices.size(); i++)
        if(my_vertices[i].isSelected) my_vertices[i].isSelected = false;
}

void Mesh::increaseWeights()
{
    float orgX,orgY;
    for(unsigned int i = 0; i < my_vertices.size(); i++)
        if(my_vertices[i].isSelected) {
            float *coords = my_vertices[i].my_point.getCoords();
            if(xyWeights) {
                orgX = coords[0]/coords[2];
                orgY = coords[1]/coords[2];
            }
            coords[2] += 0.5;
            if(xyWeights) {
                coords[0] = orgX*coords[2];
                coords[1] = orgY*coords[2];
            }
        }
}

void Mesh::decreaseWeights()
{
    float orgX,orgY;
    for(unsigned int i = 0; i < my_vertices.size(); i++)
        if(my_vertices[i].isSelected) {
            float *coords = my_vertices[i].my_point.getCoords();
            if(xyWeights) {
                orgX = coords[0]/coords[2];
                orgY = coords[1]/coords[2];
            }
            coords[2] -= 0.5; if(coords[2]<=0.1) coords[2] = .00001;
            if(xyWeights) {
                coords[0] = orgX*coords[2];
                coords[1] = orgY*coords[2];
            }
        }
}

void Mesh::increaseLocalWeights()
{
    for(unsigned int i = 0; i < my_numV; i++)
    {
        for(int j = 0; j < my_vertices[i].weights.size(); j++) {
            float tmp = my_vertices[i].weights[j]+0.01f;
            if(tmp <= 1.0f)
                my_vertices[i].weights[j] = tmp;
        }
    }
}

void Mesh::decreaseLocalWeights()
{
    for(unsigned int i = 0; i < my_numV; i++)
    {
        for(int j = 0; j < my_vertices[i].weights.size(); j++) {
            float tmp = my_vertices[i].weights[j]-0.01f;
            if(tmp >= .0f)
                my_vertices[i].weights[j] = tmp;
        }
    }
}

void Mesh::snapSelected()
{
    unsigned int id1=0,id2=my_vertices.size();
    bool id1_f=false;

    // find selected
    for(unsigned int i = 0; i < my_vertices.size(); i++)
        if(my_vertices[i].isSelected) {
            if(!id1_f){
                id1 = i;
                id1_f=true;
            } else {
                id2 = i;
                break;
            }
        }
    if(id2==my_vertices.size()) return; // only two points can be selected

    // snap vertices
    Point_3D av = (my_vertices[id1].my_point+my_vertices[id2].my_point)*0.5;

    if(my_vertices[id1].snapped && my_vertices[id2].snapped) {
        my_vertices[id1].snapped = false;
        my_vertices[id2].snapped = false;
        my_vertices[id1].my_point = my_vertices[id1].my_visPoint;
        my_vertices[id2].my_point = my_vertices[id2].my_visPoint;
    } else {
        my_vertices[id1].snapped = true;
        my_vertices[id2].snapped = true;
        my_vertices[id1].my_visPoint = my_vertices[id1].my_point;
        my_vertices[id2].my_visPoint = my_vertices[id2].my_point;
        my_vertices[id1].my_point = av;
        my_vertices[id2].my_point = av;
    }
}

void Mesh::changeSharpness(int change)
{
    int id1=0,id2=my_vertices.size();
    bool id1_f=false;

    // find selected
    for(unsigned int i = 0; i < my_vertices.size(); i++)
        if(my_vertices[i].isSelected) {
            if(!id1_f){
                id1 = i;
                id1_f=true;
            } else {
                id2 = i;
                break;
            }
        }
    if(id2==(int)my_vertices.size()) return; // only two points can be selected

    for(int i = 0; i < my_vertices[id1].sharpness.size(); i++)
        if(my_vertices[id1].weight_ids[i] == id2)
            my_vertices[id1].sharpness[i] += change;
    for(int i = 0; i < my_vertices[id2].sharpness.size(); i++)
        if(my_vertices[id2].weight_ids[i] == id1)
            my_vertices[id2].sharpness[i] += change;
    build();
}

void Mesh::draw(QOpenGLFunctions_1_0* context)
{
    unsigned int 	i, j;
    MeshFacet		*facet;
    double           r=0,g=0,b=0;
    float           *coords, x,y,RGB[3];

    for (i = 0 ; i < my_numF ; i++)
    {
        facet = &my_facets[i];
        context->glBegin(GL_POLYGON);
        for(j = 0 ; j < facet->my_valency ; j++)
        {
            coords = facet->my_corners[j].my_vertex->my_point.getCoords();
            if(!useRGB)
                LAB2RGB(facet->my_corners[j].my_vertex->my_colour.getX(),
                        facet->my_corners[j].my_vertex->my_colour.getY(),
                        facet->my_corners[j].my_vertex->my_colour.getZ(),r,g,b);
            else {
                r = facet->my_corners[j].my_vertex->my_colour.getX();
                g = facet->my_corners[j].my_vertex->my_colour.getY();
                b = facet->my_corners[j].my_vertex->my_colour.getZ();
            }
            RGB[0] = (float)r;RGB[1] = (float)g;RGB[2] = (float)b;
            context->glColor3fv(RGB);
            if(xyWeights) {
                x = coords[0]/coords[2];
                y = coords[1]/coords[2];
            } else {
                x = coords[0];
                y = coords[1];
            }
            context->glVertex3f(x,y,.0f);
        }
        context->glEnd();
    }
}

void Mesh::addEdge(vector<int> *edge, int id1, int id2)
{
    bool exists = false;
    for(int i = 0; i < (int)edge->size()/2; i++) {
        if((edge->at(2*i)==id1 && edge->at(2*i+1)==id2) ||
           (edge->at(2*i)==id2 && edge->at(2*i+1)==id1)) {
            exists = true;
            break;
        }
    }

    if(!exists) {
        (*edge)[2*id1] = id1;
        (*edge)[2*id1+1] = id2;
    }
}

void Mesh::getRightCoords(vector<double> *gtCoords, vector<double> *gnCoords, vector<int> *edge,
                          double *gtRight, double *gnRight, int index, int target)
{
    for(int i = 0; i < (int)edge->size()/2; i++) {
        if(edge->at(2*i) == index && edge->at(2*i+1)== target)
        {
            *gtRight = gtCoords->at(2*i+1);
            *gnRight = gnCoords->at(2*i+1);
            break;
        }
        if(edge->at(2*i) == target && edge->at(2*i+1)== index)
        {
            *gtRight = gtCoords->at(2*i);
            *gnRight = gnCoords->at(2*i);
            break;
        }
    }
}

void Mesh::getLeftCoords(vector<double> *gtCoords, vector<double> *gnCoords, vector<int> *edge,
                          double *gtLeft, double *gnLeft, int index, int target)
{
    for(int i = 0; i < (int)edge->size()/2; i++) {
        if(edge->at(2*i) == index && edge->at(2*i+1)== target) {
            *gtLeft = gtCoords->at(2*i);
            *gnLeft = gnCoords->at(2*i);
            break;
        }
        if(edge->at(2*i) == target && edge->at(2*i+1)== index)
        {
            *gtLeft = gtCoords->at(2*i+1);
            *gnLeft = gnCoords->at(2*i+1);
            break;
        }
    }
}
