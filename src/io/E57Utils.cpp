#include "io/E57Utils.h"

#include <sstream>

bool e57Utils::check_tl_result(TlResult result, std::string& msg)
{
    switch (result)
    {
    case TL_RESULT_OK:
        return true;
    case TL_CONVERSION_NO_ROTATION:
        msg += "E57_reader: Warning: No rotation detected in the global transformation\n";
        return true;
    case TL_CONVERSION_NO_TRANSLATION:
        msg += "E57_reader: Warning: No translation detected in the global transformation\n";
        return true;
    case TL_CONVERSION_NO_ROTATION_AND_TRANSLATION:
        msg += "E57_reader: Warning: No rotation and translation detected in the global transformation\n";
        return true;
    case TL_ERROR:
        msg += "E57_reader: An error has occured.\n";
        return false;
    case TL_ERROR_INVALID_FORMAT:
        msg += "E57 Reader: Error, invalid format in e57 file\n";
        return false;
    case TL_ERROR_FORMAT_NOT_SUPPORTED:
        msg += "E57 Reader: Error, we encountered a format not supported\n";
        return false;
    default:
        msg += "TlResult: wrong error code.\n";
        return false;
    }
}


TlResult e57Utils::detectFormat(e57::StructureNode& _dataSet, E57AttribFormat& _format)
{
    e57::StructureNode proto(e57::CompressedVectorNode(_dataSet.get("points")).prototype());

    // Determine which format is used in the compressed vector
    if (proto.isDefined("cartesianX") && proto.isDefined("cartesianY") && proto.isDefined("cartesianZ"))
    {
        // Check that the 3 coordinates have the same type
        if (proto.get("cartesianX").type() != proto.get("cartesianY").type() ||
            proto.get("cartesianX").type() != proto.get("cartesianZ").type())
        {
            return TL_ERROR_INVALID_FORMAT;
        }

        if (proto.get("cartesianX").type() == e57::E57_FLOAT) {
            _format.coordinates = TL_COORD_CARTESIAN_FLOAT;
        }
        else if (proto.get("cartesianX").type() == e57::E57_SCALED_INTEGER) {
            _format.coordinates = TL_COORD_CARTESIAN_SCALED_INTEGER;
        }
        else {
            return TL_ERROR_FORMAT_NOT_SUPPORTED;
        }
    }
    else if (proto.isDefined("sphericalRange") && proto.isDefined("sphericalAzimuth") && proto.isDefined("sphericalElevation"))
    {
        _format.coordinates = TL_COORD_SPHERICAL_FLOAT;
    }
    else
    {
        return TL_ERROR_FORMAT_NOT_SUPPORTED;
    }


    // Determine the encoding of the color 
    if (proto.isDefined("colorRed") && proto.isDefined("colorGreen") && proto.isDefined("colorBlue"))
    {
        // Check that the 3 components have the same type
        if (proto.get("colorRed").type() != proto.get("colorGreen").type() ||
            proto.get("colorRed").type() != proto.get("colorBlue").type())
        {
            return TL_ERROR_INVALID_FORMAT;
        }

        // The only supported type for the color is Integer
        auto colorType = proto.get("colorRed").type();
        if (colorType == e57::E57_INTEGER) {
            _format.color = TL_RGB_UINT8_FORMAT;
        } else if (colorType == e57::E57_FLOAT) {
            _format.color = TL_RGB_FLOAT_FORMAT;
        }
        else {
            return TL_ERROR_FORMAT_NOT_SUPPORTED;
        }
    }
    else {
        _format.color = TL_RGB_NONE;
    }

    // Determine the encoding of the intensity
    if (proto.isDefined("intensity"))
    {
        switch (proto.get("intensity").type())
        {
        case (e57::E57_FLOAT):
        {
            _format.intensity = TL_I_FLOAT_FORMAT;
            break;
        }
        case (e57::E57_SCALED_INTEGER):
        {
            _format.intensity = TL_I_SCALED_INTEGER_FORMAT;
            break;
        }
        case (e57::E57_INTEGER):
        {
            _format.intensity = TL_I_UINT8_FORMAT;
            break;
        }
        default:
            return TL_ERROR_FORMAT_NOT_SUPPORTED;
        }
    }
    else {
        _format.intensity = TL_I_NONE;
    }

    // Determine if there is a component "invalidState"
    if (proto.isDefined("cartesianInvalidState")) {
        _format.state = TL_STATE_PRESENT;
    }
    else {
        _format.state = TL_STATE_NONE;
    }

    return TL_RESULT_OK;
}


