#ifndef TYPES_H
#define TYPES_H

#include <vector>

namespace subdivMesh {

typedef float PointPrec;
typedef std::vector<PointPrec> KnotVec;
typedef std::vector<std::vector<PointPrec> > KnotNet;
typedef std::vector<unsigned int> Mults;

} // end of namespace subdivMesh

#endif
