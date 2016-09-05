#ifndef OPENMESHEXT_H
#define OPENMESHEXT_H

#include <OpenMesh/Core/Mesh/Traits.hh>
#include <QVector3D>
#include <QVector2D>

/*! Namespace for classes and data structures extending
 *  and customising the OpenMesh library
 */
namespace OpenMeshExt
{

/*! \brief Struct to customise data for mesh elements.
 *  Custom vertex data is most likely to be needed.
 */
struct CustomTraits : public OpenMesh::DefaultTraits
{
    // Add variables here...
	typedef OpenMesh::Vec3f Color;

    VertexAttributes(OpenMesh::Attributes::Status | OpenMesh::Attributes::Color);

    EdgeAttributes(OpenMesh::Attributes::Status);

    FaceAttributes(OpenMesh::Attributes::Status);

    HalfedgeTraits
    {
        private:
            QVector2D constraint_ = QVector2D(0.1,0.1);
        public:
        QVector2D constraint() const
        {
            return constraint_;
        }
        void setConstraint(QVector2D &constraint)
        {
            constraint_ = constraint;
        }
    };
};

}

#endif // OPENMESHEXT_H