TlResult e57Utils::initDestBuffers(e57::ImageFile _imf, e57::StructureNode& _dataSet, std::vector<e57::SourceDestBuffer>& _destBuffers, StagingBuffers& _stagingBuffers, const E57AttribFormat& _format, Limits& _limits)
{
    e57::StructureNode proto(e57::CompressedVectorNode(_dataSet.get("points")).prototype());

    // Determine which format is used in the compressed vector
    if (_format.coordinates == TL_COORD_CARTESIAN_FLOAT)
    {
        // by default doConversion = false, doScaling = false
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "cartesianX", _stagingBuffers.cartesianX, _stagingBuffers.size));
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "cartesianY", _stagingBuffers.cartesianY, _stagingBuffers.size));
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "cartesianZ", _stagingBuffers.cartesianZ, _stagingBuffers.size));
    }
    else if (_format.coordinates == TL_COORD_CARTESIAN_SCALED_INTEGER)
    {
        // for scaled integer specify that we want the conversion & scaling done
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "cartesianX", _stagingBuffers.cartesianX, _stagingBuffers.size, true, true));
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "cartesianY", _stagingBuffers.cartesianY, _stagingBuffers.size, true, true));
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "cartesianZ", _stagingBuffers.cartesianZ, _stagingBuffers.size, true, true));
    }
    else if (_format.coordinates == TL_COORD_SPHERICAL_FLOAT)
    {
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "sphericalRange", _stagingBuffers.sphericalRange, _stagingBuffers.size, true, true));
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "sphericalAzimuth", _stagingBuffers.sphericalAzimuth, _stagingBuffers.size, true, true));
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "sphericalElevation", _stagingBuffers.sphericalElevation, _stagingBuffers.size, true, true));
    }
    else
    {
        return TL_ERROR_FORMAT_NOT_SUPPORTED;
    }


    // Determine the encoding of the color 
    if (_format.color == TL_RGB_UINT8_FORMAT)
    {
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "colorRed", _stagingBuffers.colorRed, _stagingBuffers.size));
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "colorGreen", _stagingBuffers.colorGreen, _stagingBuffers.size));
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "colorBlue", _stagingBuffers.colorBlue, _stagingBuffers.size));
    }
    else if(_format.color == TL_RGB_FLOAT_FORMAT)
    {
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "colorRed", _stagingBuffers.colorRed, _stagingBuffers.size, true));
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "colorGreen", _stagingBuffers.colorGreen, _stagingBuffers.size, true));
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "colorBlue", _stagingBuffers.colorBlue, _stagingBuffers.size, true));
    }

    // Determine the encoding of the intensity
    if (_format.intensity == TL_I_FLOAT_FORMAT)
    {
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "intensity", _stagingBuffers.fIntensity, _stagingBuffers.size));

        // NOTE - The "intensityLimits" field is optional
        if (_dataSet.isDefined("intensityLimits")) {
            _limits.iMin = e57::FloatNode(_dataSet.get("intensityLimits/intensityMinimum")).value();
            _limits.iMax = e57::FloatNode(_dataSet.get("intensityLimits/intensityMaximum")).value();
        }
    }
    else if (_format.intensity == TL_I_SCALED_INTEGER_FORMAT)
    {
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "intensity", _stagingBuffers.fIntensity, _stagingBuffers.size, true, true));

        _limits.iMin = 0.0;
        _limits.iMax = 1.0;
    }
    else if (_format.intensity == TL_I_UINT8_FORMAT)
    {
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "intensity", _stagingBuffers.uIntensity, _stagingBuffers.size, false, false));

        if (_dataSet.isDefined("intensityLimits")) {
            _limits.iMin = (double)e57::IntegerNode(_dataSet.get("intensityLimits/intensityMinimum")).value();
            _limits.iMax = (double)e57::IntegerNode(_dataSet.get("intensityLimits/intensityMaximum")).value();
        }

        if (_limits.iMax > 255)
            // NOTE - If this happen, we must use a type with more precision than uint8
            return TL_ERROR_FORMAT_NOT_SUPPORTED;

    }

    // Determine if there is a component "invalidState"
    if (_format.state == TL_STATE_PRESENT)
    {
        // Let by default doConversion = false & doScaling = false
        _destBuffers.push_back(e57::SourceDestBuffer(_imf, "cartesianInvalidState", _stagingBuffers.state, _stagingBuffers.size));
    }

    return TL_RESULT_OK;
}


