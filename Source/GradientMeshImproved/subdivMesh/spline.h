#ifndef SPLINE_H
#define SPLINE_H

#include <iostream>
#include <vector>
#include "types.h"
#include "point_3d.h"

namespace subdivMesh {

typedef std::vector < Point_3D > CtrlPlg;
typedef std::vector < std::vector < Point_3D > > CtrlNet;

const PointPrec EPS = 10;

class SplineC
{
	friend std::ostream &operator<<(std::ostream &os, const SplineC &curve);
	friend std::istream &operator>>(std::istream &is, SplineC &curve);
	
public:
    SplineC(void);
	SplineC(const SplineC &orig);
	SplineC(int k, KnotVec tau, CtrlPlg plg);
	~SplineC(void);

	void writeC(const char *my_file);
	void readC(const char *my_file);

	int 		findInt(int kn, KnotVec refTau, int j);
	Point_3D 	findPoi(KnotVec refTau, int rp1, int i, int j);
	void 		oslo();
	void 		refine(void);
	void 		refineRay(void);	
	
	
	void		setUniSubdKnotVector(void);
	void		uniSubdiv(void);
	
	void 		insertMultiGen(void);	
	
	unsigned int		my_numv,  	// numv+1 vertices
						my_refNumv,	// refn+1 vertices after Oslo alg.
						my_k,  		// order of the spline
						my_deg;		// degree

	KnotVec				my_tau,		// knot vector
						my_refTau;	// refined knot vector

	CtrlPlg				my_plg,  	// control polygon
						my_refPlg;  // refined polygon
};

std::istream &operator>>(std::istream &is, SplineC &curve);
std::ostream &operator<<(std::ostream &os, const SplineC &curve);

class SplineS
{
public:
    SplineS(void);
	SplineS(const SplineS &orig);
	SplineS(int k1, int k2, KnotVec tau1, KnotVec tau2, CtrlNet net, int level);
	~SplineS(void);

	void readS(const char *my_file);
	void writeS(const char *my_file);
	void coutS(void);
	void uSubdiv(void);
	void swapNet(void);
	void uvSubdiv(void);
	void diffU(SplineS *der);
	void diffV(SplineS *der);

	unsigned int		my_numv1,  	// numv+1 vertices
						my_numv2,  	// numv+1 vertices
						my_refNumv1,	// refn+1 vertices after Oslo alg.
						my_refNumv2,	// refn+1 vertices after Oslo alg.
						my_k1,  		// order of the spline
						my_k2,  		// order of the spline
						my_deg1,		// degree
						my_deg2;		// degree
	int					my_level;		// subdiv level

	KnotVec				my_tau1,		// knot vector
						my_tau2,
						my_refTau1,	// refined knot vector
						my_refTau2;	// refined knot vector

	CtrlNet				my_net,  	// control net
						my_refNet;  // refined net

	SplineC				my_splineC;	// for u and v curves

	std::string			my_s;
};

} // end of namespace subdivMesh

#endif
