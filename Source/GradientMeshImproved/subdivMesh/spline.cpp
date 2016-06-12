#include <stdlib.h>
#include <fstream>
#include "spline.h"
#include <cmath>
#include <string>

using namespace std;
using namespace subdivMesh;

SplineC::SplineC(void)
{
    my_numv = 0;
    my_k = 0;
}

SplineC::SplineC(const SplineC &orig)
{
    my_numv = orig.my_numv;
    my_refNumv = orig.my_refNumv;
    my_k = orig.my_k;
    my_deg = orig.my_deg;
    my_tau = orig.my_tau;
    my_refTau = orig.my_refTau;
    my_plg = orig.my_plg;
    my_refPlg = orig.my_refPlg;
}

SplineC::SplineC(int k, KnotVec tau, CtrlPlg plg)
{
    my_numv = plg.size() - 1;
    my_k = k;
    my_deg = k - 1;
    my_tau = tau;
    my_plg = plg;
}

SplineC::~SplineC(void)
{
}

void SplineC::writeC(const char *my_file)
{
    ofstream 		file (my_file);
    unsigned int 	i;

    if (file.is_open())
    {
        file << my_deg << endl;
        file << (my_numv + 1) << endl;
        for (i = 0 ; i < my_tau.size() ; i++)
        {
            file << my_tau[i] << " ";
        }
        file << endl;

        for (i = 0 ; i <= my_numv ; i++)
        {
            file << my_plg[i];
        }
        file.close();
        cout << "Data written to " << my_file << endl;
    }
    else
    {
    cout << "Unable to open file for writing." << endl;
    }
}

void SplineC::readC(const char *my_file)
{
    unsigned int	i, tmp;

    PointPrec	knot;
    Point_3D 	poi;
    ifstream 	file (my_file);

    my_tau.clear();
    my_plg.clear();

    if (file.is_open())
    {
        file >> my_deg;
        my_k = my_deg + 1;
        file >> tmp;
        my_numv = tmp - 1;
        for (i = 0 ; i < my_numv + my_deg + 2 ; i++)
        {
            file >> knot;
            my_tau.push_back(knot);
        }
        for (i = 0 ; i < tmp ; i++)
        {
            file >> poi;
            my_plg.push_back(poi);
        }
        file.close();
    }
    else
    {
    cout << "Unable to open file for reading." << endl;
    }
}

int SplineC::findInt(int kn, KnotVec refTau, int j)
{
    int i;


    for (i = 0 ; i <= kn - 1 ; i++)
    {
        if ((my_tau[i] <= refTau[j]) && (refTau[j] < my_tau[i + 1]))
        {
//	cout << "Reftau[" << j << "]: " << my_refTau[j] << " for i: " << i << endl;
            return i;
        }
    }
//    cout << "Knot vector problem in 'findInt'!" << endl;
//    cout << "Or Bezier end conditions!" << endl;
    // hack!
    return my_tau.size() - my_deg - 2;

//	exit(EXIT_FAILURE);
}

Point_3D SplineC::findPoi(KnotVec refTau, int rp1, int i, int j)
{
    int 		r;
    PointPrec 	p1, p2, p12;
    Point_3D 	poi1, poi2, tmp1, tmp2;

    r = rp1 - 1;
    if (r > 0)
    {
        poi1 = Point_3D(0,0,0);
        poi2 = Point_3D(0,0,0);
        p1 = refTau[j + my_k - r] - my_tau[i];
        p2 = my_tau[i + my_k - r] - refTau[j + my_k - r];

        if (p1 != 0)
        {
            poi1 = findPoi(refTau, r, i, j);
        }

        if (p2 != 0)
        {
            poi2 = findPoi(refTau, r, i - 1, j);
        }

        p12 = p1 + p2;
        tmp1 = (p1 / p12) * poi1;
        tmp2 = (p2 / p12) * poi2;
        return tmp1 + tmp2;
    }
    else
    {
        if (i < 0)
        {
            return my_plg[0];
        }
        else if (i > (int)my_plg.size() - 1)
        {
            return my_plg[my_plg.size() - 1];
        }

        else
        {
            return my_plg[i];
        }
    }
}

void SplineC::oslo()
{
    unsigned int 	q, mu, j;
    Point_3D 		poi;

    my_refPlg.clear();

    q = my_refTau.size() - 1;

    for (j = 0 ; j <= q - my_k ; j++)
    {
        mu = findInt(my_k + my_numv, my_refTau, j);
        poi = findPoi(my_refTau, my_k, mu, j);
        my_refPlg.push_back(poi);
    }
    my_refNumv = my_refPlg.size() - 1;
}