TlResult e57Utils::getRigidBodyTransform(e57::StructureNode _dataSet, double translation[3], double quaternion[4])
{
    bool rotationFound = false;
    bool translationFound = false;

    // NOTE(robin) The ASTM standard says that the "rotation" field is required, but some software
    //             do not include it (i.e. Autodesk ReCap)
    if (_dataSet.isDefined("pose/rotation")) {
        // STANDARD(ASTM e57) Quaternion shall be represented using double precision Float type
        quaternion[0] = e57::FloatNode(_dataSet.get("pose/rotation/x")).value();
        quaternion[1] = e57::FloatNode(_dataSet.get("pose/rotation/y")).value();
        quaternion[2] = e57::FloatNode(_dataSet.get("pose/rotation/z")).value();
        quaternion[3] = e57::FloatNode(_dataSet.get("pose/rotation/w")).value();

        rotationFound = true;
    }
    else
    {
        memset(quaternion, 0, 4 * sizeof(double));
        quaternion[3] = 1.0;
    }

    if (_dataSet.isDefined("pose/translation")) {
        // STANDARD(ASTM e57) A translation shall be represented using double precision Float type
        translation[0] = e57::FloatNode(_dataSet.get("pose/translation/x")).value();
        translation[1] = e57::FloatNode(_dataSet.get("pose/translation/y")).value();
        translation[2] = e57::FloatNode(_dataSet.get("pose/translation/z")).value();

        translationFound = true;
    }
    else
    {
        memset(translation, 0, 3 * sizeof(double));
    }

    // Retrun the result code
    if (rotationFound)
        if (translationFound)  return TL_RESULT_OK;
        else                   return TL_CONVERSION_NO_TRANSLATION;
    else
        if (translationFound)  return TL_CONVERSION_NO_ROTATION;
        else                   return TL_CONVERSION_NO_ROTATION_AND_TRANSLATION;
}


bool e57Utils::printFileStructure(e57::ImageFile imf, std::string& outText)
{
    std::stringbuf buffer;
    std::ostream ostr(&buffer);

    try {
        e57::StructureNode root = imf.root();

        int64_t childCount = root.childCount();
        ostr << "Root--E57_STRUCTURE (" << childCount << " childs)";
        // Start the recursive print of the data tree
        for (int i = 0; i < childCount; i++) {
            e57::Node child = root.get(i);
            printNode(child, 0, ostr);
        }

        outText = buffer.str();
    }
    catch (e57::E57Exception& ex) {
        ex.report(__FILE__, __LINE__, __FUNCTION__, ostr);
        outText = buffer.str();
        return false;
    }
    catch (std::exception& ex) {
        ostr << "\nERROR: Got an std::excetion, what = " << ex.what() << std::endl;
        outText = buffer.str();
        return false;
    }
    catch (...) {
        ostr << "\nERROR: Got an unknown exception" << std::endl;
        outText = buffer.str();
        return false;
    }

    return true;
}


