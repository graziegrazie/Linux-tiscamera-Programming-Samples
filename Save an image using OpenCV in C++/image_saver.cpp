#include <image_saver.h>
#include <sys/stat.h>
#include <iomanip>
#include <time.h>
#include <sstream>
#include "opencv2/opencv.hpp"

namespace gsttcam
{

typedef struct
{
    int ImageCounter;
    bool SaveNextImage;
    bool busy;
   	cv::Mat frame; 
} CUSTOMDATA;

bool ImageSaver::makeDirectoryWithCurrentDateAsName(std::string& directory_name)
{
    bool result = true;

    time_t t = time(nullptr);
    const tm* lt = localtime(&t);

    std::stringstream ss;
    ss << "20";
    ss << lt->tm_year - 100; ss << "-";
    ss << std::setfill('0') << std::setw(2) << lt->tm_mon  + 1; ss << "-";
    ss << std::setfill('0') << std::setw(2) << lt->tm_mday;     ss << "-";
    ss << std::setfill('0') << std::setw(2) << lt->tm_hour;     ss << "-";
    ss << std::setfill('0') << std::setw(2) << lt->tm_min;      ss << "-";
    ss << std::setfill('0') << std::setw(2) << lt->tm_sec;

    directory_name = ss.str();

    // TODO: permission should be masked
    if( -1 == mkdir(directory_name.c_str(), 0777) )
    {
        result = false;
    }
    else
    {
        result = true;
    }

    return result;
}

bool ImageSaver::setParam(CameraParam& param)
{
    bool result = true;

    this->param_ = param;
    this->p_cam_->set_capture_format(param.data_type, param.data_format,
                                    FrameSize{param.frame_size.width,     param.frame_size.height},
                                    FrameRate{param.frame_rate.numerator, param.frame_rate.denominator});

    return result;
}

bool ImageSaver::saveParamToConfigFile(CameraParam& param, std::string file_path)
{
    bool result = true;

    assert("" != file_path);
    std::ofstream ofs(file_path);
    assert(ofs);

    ofs << "serial: "     << param.serial_num << std::endl;
    ofs << "type: "       << "video/x-" << param.data_type << std::endl;
    ofs << "format: "     << param.data_format << std::endl;
    ofs << "frame_size: " << "{" << param.frame_size.width     << ", " << param.frame_size.height      << "}" << std::endl;
    ofs << "frame_rate: " << "{" << param.frame_rate.numerator << ", " << param.frame_rate.denominator << "}" << std::endl;

    return result;
}

bool ImageSaver::setCaptureCallback(gpointer data)
{
    bool result = true;

    auto callback = std::bind(&ImageSaver::captureCallback, this, std::placeholders::_1, std::placeholders::_2);
    this->p_cam_->set_new_frame_callback(callback, data);

    return result;
}

GstFlowReturn ImageSaver::captureCallback(GstAppSink *appsink, gpointer data)
{
    int width, height ;
    const GstStructure *str;

    // Cast gpointer to CUSTOMDATA*
    CUSTOMDATA *pCustomData = (CUSTOMDATA*)data;
    if( !pCustomData->SaveNextImage)
    {
        return GST_FLOW_OK;
    }

    pCustomData->SaveNextImage = false;
    pCustomData->ImageCounter++;

    // The following lines demonstrate, how to acces the image
    // data in the GstSample.
    GstSample *sample = gst_app_sink_pull_sample(appsink);
    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstMapInfo info;

    gst_buffer_map(buffer, &info, GST_MAP_READ);
    
    if( NULL == info.data )
    {
        std::cout << "cb 0" << std::endl;
        gst_buffer_unmap(buffer, &info);
        gst_sample_unref(sample);

        return GST_FLOW_OK;
    }
    else
    {
        std::cout << "cb 1" << std::endl;
        // keep going
    }

    int num_of_pixel_byte = (int)ceil(this->param_.bit_depth / 8.0);
    int num_of_channel;
    if( 3 == this->param_.is_color )
    {
        std::cout << "cb 2" << std::endl;
        if( this->param_.data_format.find("x") >= 0 )
        {
            std::cout << "cb 2-1" << std::endl;
            // in this case, image data has alpha channel
            num_of_channel = 4;
        }
        else
        {
            std::cout << "cb 2-2" << std::endl;
            num_of_channel = 3;
        }
    }
    else if( 1 == this->param_.is_color )
    {
        std::cout << "cb 3" << std::endl;
        num_of_channel = 1;
    }
    else
    {
        std::cout << "cb 4" << std::endl;
        // TODO: throw exception!!
    }

    GstCaps *caps = gst_sample_get_caps(sample);
    // Get a string containg the pixel format, width and height of the image        
    str = gst_caps_get_structure (caps, 0);
    std::string received_data_type = gst_structure_get_string (str, "format");

    gst_structure_get_int (str, "width",  &width);
    gst_structure_get_int (str, "height", &height);

    cv::Mat temp;

    if( this->param_.data_type.find("x-raw") >= 0 )
    {
        std::cout << "cb 5" << std::endl;
        switch( num_of_pixel_byte )
        {
            case 1:
            std::cout << "cb 5-1" << std::endl;
                pCustomData->frame.create(height,width,CV_8UC(num_of_channel));
                memcpy( pCustomData->frame.data, info.data, width*height*num_of_pixel_byte );

                temp = cv::Mat(height, width, CV_8UC(num_of_channel));
                break;
            case 2:
                std::cout << "cb 5-2" << std::endl;
                pCustomData->frame.create(height,width,CV_16UC(num_of_channel));
                memcpy( pCustomData->frame.data, info.data, width*height*num_of_pixel_byte );

                temp = cv::Mat(height, width, CV_16UC(num_of_channel));
                break;
            default:
                std::cout << "cb 5-3" << std::endl;
                break;
        }
    }
    else if( this->param_.data_type.find("x-bayer") >= 0 )
    {
        std::cout << "cb 6" << std::endl;
        switch( num_of_pixel_byte )
        {
            case 1:
                std::cout << "cb 6-1" << std::endl;
                pCustomData->frame.create(height,width,CV_8UC(1));
                memcpy( pCustomData->frame.data, info.data, width*height*num_of_pixel_byte);

                temp = cv::Mat(height, width, CV_8UC(num_of_channel));
                break;
            case 2:
                std::cout << "cb 6-2" << std::endl;
                pCustomData->frame.create(height,width,CV_16UC(1));
                memcpy( pCustomData->frame.data, info.data, width*height*num_of_pixel_byte);

                temp = cv::Mat(height, width, CV_16UC(num_of_channel));
                break;
            default:
                std::cout << "cb 6-3" << std::endl;
                break;
        }
    }
    else
    {
        // no operation
        std::cout << "cb 7" << std::endl;
    }

    std::stringstream ss;
    ss << this->save_directory_name_;
    ss << "/";
    ss << "image";
    ss << std::setfill('0') << std::setw(5) << pCustomData->ImageCounter;
    ss << ".jpg";
    cv::imwrite(ss.str().c_str(), pCustomData->frame);

    // Calling Unref is important!
    gst_buffer_unmap (buffer, &info);
    gst_sample_unref(sample);

    // Set our flag of new image to true, so our main thread knows about a new image.
   return GST_FLOW_OK;
}

};