void SplineC::refine(void)
{
    unsigned int i;

    // shorten knot vector at each end
    my_tau.clear();
    for (i = floor(my_deg/2) ; i < my_refTau.size() - floor(my_deg/2) ; i++)
    {
        my_tau.push_back(my_refTau[i]);
    }
    my_refTau.clear();
    // shorten control net at each end
    my_plg.clear();
    for (i = floor(my_deg/2) ; i < my_refPlg.size() - floor(my_deg/2) ; i++)
    {
        my_plg.push_back(my_refPlg[i]);
    }
    my_refPlg.clear();
    my_numv = my_plg.size() - 1;
    my_refNumv = 0;
}

void SplineC::refineRay(void)
{
    my_tau = my_refTau;
    my_refTau.clear();
    my_plg = my_refPlg;
    my_refPlg.clear();
    my_numv = my_plg.size() - 1;
    my_refNumv = 0;
}

void SplineC::setUniSubdKnotVector(void)
{
    unsigned int i, ce;

//cout << "Old: ";
//	for (i = 0 ; i < my_tau.size(); i++)
//	{
//		cout << my_tau[i] << " ";
//	}
//cout << endl;

    my_refTau.clear();
	ce = ceil((float)my_deg / 2);

    // copy first few knots over
    for (i = 0 ; i < ce; i++)
    {
        my_refTau.push_back(my_tau[i]);
    }
    // half non-zero intervals
    for (i = ce ; i < my_tau.size() - (ce + 1); i++)
    {
        if (my_tau[i] == my_tau[i + 1] && i >= ce + 1 && i <= my_tau.size() - ce - 3)
        {
            my_refTau.push_back(my_tau[i]);
        }
        else
        {
            my_refTau.push_back(my_tau[i]);
            my_refTau.push_back((my_tau[i] + my_tau[i + 1]) / 2);
        }
    }
    // copy last knots over
    for (i = my_tau.size() - (ce + 1) ; i < my_tau.size(); i++)
    {
        my_refTau.push_back(my_tau[i]);
    }

//cout << "New: ";
//	for (i = 0 ; i < my_refTau.size(); i++)
//	{
//		cout << my_refTau[i] << " ";
//	}
//cout << endl;

}

void SplineC::uniSubdiv(void)
{
    setUniSubdKnotVector();
    oslo();
    refine();
}

void SplineC::insertMultiGen(void)
{
//	KnotVec	tmp;
//
//	my_refNumv = my_refTau.size() - my_deg - 2;

//	if (my_refTau.size() != my_tau.size())
//	{
//		oslo();
//		refine();
//	}
}

istream &operator>>(istream &is, SplineC &curve)
{
    unsigned int	i, nmv;
    Point_3D		poi;
    char			chr;
    PointPrec		knot;

    curve.my_tau.clear();
    curve.my_plg.clear();

    is >> chr; 	// just a character separating curves
    is >> nmv;
    curve.my_numv = nmv - 1;

    for (i = 0 ; i < curve.my_numv + curve.my_deg + 2 ; i++)
    {
        is >> knot;
        curve.my_tau.push_back(knot);
    }

    poi = Point_3D();

    for (i = 0 ; i < curve.my_numv + 1 ; i++)
    {
        is >> poi;
        curve.my_plg.push_back(poi);
    }
    return is;
}

ostream &operator<<(ostream &os, const SplineC &curve)
{
    unsigned int i;

    os << "\%" << endl;
    os << curve.my_numv + 1 << endl;

    for (i = 0 ; i < curve.my_tau.size() ; i++)
    {
        os << curve.my_tau[i] << " ";
    }
    os << endl;
    for (i = 0 ; i < curve.my_plg.size() ; i++)
    {
        os << curve.my_plg[i];
    }
    return os;
}

// #########################################################################

SplineS::SplineS(void)
{
    my_numv1 = 0;
    my_numv2 = 0;
    my_k1 = 0;
    my_k2 = 0;
    my_deg1 = 0;
    my_deg2 = 0;
    my_level = 0;
    my_net.clear();
}

SplineS::SplineS(const SplineS &orig)
{
    my_numv1 = orig.my_numv1;
    my_numv2 = orig.my_numv2;
    my_refNumv1 = orig.my_refNumv1;
    my_refNumv2 = orig.my_refNumv2;
    my_k1 = orig.my_k1;
    my_k2 = orig.my_k2;
    my_deg1 = orig.my_deg1;
    my_deg2 = orig.my_deg2;
    my_level = orig.my_level;
    my_tau1 = orig.my_tau1;
    my_tau2 = orig.my_tau2;
    my_refTau1 = orig.my_refTau1;
    my_refTau2 = orig.my_refTau2;
    my_net = orig.my_net;
    my_refNet = orig.my_refNet;
    my_s = orig.my_s;
}

