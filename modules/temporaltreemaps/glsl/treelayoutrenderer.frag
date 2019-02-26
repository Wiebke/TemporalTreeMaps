/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, November 22, 2017 - 17:26:10
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include "utils/shading.glsl"

uniform vec4 diffuseLightColor;
uniform vec4 ambientLightColor;

const float eps = 0.0000001;

vec3 getCoefficients(vec3 xValues, vec3 yValues)
{
    float diffx1x2 = xValues.x - xValues.y; // x1 - x2
    float diffx1x3 = xValues.x - xValues.z; // x1 - x3
    float diffx2x3 = xValues.y - xValues.z; // x2 - x3

    bool noDiffx1x2 = abs(diffx1x2) < eps;
    bool noDiffx1x3 = abs(diffx1x3) < eps;
    bool noDiffx2x3 = abs(diffx2x3) < eps;

    // If any of these differences are approximately 0, the result will not be a parabola
    if (noDiffx1x2 && !noDiffx1x3 && !noDiffx2x3)
        {
            // x values for x1 and x2 are approx the same, but y values are not
            if (abs(yValues.x - yValues.y) > eps)
            {
                return vec3(0.0f);
            }
            // line passing through x1 and x3
            float m = (yValues.x - yValues.z) / diffx1x3;
            float b = (-xValues.x*yValues.z + xValues.z*yValues.x) / diffx1x3;
            return vec3(0.0f, m, b);
        }
        if (!noDiffx1x2 && noDiffx1x3 && !noDiffx2x3)
        {
            // x values are approx the same for x1 and x3, but y values are not
            if (abs(yValues.x - yValues.z) > eps)
            {
                return vec3(0.0f);
            }
            // line passing through x2 and x3
            float m = (yValues.y - yValues.z) / diffx2x3;
            float b = (-xValues.y*yValues.z + xValues.z*yValues.y) / diffx2x3;
            return vec3(0.0f, m, b);
        }
        if (!noDiffx1x2 && !noDiffx1x3 && noDiffx2x3)
        {
            // x values are approx the same for x2 and x3, but y values are not
            if (abs(yValues.y - yValues.z) > eps)
            {
                return vec3(0.0f);
            }
            // line passing through x1 and x3
            float m = (yValues.x - yValues.z) / diffx1x3;
            float b = (-xValues.x*yValues.z + xValues.z*yValues.x) / diffx1x3;
            return vec3(0.0f, m, b);
        }
        if (noDiffx1x2 && noDiffx1x3 && noDiffx2x3)
        {
            if (abs(yValues.x - yValues.y) > eps
                || abs(yValues.x - yValues.z) > eps
                || abs(yValues.y - yValues.z) > eps)
            {
                return vec3(0.0f);
            }
            return vec3(0.0f, 0.0f, yValues.x);
        }
    // (x - d)(x - e) = x^2 - (d + e)x + de
    float a = yValues.x / (diffx1x2 * diffx1x3) 
        + yValues.y / (-diffx1x2 * diffx2x3) 
        + yValues.z / (-diffx1x3 * -diffx2x3);
    float b = -(xValues.y + xValues.z) * yValues.x / (diffx1x2 * diffx1x3) + 
        -(xValues.x + xValues.z) * yValues.y / (-diffx1x2 * diffx2x3) +
        -(xValues.x + xValues.y) * yValues.z / (-diffx1x3 * -diffx2x3);
    float c = (xValues.y * xValues.z) * yValues.x / (diffx1x2 * diffx1x3) +
        (xValues.x * xValues.z) * yValues.y / (-diffx1x2 * diffx2x3) +
        (xValues.x * xValues.y) * yValues.z / (-diffx1x3 * -diffx2x3);
    return vec3(a, b, c);
}

vec3 getNormalAt(vec3 coefficients, float x){
        // outer normal (0.0, 1, -(b + 2ax))
        return vec3(0.0, -coefficients.y - 2 * coefficients.x * x , 1.0);
}

in float vertex_;
in vec3 texCoord_;
in vec4 color_;
in vec3 lightDir_;
in vec3 normal_;

void main() {
	vec3 coeff = vec3(0.0);
#ifdef NORMAL_AS_COEFF
    coeff = normal_;
#endif
#ifdef NORMAL_AND_TEXTURE_AS_COEFF
    coeff = getCoefficients(normal_, texCoord_);
#endif

vec3 actualNormal = getNormalAt(coeff, vertex_);

vec3 N = normalize(actualNormal);
vec3 L = normalize(lightDir_);

vec3 diffuseColor = max(dot(N, L),0) * color_.xyz * diffuseLightColor.xyz;
FragData0 = vec4(diffuseColor + ambientLightColor.xyz, 1.0);
}
