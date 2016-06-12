#include "point_3d.h"
#include <math.h>

using namespace std;
using namespace subdivMesh;

Point_3D::Point_3D(void)
{
	setX(0);
	setY(0);
	setZ(0);
}

Point_3D::Point_3D(PointPrec x,
                   PointPrec y,
                   PointPrec z)
{
	setX(x);
	setY(y);
	setZ(z);
}

Point_3D::Point_3D(PointPrec xyz[3])
{
	setX(xyz[0]);
	setY(xyz[1]);
	setZ(xyz[2]);
}

Point_3D::Point_3D(const Point_3D &orig)
{
	setX(orig.getX());
	setY(orig.getY());
	setZ(orig.getZ());
}

Point_3D::~Point_3D(void)
{
}

PointPrec Point_3D::getX(void) const
{
	return(my_coords[0]);
}

PointPrec Point_3D::getY(void) const
{
	return(my_coords[1]);
}

PointPrec Point_3D::getZ(void) const
{
	return(my_coords[2]);
}

PointPrec* Point_3D::getCoords(void)
{
    return(my_coords);
}

const Point_3D Point_3D::operator+(const Point_3D &point) const
{

	Point_3D temp;

    temp.setX(my_coords[0] + point.getX());
    temp.setY(my_coords[1] + point.getY());
    temp.setZ(my_coords[2] + point.getZ());
	return(temp);
}

const Point_3D Point_3D::operator-(const Point_3D &point) const
{

    Point_3D temp;

    temp.setX(my_coords[0] - point.getX());
    temp.setY(my_coords[1] - point.getY());
    temp.setZ(my_coords[2] - point.getZ());
    return(temp);
}

const Point_3D Point_3D::operator*(const PointPrec &p) const
{
	Point_3D temp;

    temp.setX(my_coords[0] * p);
    temp.setY(my_coords[1] * p);
    temp.setZ(my_coords[2] * p);
	return(temp);
}

Point_3D Point_3D::operator+=(const Point_3D &point)
{
    setX(my_coords[0] + point.getX());
    setY(my_coords[1] + point.getY());
    setZ(my_coords[2] + point.getZ());
    return *this;
}

Point_3D Point_3D::operator*=(const PointPrec &p)
{
    setX(my_coords[0] * p);
    setY(my_coords[1] * p);
    setZ(my_coords[2] * p);
    return *this;
}

void Point_3D::setX(PointPrec x)
{
	my_coords[0] = x;
}

void Point_3D::setY(PointPrec y)
{
	my_coords[1] = y;
}

void Point_3D::setZ(PointPrec z)
{
	my_coords[2] = z;
}

//void Point_3D::gl(void) const
//{
//    glVertex3f(getX(), getY(), getZ());
//}

namespace subdivMesh {

ostream &operator<<(ostream &os, const Point_3D &point)
{
    os << point.getX() << " " << point.getY() << " " << point.getZ();
	return os;
}

istream &operator>>(istream &is, Point_3D &point)
{
	is >> point.my_coords[0] >> point.my_coords[1] >> point.my_coords[2];
	return is;
}

Point_3D operator*(const PointPrec &p, const Point_3D &point)
{
	Point_3D temp;

	temp.setX(point.getX()*p);
	temp.setY(point.getY()*p);
	temp.setZ(point.getZ()*p);
	return(temp);
}

} // end of namespace subdivMesh

PointPrec Point_3D::dist(Point_3D poi)
{
	return(sqrt((getX() - poi.getX()) * (getX() - poi.getX()) + (getY() - poi.getY()) * (getY() - poi.getY()) + (getZ() - poi.getZ()) * (getZ() - poi.getZ()) ));
}