SplineS::SplineS(int k1, int k2, KnotVec tau1, KnotVec tau2, CtrlNet net,
                 int level)
{
    my_numv1 = net.size() - 1;
    my_numv2 = net[0].size() - 1;
    my_k1 = k1;
    my_k2 = k2;
    my_deg1 = k1 - 1;
    my_deg2 = k2 - 1;
    my_tau1 = tau1;
    my_tau2 = tau2;
    my_net = net;
    my_level = level;
}

SplineS::~SplineS(void)
{
}

void SplineS::readS(const char *my_file)
{
    unsigned int	i, j, tmp1, tmp2;
    PointPrec	knot;
    Point_3D 	poi;
    CtrlPlg		plg;
    KnotVec		knotInts;
    ifstream 	file (my_file);

    my_tau1.clear();
    my_tau2.clear();
    my_net.clear();
    my_level = 0;

    if (file.is_open())
    {
        cout << "===== Start reading file =====" << endl;
        getline(file, my_s);
        cout << my_s << endl;
        file >> my_deg1 >> my_deg2;
        my_k1 = my_deg1 + 1;
        my_k2 = my_deg2 + 1;
        file >> tmp1 >> tmp2;
        my_numv1 = tmp1 - 1;
        my_numv2 = tmp2 - 1;
        for (i = 0 ; i < my_numv1 + my_deg1 + 2 ; i++)
        {
            file >> knot;
            my_tau1.push_back(knot);
        }
        for (i = 0 ; i < my_numv2 + my_deg2 + 2 ; i++)
        {
            file >> knot;
            my_tau2.push_back(knot);
        }

        for (i = 0 ; i < tmp1 ; i++)
        {
            for (j = 0 ; j < tmp2 ; j++)
            {
                file >> poi;
                plg.push_back(poi);
            }
            my_net.push_back(plg);
            plg.clear();
        }

        file.close();
        cout << "Net: " << my_numv1 + 1 << " x " << my_numv2 + 1<< endl;
        cout << "===== Done reading file =====" << endl;
    }
    else
    {
    cout << "Unable to open file for reading." << endl;
    }
}

void SplineS::writeS(const char *my_file)
{
//  unsigned int	i, j, tmp1, tmp2;
    ofstream 	file (my_file);

//    if (file.is_open())
//	{
//
//		cout << "===== Start writing file =====" << endl;
//		file << my_s << endl;
//		file << my_deg1 << endl << my_deg2 << endl;
//		tmp1 = my_numv1 + 1;
//		tmp2 = my_numv2 + 1;
//		file << tmp1 << endl << tmp2 << endl;

//		//writing control mesh
//		for (i = 0 ; i < tmp1 ; i++)
//		{
//			file << "\%" << endl;
//			for (j = 0 ; j < tmp2 ; j++)
//			{
//		    	file << my_net[i][j];
//			}
//		}

//		file.close();
//		cout << "===== Done writing file =====" << endl;
//	}
//	else
//	{
//	cout << "Unable to open file for reading." << endl;
//	}
}

void SplineS::coutS(void)
{
    unsigned int i, j;

    if (my_k1 == 0) cout << "Empty" << endl;
    else
    {
        cout << my_deg1 << " " << my_deg2 << endl;
        cout << my_numv1 + 1 << " " << my_numv2 + 1<< endl;
        cout << "tau1: ";
        for (i = 0 ; i < my_tau1.size() ; i++)
        {
            cout << my_tau1[i] << " ";
        }
        cout << endl;
        cout << "tau2: ";
        for (i = 0 ; i < my_tau2.size() ; i++)
        {
            cout << my_tau2[i] << " ";
        }
        cout << endl;

        for (i = 0 ; i <= my_numv1 ; i++)
        {
            for (j = 0 ; j <= my_numv2 ; j++)
            {
                cout << my_net[i][j];
            }
            cout << endl;
        }
    }
}

