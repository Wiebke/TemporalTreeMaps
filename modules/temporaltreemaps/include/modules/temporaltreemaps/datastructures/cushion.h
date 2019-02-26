/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, December 06, 2017 - 11:09:39
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <modules/temporaltreemaps/temporaltreemapsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo
{
namespace kth
{

namespace cushion
{
    vec2 getNormalAt(const vec3& coefficients, const float x);

    /// Evaluate the parabola given by coefficients at point x 
    float evaluate(const vec3& coefficients, const float x);

    ///Compute the parabola given by three (x,y)-pairs and return coefficients (a,b,c)
    vec3 getCoefficients(const vec3& xValues, const vec3& yValues);

    ///Compute three point pairs for a parabola given by coefficients 
    ///(the left and right x value must be stored in the first and last coordinate of xValues)
    void getPoints(vec3& xValues, vec3& yValues, const vec3& coefficients);

    ///Compute the extremum value for a given parabola, if it is not a parabola, return
    ///the maximum point between left and right x value 
    ///which must be stored in the first and last coordinate of xValues)
    std::pair<float, float> getGlobalExtremum(const vec3& xValues, const vec3& coefficients);

    vec3 makeCushion(float xLeft, float xRight, const float baseHight, const float scaleFactor, const uint8_t depth);

    ///Span a parabola between two points
    ///(the left and right x value must be stored in the first and last coordinate of xValues)
    void makeCushion(vec3& xValues, vec3& yValues, const float baseHight, const float scaleFactor, const uint8_t depth);

}


} // namespace kth
} // namespace
