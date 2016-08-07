#ifndef OPENMESHEXT_H
#define OPENMESHEXT_H

#include <OpenMesh/Core/Mesh/Traits.hh>
#include <QVector3D>

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
    VertexAttributes(OpenMesh::Attributes::Status);

    EdgeAttributes(OpenMesh::Attributes::Status);

    FaceAttributes(OpenMesh::Attributes::Status);

    VertexTraits
    {
        private:
            float weight_ = 0.5;
            QVector3D color_;
        public:
            float weight()
			{
                return weight_;
            }
            void setWeight(float weight)
			{
                if(weight<0){
                    throw "Weight illegal value (must be positive)";
                }
                this->weight_ = weight;
            }

            QVector3D color()
			{
                return color_;
            }
            void setColor(QVector3D &color)
			{
                this->color_ = color;
            }
    };
};

}

#endif // OPENMESHEXT_H
