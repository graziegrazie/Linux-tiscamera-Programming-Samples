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
        gst_buffer_unmap(buffer, &info);
        gst_sample_unref(sample);

        return GST_FLOW_OK;
    }
    else
    {
        // keep going
    }

    GstCaps *caps = gst_sample_get_caps(sample);
    // Get a string containg the pixel format, width and height of the image        
    str = gst_caps_get_structure (caps, 0);
    std::cout << "image type = " << gst_structure_get_string (str, "format") << std::endl;

    gst_structure_get_int (str, "width",  &width);
    gst_structure_get_int (str, "height", &height);

#if 1 // use 16bit
    pCustomData->frame.create(height,width,CV_16UC(1));
    memcpy( pCustomData->frame.data, info.data, width*height*2);
    cv::Mat temp(height, width, CV_16UC(3));
    cv::cvtColor(pCustomData->frame, temp, cv::COLOR_BayerRG2RGB);
#else // use 8bit
    pCustomData->frame.create(height,width,CV_8UC(1));
    cv::Mat temp(height, width, CV_16UC(3));
    memcpy( pCustomData->frame.data, info.data, width*height*2);
#endif

    std::stringstream ss;
    ss << this->save_directory_name_;
    ss << "/";
    ss << "image";
    ss << std::setfill('0') << std::setw(5) << pCustomData->ImageCounter;
    //ss << ".png";
    ss << ".ppm";
    cv::imwrite(ss.str().c_str(), temp);

    // Calling Unref is important!
    gst_buffer_unmap (buffer, &info);
    gst_sample_unref(sample);

    // Set our flag of new image to true, so our main thread knows about a new image.
   return GST_FLOW_OK;
}

};