#ifndef TOSTRING_H
#define TOSTRING_H

#include <sstream>

namespace subdivMesh {

template <class T>
inline std::string to_string (const T& t)
{
std::stringstream ss;
ss << t;
return ss.str();
}

} // end of namespace subdivMesh

#endif

