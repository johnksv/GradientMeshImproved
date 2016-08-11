#ifndef GMUTILS_H
#define GMUTILS_H
#include <QPointF>

namespace GMView {

static inline vector<QPointF> subdCurve(const vector<QPointF>& spline, int steps = 3, bool closed = false)
{
    // recursive halting condition
    if(steps==0)
        return spline;

    // allocate new polygon on stack
    vector<QPointF> newVec;

    // SPECIAL CASES: open curve: start at end point; start at end point if closed and
    // number of vertices is below 4
    if(!closed||spline.size()<4) // size or size method
        newVec.push_back(spline.front());

    // SPECIAL CASE: deal with splines with less than 4 control points
    if(spline.size()==2) { // create a line (between the two points)
        newVec.push_back(0.667*spline.front()+0.333*spline.back());
        newVec.push_back(0.333*spline.front()+0.667*spline.back());
        newVec.push_back(spline.back());
        return subdCurve(newVec,steps-1, closed);
    }

    // three points
    if(spline.size()==3) {
        newVec.push_back(0.5*spline.front()+0.5*spline.at(1));
        newVec.push_back(0.125*spline.front()+0.75*spline.at(1) + 0.125*spline.back());
        newVec.push_back(0.5*spline.back()+0.5*spline.at(1));

        newVec.push_back(spline.back());
//        if (spline.at(1).isSharp)
//        {
//            newVec[newVec.size()/2] = spline.at(1);
//        }
        return subdCurve(newVec,steps-1, closed);
    }

    // Perform subdivision for the standard case.
    for(int i = 1;i<spline.size()-1;i++)
    {
        QPointF new1 = 0.5*spline.at(i-1)+0.5*spline.at(i);
        QPointF new2 = 0.125*spline.at(i-1)+0.75*spline.at(i)+0.125*spline.at(i+1);
//        if (spline.at(i).isSharp) // if control point is set to "sharp" (ignore if feature is not supported)
//        {
//            new2 = spline.at(i);
//        }
        newVec.push_back(new1);
        newVec.push_back(new2);
    }

    // add second last (open) or last (closed) control point
    newVec.push_back(0.5*spline.at(spline.size()-2)+0.5*spline.back());

    // if open, add last point
    if(!closed)
        newVec.push_back(spline.back());

    // recursively perform the next subdivision step
    return subdCurve(newVec,steps-1, closed);
}

} //End namespace GMView


#endif // GMUTILS_H
