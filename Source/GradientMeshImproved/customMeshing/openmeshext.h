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
            double weight_;
            QVector3D color_;
        public:
            double weight(){
                return weight_;
            }
            void setWeight(double weight_){
                if(weight_<0){
                    throw "Weight illegal value (must be positive)";
                }
                this->weight_ = weight_;
            }

            QVector3D color(){
                return color_;
            }
            void setColor(QVector3D color_){
                this->color_ = color_;
            }
    };

    EdgeTraits
    {

    };
};

}

#endif // OPENMESHEXT_H
