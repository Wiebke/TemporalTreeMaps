/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Köpp
 *  Init    : Monday, October 09, 2017 - 14:47:18
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <modules/temporaltreemaps/temporaltreemapsmoduledefine.h>
#include <modules/temporaltreemaps/datastructures/tree.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>

namespace inviwo 
{
namespace kth
{

    using TemporalTreeInport = DataInport<TemporalTree>;
    using TemporalTreeOutport = DataOutport<TemporalTree>;

} // namespace kth
}