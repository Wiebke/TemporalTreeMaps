/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, November 22, 2017 - 17:26:10
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include "utils/structs.glsl"

uniform GeometryParameters geometry_;
uniform mat4 projectionMatrix;
uniform vec3 lightPos;

out vec4 color_;
out vec3 texCoord_;
out vec3 lightDir_;
out vec3 normal_;
out float vertex_;
 
void main() {
    color_ = in_Color;
    texCoord_ = in_TexCoord;
    lightDir_ = lightPos;
    normal_ = in_Normal;
    vertex_ = in_Vertex.y;
    gl_Position = projectionMatrix * in_Vertex; 
}