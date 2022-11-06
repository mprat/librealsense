// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2022 Intel Corporation. All Rights Reserved.

#include <realdds/dds-stream-profile.h>
#include <realdds/dds-exceptions.h>

#include <librealsense2/utilities/json.h>
using nlohmann::json;

#include <map>


namespace realdds {


dds_stream_format::dds_stream_format( std::string const & s )
{
    if( s.length() > size )
        DDS_THROW( runtime_error, "format is too long" );
    data[s.copy( data, size )] = 0;
}


template< typename T > uint32_t rs_fourcc( const T a, const T b, const T c, const T d )
{
    static_assert( ( std::is_integral< T >::value ), "rs_fourcc supports integral built-in types only" );
    return ( ( static_cast< uint32_t >( a ) << 24 ) | ( static_cast< uint32_t >( b ) << 16 )
             | ( static_cast< uint32_t >( c ) << 8 ) | ( static_cast< uint32_t >( d ) << 0 ) );
}


enum rs2_format  // copy from rs2_sensor.h
{
    RS2_FORMAT_ANY, /**< When passed to enable stream, librealsense will try to provide best suited format */
    RS2_FORMAT_Z16, /**< 16-bit linear depth values. The depth is meters is equal to depth scale * pixel value. */
    RS2_FORMAT_DISPARITY16, /**< 16-bit float-point disparity values. Depth->Disparity conversion : Disparity =
                               Baseline*FocalLength/Depth. */
    RS2_FORMAT_XYZ32F,      /**< 32-bit floating point 3D coordinates. */
    RS2_FORMAT_YUYV,  /**< 32-bit y0, u, y1, v data for every two pixels. Similar to YUV422 but packed in a different
                         order - https://en.wikipedia.org/wiki/YUV */
    RS2_FORMAT_RGB8,  /**< 8-bit red, green and blue channels */
    RS2_FORMAT_BGR8,  /**< 8-bit blue, green, and red channels -- suitable for OpenCV */
    RS2_FORMAT_RGBA8, /**< 8-bit red, green and blue channels + constant alpha channel equal to FF */
    RS2_FORMAT_BGRA8, /**< 8-bit blue, green, and red channels + constant alpha channel equal to FF */
    RS2_FORMAT_Y8,    /**< 8-bit per-pixel grayscale image */
    RS2_FORMAT_Y16,   /**< 16-bit per-pixel grayscale image */
    RS2_FORMAT_RAW10, /**< Four 10 bits per pixel luminance values packed into a 5-byte macropixel */
    RS2_FORMAT_RAW16, /**< 16-bit raw image */
    RS2_FORMAT_RAW8,  /**< 8-bit raw image */
    RS2_FORMAT_UYVY,  /**< Similar to the standard YUYV pixel format, but packed in a different order */
    RS2_FORMAT_MOTION_RAW,    /**< Raw data from the motion sensor */
    RS2_FORMAT_MOTION_XYZ32F, /**< Motion data packed as 3 32-bit float values, for X, Y, and Z axis */
    RS2_FORMAT_GPIO_RAW,      /**< Raw data from the external sensors hooked to one of the GPIO's */
    RS2_FORMAT_6DOF, /**< Pose data packed as floats array, containing translation vector, rotation quaternion and
                        prediction velocities and accelerations vectors */
    RS2_FORMAT_DISPARITY32, /**< 32-bit float-point disparity values. Depth->Disparity conversion : Disparity =
                               Baseline*FocalLength/Depth */
    RS2_FORMAT_Y10BPACK,    /**< 16-bit per-pixel grayscale image unpacked from 10 bits per pixel packed
                               ([8:8:8:8:2222]) grey-scale image. The data is unpacked to LSB and padded with 6 zero
                               bits */
    RS2_FORMAT_DISTANCE,    /**< 32-bit float-point depth distance value.  */
    RS2_FORMAT_MJPEG, /**< Bitstream encoding for video in which an image of each frame is encoded as JPEG-DIB   */
    RS2_FORMAT_Y8I,   /**< 8-bit per pixel interleaved. 8-bit left, 8-bit right.  */
    RS2_FORMAT_Y12I,  /**< 12-bit per pixel interleaved. 12-bit left, 12-bit right. Each pixel is stored in a 24-bit
                         word in little-endian order. */
    RS2_FORMAT_INZI,  /**< multi-planar Depth 16bit + IR 10bit.  */
    RS2_FORMAT_INVI,  /**< 8-bit IR stream.  */
    RS2_FORMAT_W10,   /**< Grey-scale image as a bit-packed array. 4 pixel data stream taking 5 bytes */
    RS2_FORMAT_Z16H,  /**< Variable-length Huffman-compressed 16-bit depth values. */
    RS2_FORMAT_FG,    /**< 16-bit per-pixel frame grabber format. */
    RS2_FORMAT_Y411,  /**< 12-bit per-pixel. */
    RS2_FORMAT_Y16I,  /**< 12-bit per pixel interleaved. 12-bit left, 12-bit right. */
    RS2_FORMAT_COUNT  /**< Number of enumeration values. Not a valid input: intended to be used in for-loops. */
};


int dds_stream_format::to_rs2() const
{
    static std::map< uint32_t, int > fcc_to_rs2{  // copy from ds5-device.cpp
        { rs_fourcc( 'Y', 'U', 'Y', '2' ), RS2_FORMAT_YUYV },
        { rs_fourcc( 'Y', 'U', 'Y', 'V' ), RS2_FORMAT_YUYV },
        { rs_fourcc( 'U', 'Y', 'V', 'Y' ), RS2_FORMAT_UYVY },
        { rs_fourcc( 'G', 'R', 'E', 'Y' ), RS2_FORMAT_Y8 },
        { rs_fourcc( 'Y', '8', 'I', ' ' ), RS2_FORMAT_Y8I },
        { rs_fourcc( 'W', '1', '0', ' ' ), RS2_FORMAT_W10 },
        { rs_fourcc( 'Y', '1', '6', ' ' ), RS2_FORMAT_Y16 },
        { rs_fourcc( 'Y', '1', '2', 'I' ), RS2_FORMAT_Y12I },
        { rs_fourcc( 'Y', '1', '6', 'I' ), RS2_FORMAT_Y16I },
        { rs_fourcc( 'Z', '1', '6', ' ' ), RS2_FORMAT_Z16 },
        { rs_fourcc( 'Z', '1', '6', 'H' ), RS2_FORMAT_Z16H },
        { rs_fourcc( 'R', 'G', 'B', '8' ), RS2_FORMAT_RGB8 },
        { rs_fourcc( 'R', 'G', 'B', 'A' ), RS2_FORMAT_RGBA8 },
        { rs_fourcc( 'R', 'G', 'B', '2' ), RS2_FORMAT_BGR8 },
        { rs_fourcc( 'B', 'G', 'R', 'A' ), RS2_FORMAT_BGRA8 },
        { rs_fourcc( 'M', 'J', 'P', 'G' ), RS2_FORMAT_MJPEG },
        { rs_fourcc( 'C', 'N', 'F', '4' ), RS2_FORMAT_RAW8 },
        { rs_fourcc( 'B', 'Y', 'R', '2' ), RS2_FORMAT_RAW16 },
        { rs_fourcc( 'M', 'X', 'Y', 'Z' ), RS2_FORMAT_MOTION_XYZ32F },
    };

    std::string s = to_string();
    s.resize( size, ' ' );  // pad with ' '
    char const * data = s.data();
    auto it = fcc_to_rs2.find( rs_fourcc( s[0], s[1], s[2], s[3] ) );
    if( it == fcc_to_rs2.end() )
        DDS_THROW( runtime_error, "invalid format '" + to_string() + "'" );
    return it->second;
}


dds_stream_format dds_stream_format::from_rs2( int rs2_format )
{
    char const * fourcc = nullptr;
    switch( rs2_format )
    {
    case RS2_FORMAT_YUYV: fourcc = "YUYV"; break;
    case RS2_FORMAT_Y8: fourcc = "GREY"; break;
    case RS2_FORMAT_Y8I: fourcc = "Y8I"; break;
    case RS2_FORMAT_W10: fourcc = "W10"; break;
    case RS2_FORMAT_Y16: fourcc = "Y16"; break;
    case RS2_FORMAT_Y12I: fourcc = "Y12I"; break;
    case RS2_FORMAT_Y16I: fourcc = "Y16I"; break;
    case RS2_FORMAT_Z16: fourcc = "Z16"; break;
    case RS2_FORMAT_Z16H: fourcc = "Z16H"; break;
    case RS2_FORMAT_RGB8: fourcc = "RGB8"; break;
    case RS2_FORMAT_RGBA8: fourcc = "RGBA"; break;  // todo
    case RS2_FORMAT_BGR8: fourcc = "RGB2"; break;
    case RS2_FORMAT_BGRA8: fourcc = "BGRA"; break;  // todo
    case RS2_FORMAT_MJPEG: fourcc = "MJPG"; break;
    case RS2_FORMAT_RAW8: fourcc = "CNF4"; break;
    case RS2_FORMAT_RAW16: fourcc = "BYR2"; break;
    case RS2_FORMAT_UYVY: fourcc = "UYVY"; break;
    case RS2_FORMAT_MOTION_XYZ32F: fourcc = "MXYZ"; break;  // todo
    default:
        DDS_THROW( runtime_error, "cannot translate rs2_format " + std::to_string( rs2_format ) + " to any known dds_stream_format" );
    };
    return dds_stream_format( fourcc );
}


dds_stream_profile::dds_stream_profile( json const & j, int & it )
    : _frequency( utilities::json::get< int16_t >( j, it++ ) )
    , _format( utilities::json::get< std::string >( j, it++ ) )
{
    // NOTE: the order of construction is the order of declaration -- therefore the to_json() function
    // should use the same ordering!
}


/* static */ void dds_stream_profile::verify_end_of_json( nlohmann::json const & j, int index )
{
    if( index != j.size() )
        DDS_THROW( runtime_error, "expected end of json at index " + std::to_string( index ) );
}


std::string dds_stream_profile::to_string() const
{
    std::ostringstream os;
    os << '<' << details_to_string();
    os << '>';
    return os.str();
}

std::string dds_stream_profile::details_to_string() const
{
    std::ostringstream os;
    os << _format.to_string() << " @ " << _frequency << " Hz";
    return os.str();
}

std::string dds_video_stream_profile::details_to_string() const
{
    std::ostringstream os;
    os << _width << 'x' << _height << ' ';
    os << super::details_to_string();
    return os.str();
}


void dds_stream_profile::init_stream( std::weak_ptr< dds_stream_base > const & stream )
{
    if( ! stream.lock() )
        DDS_THROW( runtime_error, "cannot set stream to null" );
    if( _stream.lock() )
        DDS_THROW( runtime_error, "profile is already associated with a stream" );
    _stream = stream;
}


json dds_stream_profile::to_json() const
{
    // NOTE: same ordering as construction!
    return json::array( { frequency(), format().to_string() } );
}


dds_video_stream_profile::dds_video_stream_profile( nlohmann::json const & j, int & index )
    : super( j, index )
{
    _width = utilities::json::get< int16_t >( j, index++ );
    _height = utilities::json::get< int16_t >( j, index++ );
}


json dds_video_stream_profile::to_json() const
{
    auto profile = super::to_json();
    profile += width();
    profile += height();
    return profile;
}


}  // namespace realdds
