/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, December 06, 2017 - 12:14:16
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/datastructures/cushion.h>
#include <math.h>

namespace inviwo
{
namespace kth
{

namespace cushion
{

    vec2 getNormalAt(const vec3& coefficients, const float x)
    {
        // outer normal (-(b + 2ax), 1)
        return vec2(-coefficients.y - 2 * coefficients.x * x, 1.0f);
    }

    float evaluate(const vec3& coefficients, const float x)
    {
        // ax^2 + bx + c
        return coefficients.x * x *x + coefficients.y * x + coefficients.z;
    }

    vec3 getCoefficients(const vec3& xValues, const vec3& yValues)
    {
        // From lagragian interpolation:
        //          (x - x2)(x - x3)          (x - x1)(x - x2)          (x - x1)(x - x3)
        // y(x) = -------------------- y1 + -------------------- y2 + -------------------- y3
        //         (x1 - x2)(x1 - x3)        (x2 - x1)(x2 - x3)        (x3 - x1)(x3 - x2)
        // Here: 
        // x1: xValues.x, x2: xValues.y, x3: xValues.z
        // y1: yValues.x, y2: yValues.y, y3: yValues.z

        const float diffx1x2 = xValues.x - xValues.y; // x1 - x2
        const float diffx1x3 = xValues.x - xValues.z; // x1 - x3
        const float diffx2x3 = xValues.y - xValues.z; // x2 - x3

        // If any of these differences are approximately 0, the result will not be a parabola
        bool noDiffx1x2 = std::fabs(diffx1x2) < std::numeric_limits<float>::epsilon();
        bool noDiffx1x3 = std::fabs(diffx1x3) < std::numeric_limits<float>::epsilon();
        bool noDiffx2x3 = std::fabs(diffx2x3) < std::numeric_limits<float>::epsilon();
        // If only one difference is approx 0, the result is a line, e.g through (x1,y1) and (x2,y2)
        //         (x - x2)        (x - x1)         (y1 - y2)       (-x2*y1 + x1*y2) 
        // y(x) = ---------- y1 + ----------- y2 = ----------- x + ------------------
        //         (x1 - x2)       (x2 - x1)        (x1 - x2)          (x1 - x2)
        if (noDiffx1x2 && !noDiffx1x3 && !noDiffx2x3)
        {
            // x values for x1 and x2 are approx the same, but y values are not
            if (std::fabs(yValues.x - yValues.y) > std::numeric_limits<float>::epsilon())
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
            if (std::fabs(yValues.x - yValues.z) > std::numeric_limits<float>::epsilon())
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
            if (std::fabs(yValues.y - yValues.z) > std::numeric_limits<float>::epsilon())
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
            if (std::fabs(yValues.x - yValues.y) > std::numeric_limits<float>::epsilon()
                || std::fabs(yValues.x - yValues.z) > std::numeric_limits<float>::epsilon()
                || std::fabs(yValues.y - yValues.z) > std::numeric_limits<float>::epsilon())
            {
                return vec3(0.0f);
            }
            return vec3(0.0f, 0.0f, yValues.x);
        }

        // General case:
        // (x - d)(x - e) = x^2 - (d + e)x + de
        float a = yValues.x / (diffx1x2 * diffx1x3) 
            + yValues.y / (-diffx1x2 * diffx2x3) 
            + yValues.z / (-diffx1x3 * -diffx2x3);
        float b = -(xValues.y + xValues.z) * yValues.x / (diffx1x2 * diffx1x3) + 
            -(xValues.x + xValues.z) * yValues.y / (-diffx1x2 * diffx2x3) +
            -(xValues.x + xValues.y) * yValues.z / (-diffx1x3 * -diffx2x3);
        float c = (xValues.y * xValues.z) * yValues.x / (diffx1x2 * diffx1x3) +
            (xValues.x * xValues.z) * yValues.y / (-diffx1x2 * diffx2x3) +
            (xValues.x * xValues.y) * yValues.z / (-diffx1x3 * -diffx2x3);;
        return vec3(a, b, c);
    }

    void getPoints(vec3 & xValues, vec3 & yValues, const vec3 & coefficients)
    {
        xValues.x -= 2.0;
        yValues.x = evaluate(coefficients, xValues.x);

        xValues.z += 2.0;
        yValues.z = evaluate(coefficients, xValues.z);
        
        const auto extremum = getGlobalExtremum(xValues, coefficients);
        xValues.y = extremum.first;
        yValues.y = extremum.second;
    }

    std::pair<float, float> getGlobalExtremum(const vec3& xValues, const vec3& coefficients)
    {
        // f(x) = ax^2 + bx = c => f'(x) = 2ax + b 
        // f'(x) = 0 => x = -b / 2a

        // Not a parabola
        if (std::fabs(coefficients.x) < std::numeric_limits<float>::epsilon())
        {
            // Line with slope 0 -> Take the middle x value
            if (std::fabs(coefficients.y) < std::numeric_limits<float>::epsilon())
            {
                float xMiddle = (xValues.x + xValues.z) / 2.0f;
                return std::make_pair(xMiddle, evaluate(coefficients, xMiddle));
            }
            // Negative slope, smaller value will be first
            if (coefficients.y < 0.0f)
            {
                return std::make_pair(xValues.x, evaluate(coefficients, xValues.x));
            }
            return std::make_pair(xValues.z, evaluate(coefficients, xValues.z));
        }
        float x = -coefficients.y / (2 * coefficients.x);
        return std::make_pair(x, evaluate(coefficients, x));
    }

    vec3 makeCushion(float xLeft, float xRight, const float baseHight, const float scaleFactor, const uint8_t depth)
    {
        // h_d = f^d * h, y(x) = 4h_d / (xL - xR) (x - xL)(xR - x)
        // y(x) = -Fx^2 + F(xL + xR)x - FxRxL
        if (std::fabs(xLeft - xRight) < std::numeric_limits<float>::epsilon())
        {
            //LogInfo("Left and right value are equal.");
            return vec3(0.0f);
            //xLeft -= 0.1f;
            //xRight += 0.1f;
        }
        float F = 4.f * baseHight * powf(scaleFactor, depth) / (xRight - xLeft);
        return vec3(-F, F * (xLeft + xRight), F * (xLeft * xRight));
    }

    void makeCushion(vec3& xValues, vec3& yValues, const float baseHight, const float scaleFactor, const uint8_t depth)
    {
        const vec3 coefficients = makeCushion(xValues.x, xValues.z, baseHight, scaleFactor, depth);
        getPoints(xValues, yValues, coefficients);
    }

}

} // namespace kth
} // namespace