void SplineS::swapNet(void)
{
    unsigned int 	i, j, tmp;
    CtrlPlg			plg;
    CtrlNet			net;
    KnotVec			tau;
    KnotNet			knet;

    net.clear();
    for (i = 0 ; i <= my_numv2 ; i++)
    {
        for (j = 0 ; j <= my_numv1 ; j++)
        {
            plg.push_back(my_net[j][i]);
        }
        net.push_back(plg);
        plg.clear();
    }
    my_net = net;

    tmp = my_numv1;
    my_numv1 = my_numv2;
    my_numv2 = tmp;
    tmp = my_deg1;
    my_deg1 = my_deg2;
    my_deg2 = tmp;
    tmp = my_k1;
    my_k1 = my_k2;
    my_k2 = tmp;
    tau = my_tau1;
    my_tau1 = my_tau2;
    my_tau2 = tau;
}

void SplineS::uSubdiv(void)
{
    unsigned int 	i;
    SplineC			spl;
    CtrlPlg			plg;
    KnotVec			tau;

    my_refNet.clear();

    for (i = 0 ; i <= my_numv1 ; i++)
    {
        plg = my_net[i];
        spl = SplineC(my_k2, my_tau2, plg);
        spl.uniSubdiv();
        my_refNet.push_back(spl.my_plg);
    }

    my_numv2 = my_refNet[0].size() - 1;
    my_tau2 = spl.my_tau;
    my_refTau2.clear();

    my_net = my_refNet;
    my_refNet.clear();
}

void SplineS::uvSubdiv(void)
{
    uSubdiv();
    swapNet();
    uSubdiv();
    swapNet();
    my_level++;
}

void SplineS::diffU(SplineS *der)
{
    unsigned int 	i, j;
    CtrlPlg			plg;
    Point_3D		poi, tmp, inf;
    PointPrec		mult;

    der->my_numv1 = my_numv1;
    der->my_numv2 = my_numv2 - 1;

//	// generate uniform taus (only for hayes version)
//    my_tau1.clear();
//	for (j = 0 ; j < my_deg1 + 1 ; j++)
//	{
//		my_tau1.push_back(0);
//	}
//	for (j = 0 ; j < my_numv1 - my_deg1 ; j++)
//	{
//		my_tau1.push_back(j + 1);
//	}
//	for (j = 0 ; j < my_deg1 + 1; j++)
//	{
//		my_tau1.push_back(my_numv1 - my_deg1 + 1);
//	}
//    my_tau2.clear();
//	for (j = 0 ; j < my_deg2 + 1 ; j++)
//	{
//		my_tau2.push_back(0);
//	}
//	for (j = 0 ; j < my_numv2 - my_deg2 ; j++)
//	{
//		my_tau2.push_back(j + 1);
//	}
//	for (j = 0 ; j < my_deg2 + 1; j++)
//	{
//		my_tau2.push_back(my_numv2 - my_deg2 + 1);
//	}

    der->my_refNumv1 = 0;
    der->my_refNumv2 = 0;
    der->my_k1 = my_k1;
    der->my_k2 = my_k2 - 1;
    der->my_deg1 = my_deg1;
    der->my_deg2 = my_deg2 - 1;
    der->my_level = my_level;

    der->my_tau1 = my_tau1;
    der->my_tau2 = my_tau2;
    der->my_refTau1.clear();
    der->my_refTau2.clear();

    der->my_tau2.erase(der->my_tau2.begin());
    der->my_tau2.pop_back();
    der->my_net.clear();
    der->my_refNet.clear();

    for (i = 0 ; i < my_net.size() ; i++)
    {
        plg.clear();
        for (j = 0 ; j < my_net[i].size() - 1 ; j++)
        {
            if (my_tau2[j + my_deg2 + 1] - my_tau2[j + 1] != 0)
            {
                mult = my_deg2 / (my_tau2[j + my_deg2 + 1] - my_tau2[j + 1]);
                poi = -mult * my_net[i][j];
                tmp = mult * my_net[i][j + 1];
                poi = poi + tmp;
                plg.push_back(poi);
            }
//			else if (abs(my_net[i][j + 1].getX() - my_net[i][j].getX()) <= EPS &&
//			         abs(my_net[i][j + 1].getY() - my_net[i][j].getY()) <= EPS &&
//			         abs(my_net[i][j + 1].getZ() - my_net[i][j].getZ()) <= EPS )
            else
            {
//				cout << "diff: " << abs(my_net[i][j + 1].getX() - my_net[i][j].getX()) <<
//				abs(my_net[i][j + 1].getY() - my_net[i][j].getY()) <<
//				abs(my_net[i][j + 1].getZ() - my_net[i][j].getZ()) << endl;

                //fix for crease
                plg.push_back(plg.back());
            }
        }
        der->my_net.push_back(plg);
    }
}

void SplineS::diffV(SplineS *der)
{
    swapNet();
    diffU(der);
    der->swapNet();
    swapNet();
}
