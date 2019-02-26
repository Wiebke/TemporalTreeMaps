#include <modules/tools/setminmaxdatamap.h>
#include <inviwo/core/util/formats.h>


namespace inviwo
{
namespace kth
{


namespace
{
template< typename T >
void SetMinMax(const char* pData, const size_t NumOfBytes, std::shared_ptr<inviwo::Volume> volume)
{
    auto minmax = std::minmax_element(reinterpret_cast<const T*>(pData),
                                      reinterpret_cast<const T*>(pData + NumOfBytes));
    volume->dataMap_.dataRange = dvec2(*minmax.first, *minmax.second);
    volume->dataMap_.valueRange = dvec2(*minmax.first, *minmax.second);
}
};

void SetMinMaxForInviwoDataMap(const char* pData, const size_t NumOfBytes, std::shared_ptr<inviwo::Volume> pVolume)
{
    const auto Format = pVolume->getDataFormat();
    const auto Precision = Format->getPrecision();

    switch (Format->getNumericType())
    {
        case inviwo::NumericType::Float:
        {
            switch (Precision)
            {
                case 32: SetMinMax<float>(pData, NumOfBytes, pVolume); break;
                case 64: SetMinMax<double>(pData, NumOfBytes, pVolume); break;
                default: break;
            }
            break;
        }

        case inviwo::NumericType::SignedInteger:
        {
            switch (Precision)
            {
                case 16: SetMinMax<DataInt16::type>(pData, NumOfBytes, pVolume); break;
                case 32: SetMinMax<DataInt32::type>(pData, NumOfBytes, pVolume); break;
                case 64: SetMinMax<DataInt64::type>(pData, NumOfBytes, pVolume); break;
                default: break;
            }
            break;
        }

        case inviwo::NumericType::UnsignedInteger:
        {
            switch (Precision)
            {
                case 16: SetMinMax<DataUInt16::type>(pData, NumOfBytes, pVolume); break;
                case 32: SetMinMax<DataUInt32::type>(pData, NumOfBytes, pVolume); break;
                case 64: SetMinMax<DataUInt64::type>(pData, NumOfBytes, pVolume); break;
                default: break;
            }
            break;
        }

        case inviwo::NumericType::NotSpecialized:
        default:
            break;
    }
}
} // namespace kth
};
