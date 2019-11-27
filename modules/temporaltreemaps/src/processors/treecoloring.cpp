/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Köpp
 *  Init    : Thursday, November 30, 2017 - 16:22:24
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <inviwo/core/util/utilities.h>
#include <modules/temporaltreemaps/processors/treecoloring.h>
#include <modules/temporaltreemaps/datastructures/treecolor.h>
#include <inviwo/core/util/colorconversion.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeColoring::processorInfo_
{
    "org.inviwo.TemporalTreeColoring",      // Class identifier
    "Tree Coloring",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeColoring::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeColoring::TemporalTreeColoring()
    :Processor()
    , portInTree("inTree")
    , portOutTree("outTree")
    // Bands
    , propColorScheme("colorScheme", "Color Scheme")
	, propValueTranserFunc("valueTransferFunc", "Coloring Range")
    , propColorUniform("colorUniform", "Uniform Color", vec4(0.5f, 0.5f, 0.5f, 1.0f),
        vec4(0.0f), vec4(1.0f), vec4(0.1f), InvalidationLevel::InvalidOutput, PropertySemantics::Color)
    , propColorSeed("colorSeed", "Random Seed", 0, 0, RAND_MAX + 1, 1)
    , propColorBrewerScheme("colorBrewerScheme", "Color Brewer Scheme")
    , propAlternate("alternate", "Alternate", true)
    , propColorSpace("colorSpace","Color Space")
    , propColorFrom("colorFromDepth", "From Depth", -1, 0)
    , propColorTo("colorToDepth", "Until Depth", -1, -1)
    , propColorSpaceMap("colorSpaceMap", "Color Map")
    , propHSVOffset("hsvOffset", "HSV Offset", 0.0f, -1.0f, 1.0f)
    , propRangeDecay("rangeDecay", "Range Decay", 0.99f, 0.1f, 1.0f, 0.01f)
    , propSaturationMin("saturationMin", "Sat. Min", 0, 0, 1)
    , propSaturationMax("saturationMax", "Sat. Max", 1, 0, 1)
{
    // Ports
    addPort(portInTree);
    addPort(portOutTree);
    
    // Properties
    addProperty(propColorScheme);
    propColorScheme.addOption("alreadySet", "Aready Set", 0);
    propColorScheme.addOption("uniform", "Uniform Color", 1);
    propColorScheme.addOption("transfer", "Transferfunction", 2);
    propColorScheme.addOption("randomPerLeaf", "Random Color", 3);
    propColorScheme.addOption("colorBrewer", "Color Brewer Scheme", 4);
    propColorScheme.addOption("hierchicalColor", "Hierarchical Color", 5);
    propColorScheme.setSelectedIndex(1);
    propColorScheme.setCurrentStateAsDefault();

    addProperty(propColorUniform);
    addProperty(propValueTranserFunc);
    addProperty(propColorSpace);
    addProperty(propColorBrewerScheme);
    addProperty(propColorSeed);
    addProperty(propColorSpaceMap);
    addProperty(propHSVOffset);
    addProperty(propAlternate);
    addProperty(propSaturationMin);
    addProperty(propSaturationMax);
    addProperty(propColorFrom);
    addProperty(propColorTo);
    addProperty(propRangeDecay);

    propColorBrewerScheme.addOption("accent", "Accent [3,8]", colorbrewer::Family::Accent);
    propColorBrewerScheme.addOption("dark2", "Dark2 [3,8]", colorbrewer::Family::Dark2);
    propColorBrewerScheme.addOption("pastel1", "Pastel1 [3,9]", colorbrewer::Family::Pastel1);
    propColorBrewerScheme.addOption("pastel2", "Pastel2 [3,8]", colorbrewer::Family::Pastel2);
    propColorBrewerScheme.addOption("set1", "Set1 [3,9]", colorbrewer::Family::Set1);
    propColorBrewerScheme.addOption("set2", "Set2 [3,8]", colorbrewer::Family::Set2);
    propColorBrewerScheme.addOption("set3", "Set3 [3,12]", colorbrewer::Family::Set3);

    propColorSpace.addOption("sampleHSV", "HSV", 0);
    propColorSpace.addOption("sampleTransfer", "Color Map", 1);
    propColorSpace.addOption("sampleColorBrewer", "ColorBrewer", 2);
    propColorSpace.setSelectedIndex(0);
    propColorScheme.setCurrentStateAsDefault();

    propColorSpace.onChange([&]()
    {
        if (propColorSpace.get() == 0)
        {
            util::hide(propColorSpaceMap, propColorBrewerScheme, propColorSeed, propSaturationMin);
            util::show(propHSVOffset, propAlternate, propSaturationMax);
        } else if (propColorSpace.get() == 1)
        {
            util::hide(propHSVOffset, propColorBrewerScheme, propColorSeed, propSaturationMin, propSaturationMax);
            util::show(propColorSpaceMap, propAlternate);
        } else
        {
            util::hide(propColorSpaceMap, propAlternate);
            util::show(propHSVOffset, propColorBrewerScheme, propColorSeed, propSaturationMin, propSaturationMax);
        }
    });

    util::show(propColorUniform);
    
    util::hide(propColorSeed, propValueTranserFunc, propColorBrewerScheme,
            propAlternate, propColorTo, propColorFrom, propColorSpace, propColorSpaceMap, propHSVOffset, propRangeDecay,
            propSaturationMin, propSaturationMax);

    propColorScheme.onChange([&]()
    {
        switch (propColorScheme.get())
        {
        case 0:
            util::hide(propColorUniform, propColorSeed, propValueTranserFunc, propColorBrewerScheme,
                propAlternate, propColorTo, propColorFrom, propColorSpace, propColorSpaceMap, propHSVOffset, propRangeDecay);
            break;
        case 1:
            util::hide(propColorSeed, propValueTranserFunc, propColorBrewerScheme,
                propAlternate, propColorTo, propColorFrom, propColorSpace, propColorSpaceMap, propHSVOffset, propRangeDecay);
            util::show(propColorUniform);
            break;
        case 2:
            util::hide(propColorUniform, propColorSeed, propColorBrewerScheme,
                propAlternate, propColorTo, propColorFrom, propColorSpace, propColorSpaceMap, propHSVOffset, propRangeDecay);
            util::show(propValueTranserFunc);
            break;
        case 3:
            util::hide(propColorUniform, propValueTranserFunc, propColorBrewerScheme,
                propAlternate, propColorTo, propColorFrom, propColorSpace, propColorSpaceMap, propHSVOffset, propRangeDecay);
            util::show(propColorSeed);
            break;
        case 4:
            util::hide(propColorUniform, propValueTranserFunc, 
                propAlternate, propColorTo, propColorFrom, propColorSpace, propColorSpaceMap, propHSVOffset, propRangeDecay);
            util::show(propColorBrewerScheme);
            break;
        case 5:
            util::hide(propColorUniform, propColorSeed, propValueTranserFunc, propColorBrewerScheme);
            util::show(propAlternate, propColorTo, propColorFrom, propColorSpace, propRangeDecay);
            if (propColorSpace.get() == 0)
            {
                util::hide(propColorSpaceMap, propColorBrewerScheme, propColorSeed, propSaturationMin, propSaturationMax);
                util::show(propHSVOffset);
            } if (propColorSpace.get() == 1)
            {
                util::hide(propHSVOffset, propColorBrewerScheme, propColorSeed, propSaturationMin, propSaturationMax);
                util::show(propColorSpaceMap);
            }
            else
            {
                util::hide(propColorSpaceMap, propSaturationMin, propSaturationMax);
                util::show(propColorBrewerScheme, propColorSeed, propHSVOffset);
            }
            break;
        default:
            util::hide(propColorUniform, propColorSeed, propValueTranserFunc, propColorBrewerScheme,
                propAlternate, propColorTo, propColorFrom, propColorSpace, propColorSpaceMap, propHSVOffset, propRangeDecay);
            break;
        }
        
    });

}


void TemporalTreeColoring::process()
{
    // Get tree
    std::shared_ptr<const TemporalTree> pInTree = portInTree.getData();
    if (!pInTree) return;

    std::shared_ptr<TemporalTree> pOutTree = std::make_shared<TemporalTree>(TemporalTree(*pInTree));

    auto leaves = pOutTree->getLeaves();

    std::vector<dvec4> colorMap;

    int levels = int(pOutTree->getNumLevels(0));

    propColorTo.setMaxValue(levels);
    propColorFrom.setMaxValue(levels);

    if (propColorScheme.get() == 4)
    {
        auto numLeaves = leaves.size();
        if (leaves.size() > 12)
        {
            LogProcessorError("Color Brewer Schemes only work for trees with a " <<
                "number of leaves between 3 and 12, not " << numLeaves << ".");
            return;
        }
        if (colorbrewer::getMinNumberOfColorsForFamily(propColorBrewerScheme) > numLeaves ||
            colorbrewer::getMaxNumberOfColorsForFamily(propColorBrewerScheme) < numLeaves)
        {
            LogProcessorError("The Color Brewer Family " << 
                propColorBrewerScheme.getSelectedDisplayName() <<
                " does not work for number of leaves: " << numLeaves << ".");
            return;
        }
        colorMap = colorbrewer::getColormap(propColorBrewerScheme, (glm::uint8) leaves.size());
        if (propColorSeed != 0)
        {
            randomGen.seed(static_cast<std::mt19937::result_type>(propColorSeed.get()));
            std::shuffle(colorMap.begin(), colorMap.end(), randomGen);
        }
    }

    if (propColorScheme.get() == 5)
    {
        std::function<vec3(float)> sampleColor;
        
        if (propColorSpace == 0)
        {
            sampleColor = [&](float x)->vec3 {
                x = x + propHSVOffset;
                while (x > 1.0) x -= 1.0;
                while (x < 0.0) x += 1.0;
                return color::hsv2rgb(vec3(x, propSaturationMax, 1.0)); };
        }
        else if (propColorSpace == 2)
        {
            auto firstLevel = pInTree->getHierarchicalChildren(0);
            std::map<size_t, size_t> prepostPairs;
            // Find pre post pairs in the name, if a pre is found in the string, look for the post
            size_t inFirstLevelIndexPre = 0;
            for (auto nodeIndex : firstLevel)
            {
                size_t found = pInTree->nodes[nodeIndex].name.find(" (pre");
                if (found != std::string::npos)
                {
                    // Found a pre
                    std::string name = pInTree->nodes[nodeIndex].name.substr(0, found);
                    LogInfo(name);
                    size_t inFirstLevelIndexPost = 0;
                    for (auto nodeIndexPost : firstLevel)
                    {
                        if (nodeIndex != nodeIndexPost)
                        {
                            size_t foundPost = pInTree->nodes[nodeIndexPost].name.find(name);
                            if (foundPost != std::string::npos)
                            {
                                if (inFirstLevelIndexPre < inFirstLevelIndexPost)
                                {
                                    prepostPairs[inFirstLevelIndexPre] = inFirstLevelIndexPost;
                                } else
                                {
                                    prepostPairs[inFirstLevelIndexPost] = inFirstLevelIndexPre;
                                }
                            }
                        }
                        inFirstLevelIndexPost++;
                    }
                }
                inFirstLevelIndexPre++;
            }
            
            size_t numFirstLevel = firstLevel.size() - prepostPairs.size();

            if (colorbrewer::getMinNumberOfColorsForFamily(propColorBrewerScheme) > numFirstLevel ||
                colorbrewer::getMaxNumberOfColorsForFamily(propColorBrewerScheme) < numFirstLevel)
            {
                LogProcessorError("The Color Brewer Family " <<
                    propColorBrewerScheme.getSelectedDisplayName() <<
                    " does not work for nodes in first level: " << numFirstLevel << ".");
                return;
            }
            auto colorBrewerMap = colorbrewer::getColormap(propColorBrewerScheme, (glm::uint8) numFirstLevel);
            std::reverse(colorBrewerMap.begin(), colorBrewerMap.end());

            if (propColorSeed != 0)
            {
                randomGen.seed(static_cast<std::mt19937::result_type>(propColorSeed.get()));
                std::shuffle(colorBrewerMap.begin(), colorBrewerMap.end(), randomGen);
            }

            colorMap = std::vector<dvec4>(firstLevel.size());
            
            size_t inColorIndex = 0;
            std::set<size_t> alreadyProcessed;
            for (size_t inFirstLevelIndex = 0; inFirstLevelIndex < firstLevel.size(); inFirstLevelIndex++)
            {
                if (alreadyProcessed.find(inFirstLevelIndex) != alreadyProcessed.end())
                {
                    continue;
                }

                // If we encounter a pre-post-pair, we cannot have processed the counterpart already
                // We have inserted the minimum of the two, and they are not in already processed
                if (prepostPairs.count(inFirstLevelIndex))
                {
                    // Both the pre and post get the color for the current Index
                    colorMap[prepostPairs[inFirstLevelIndex]] = colorBrewerMap[inColorIndex];
                    alreadyProcessed.insert(prepostPairs[inFirstLevelIndex]);
                }
                colorMap[inFirstLevelIndex] = colorBrewerMap[inColorIndex];
                inColorIndex++;
            }
            numFirstLevel = firstLevel.size();



            // Color the first level by a colorbrewer scheme
            sampleColor = [this, &numFirstLevel, &colorMap](float x)->vec3
            {
                if (std::fabs(x-1.0) < std::numeric_limits<float>::epsilon())
                {
                    x -= std::numeric_limits<float>::epsilon();
                }
                int colorIndex = int(floor(x * (numFirstLevel)));
                // baseColor.y is saturation
                vec3 baseColor = color::rgb2hsv(colorMap[colorIndex]);
                 
                float s = propSaturationMin * (float(colorIndex + 1) / numFirstLevel - x) / (1.0f / numFirstLevel)
                    + propSaturationMax * (x - float(colorIndex) / numFirstLevel) / (1.0f / numFirstLevel);
                // Max and Min are fraction of the base Color here
                s *= baseColor.y;
                return color::hsv2rgb(vec3(baseColor.x, s, baseColor.z));
            };
        }
        else
        {
            sampleColor = [this](float x)->vec3 { return propColorSpaceMap.get().sample(x); };
        }

        // TODO: Hacky
        treecolor::traverseToLeavesForColor(*pOutTree, 
            0, // nodeIndex
            0, 1, // range
            propColorSpace != 2 ? propAlternate : false,
            pOutTree->nodes[0].startTime(), pOutTree->nodes[0].endTime(), 
            0, // depth
            propRangeDecay,
            propColorFrom, propColorTo, sampleColor);
        portOutTree.setData(pOutTree);
        return;
    }

    size_t leafCounter(0);

    for (auto leaf: leaves)
    {
        TemporalTree::TNode& leafNode = pOutTree->nodes[leaf];
        TemporalTree::TColorPerTimeMap& colors = leafNode.colors;
        colors.clear();
        
        const auto& lowerLimitLeaf = leafNode.lowerLimit;
        const auto& upperLimitLeaf = leafNode.upperLimit;

        for (auto itLowerLimit = lowerLimitLeaf.begin(), itUpperLimit = upperLimitLeaf.begin();
            itLowerLimit != lowerLimitLeaf.end() && itUpperLimit != upperLimitLeaf.end(); itLowerLimit++, itUpperLimit++)
        {
            switch (propColorScheme.get())
            {
            case 0:
                // There already exists a color, we only need to expand and handle fading
                colors.emplace_hint(colors.end(), itLowerLimit->first, vec4(leafNode.color,1.0f));
                break;
            case 1:
                colors.emplace_hint(colors.end(), itLowerLimit->first, propColorUniform.get());
                break;
            case 2:
                colors.emplace_hint(colors.end(), itLowerLimit->first, 
                    propValueTranserFunc.get().sample(itLowerLimit == lowerLimitLeaf.begin() ?
                        itUpperLimit->second.second - itLowerLimit->second.second :
                        itUpperLimit->second.first - itLowerLimit->second.first));
                break;
            case 3:
                // Make sure colors stay the same even if the order changes 
                // by setting the seed based on the index (which does not change regardless of order)
                randomGen.seed(static_cast<std::mt19937::result_type>(leaf + propColorSeed.get()));
                colors.emplace_hint(colors.end(), itLowerLimit->first, 
                    vec4(rand(0.0f, 1.0f), rand(0.0f, 1.0f), rand(0.0f, 1.0f), 1.0f));
                break;
            case 4:
                colors.emplace_hint(colors.end(), itLowerLimit->first,
                    colorMap[leafCounter]);
            default:
                colors.emplace_hint(colors.end(), itLowerLimit->first,
                    vec4(0.0f, 0.0f, 0.0f, 1.0f));
                break;
            }
        }

        leafCounter++;

    }

    portOutTree.setData(pOutTree);
}

float TemporalTreeColoring::rand(const float min, const float max) const
{
    return min + randomDis(randomGen) * (max - min);
}

} // namespace kth
} // namespace