void e57Utils::printNode(e57::Node& _childNode, uint32_t _depth, std::ostream& _os)
{
    switch (_childNode.type())
    {
    case e57::E57_STRUCTURE:
    {
        e57::StructureNode sNode(_childNode);
        int64_t childCount = sNode.childCount();
        for (uint32_t d = 0; d < _depth + 1; d++)
            _os << "|  ";
        _os << "|--E57_STRUCTURE {" << sNode.elementName() << "}" << std::endl;

        for (int64_t i = 0; i < childCount; i++)
        {
            e57::Node child = sNode.get(i);
            printNode(child, _depth + 1, _os);
        }
        break;
    }
    case e57::E57_VECTOR:
    {
        e57::VectorNode vNode(_childNode);
        int64_t childCount = vNode.childCount();
        for (uint32_t d = 0; d < _depth + 1; d++)
            _os << "|  ";
        if (vNode.elementName() == "data3D")
            _os << "|--E57_VECTOR {" << vNode.elementName() << "} - (" << childCount << " sets)" << std::endl;
        else
            _os << "|--E57_VECTOR {" << vNode.elementName() << "}" << std::endl;

        for (int64_t i = 0; i < childCount; i++)
        {
            e57::Node child = vNode.get(i);
            printNode(child, _depth + 1, _os);
        }
        break;
    }
    case e57::E57_COMPRESSED_VECTOR:
    {
        e57::CompressedVectorNode cvNode(_childNode);
        for (uint32_t d = 0; d < _depth + 1; d++)
            _os << "|  ";
        int64_t ptsCount = cvNode.childCount();
        //if (cvNode.elementName() == "points")  m_nbTotalPoints += ptsCount;
        _os << "|--E57_COMPRESSED_VECTOR {" << cvNode.elementName() << "} -- " << ptsCount << " records" << std::endl;

        e57::Node proto = cvNode.prototype();
        printNode(proto, _depth + 1, _os);
        break;
    }
    case e57::E57_INTEGER:
    {
        for (uint32_t d = 0; d < _depth + 1; d++)
            _os << "|  ";
        _os << "|--E57_INTEGER {" << _childNode.elementName() << "} -- value = " << e57::IntegerNode(_childNode).value() << std::endl;
        break;
    }
    case e57::E57_SCALED_INTEGER:
    {
        for (uint32_t d = 0; d < _depth + 1; d++)
            _os << "|  ";
        e57::ScaledIntegerNode scIntNode(_childNode);
        _os << "|--E57_SCALED_INTEGER {" << _childNode.elementName() << "} -- raw value = " << scIntNode.rawValue() << ", scale = " << scIntNode.scale() << ", offset = " << scIntNode.offset() << ", scaled value = " << scIntNode.scaledValue() << std::endl;
        break;
    }
    case e57::E57_FLOAT:
    {
        for (uint32_t d = 0; d < _depth + 1; d++)
            _os << "|  ";
        double fv = e57::FloatNode(_childNode).value();
        _os << "|--E57_FLOAT {" << _childNode.elementName() << "} -- value = " << fv << std::endl;
        break;
    }
    case e57::E57_STRING:
    {
        for (uint32_t d = 0; d < _depth + 1; d++)
            _os << "|  ";
        _os << "|--E57_STRING {" << _childNode.elementName() << "} -- string = " << e57::StringNode(_childNode).value() << std::endl;
        break;
    }
    case e57::E57_BLOB:
    {
        for (uint32_t d = 0; d < _depth + 1; d++)
            _os << "|  ";
        _os << "|--E57_BLOB {" << _childNode.elementName() << "} -- byte count = " << e57::BlobNode(_childNode).byteCount() << std::endl;
        break;
    }
    default:
        break;
    }
}
