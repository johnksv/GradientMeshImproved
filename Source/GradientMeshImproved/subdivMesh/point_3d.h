#ifndef POINT_3D_H
#define POINT_3D_H

#include <iostream>
#include "types.h"

namespace subdivMesh {

class Point_3D
{
	friend std::ostream &operator<<(std::ostream &os, const Point_3D &point);
	friend std::istream &operator>>(std::istream &is, Point_3D &point);
	friend Point_3D operator*(const PointPrec &p, const Point_3D &point);

public:
    Point_3D(void);
	Point_3D(PointPrec x,
			 PointPrec y,
             PointPrec z);
	Point_3D(PointPrec xyz[3]);
    Point_3D(const Point_3D &orig);
	~Point_3D(void);
	PointPrec getX(void) const;
	PointPrec getY(void) const;
	PointPrec getZ(void) const;
    PointPrec* getCoords(void);
	void setX(PointPrec x);
	void setY(PointPrec y);
	void setZ(PointPrec z);
    const Point_3D operator+(const Point_3D &point) const;
    const Point_3D operator-(const Point_3D &point) const;
	const Point_3D operator*(const PointPrec &p) const;

    Point_3D operator+=(const Point_3D &point);
    Point_3D operator*=(const PointPrec &p);

	PointPrec	dist(Point_3D poi);

//    void gl(void) const;
	
private:
	PointPrec my_coords[3];
};

std::ostream &operator<<(std::ostream &os, const Point_3D &point);
std::istream &operator>>(std::istream &is, Point_3D &point);

Point_3D operator*(const PointPrec &p, const Point_3D &point);

} // end of namespace subdivMesh

#endif
