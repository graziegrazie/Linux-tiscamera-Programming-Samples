//////////////////////////////////////////////////////////////////
/*
Tcam Software Trigger
This sample shows, how to trigger the camera by software and use a callback for image handling.

Prerequisits
It uses the the examples/cpp/common/tcamcamera.cpp and .h files of the *tiscamera* repository as wrapper around the
GStreamer code and property handling. Adapt the CMakeList.txt accordingly.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "tcamcamera.h"
#include <unistd.h>

#include "opencv2/opencv.hpp"
#include "common.h"
#include <image_saver.h>

using namespace gsttcam;

//#define DATA_FORMAT  "rggb16"
#define DEBUG


#define TISCAMERA_SERIAL_NO                "45710317"
#define TISCAMERA_IMAGE_TYPE               "bayer"
#define TISCAMERA_IMAGE_FORMAT             "rggb16"
#define TISCAMERA_FRAME_SIZE_WIDTH         (4096)
#define TISCAMERA_FRAME_SIZE_HEIGHT        (3000)
#define TISCAMERA_FRAME_RATE_NUMERATOR     (5)
#define TISCAMERA_FRAME_RATE_DENOMINATOR   (1)

std::string image_save_directory_name;

// Create a custom data structure to be passed to the callback function. 
typedef struct
{
    int ImageCounter;
    bool SaveNextImage;
    bool busy;
   	cv::Mat frame; 
} CUSTOMDATA;

////////////////////////////////////////////////////////////////////
// List available properties helper function.
void ListProperties(TcamCamera &cam)
{
    // Get a list of all supported properties and print it out
    auto properties = cam.get_camera_property_list();
    std::cout << "Properties:" << std::endl;
    for(auto &prop : properties)
    {
        std::cout << prop->to_string() << std::endl;
    }
}

////////////////////////////////////////////////////////////////////
// Callback called for new images by the internal appsink
GstFlowReturn new_frame_cb(GstAppSink *appsink, gpointer data)
{
    //printf("new_frame_cb\n");
    int width, height ;
    const GstStructure *str;

    // Cast gpointer to CUSTOMDATA*
    CUSTOMDATA *pCustomData = (CUSTOMDATA*)data;
    if( !pCustomData->SaveNextImage)
        return GST_FLOW_OK;
    pCustomData->SaveNextImage = false;

    pCustomData->ImageCounter++;

    // The following lines demonstrate, how to acces the image
    // data in the GstSample.
    GstSample *sample = gst_app_sink_pull_sample(appsink);

    GstBuffer *buffer = gst_sample_get_buffer(sample);

    GstMapInfo info;

    gst_buffer_map(buffer, &info, GST_MAP_READ);
    
    if (info.data != NULL) 
    {
        // info.data contains the image data as blob of unsigned char 

        GstCaps *caps = gst_sample_get_caps(sample);
        // Get a string containg the pixel format, width and height of the image        
        str = gst_caps_get_structure (caps, 0);    

#ifdef DEBUG
        printf("data format = %s\n",  gst_structure_get_string (str, "format"));
#endif

        ///if( strcmp( gst_structure_get_string (str, "format"), DATA_FORMAT) == 0)  
        if( true )
        {
            // Now query the width and height of the image
            gst_structure_get_int (str, "width", &width);
            gst_structure_get_int (str, "height", &height);

            // Create a cv::Mat, copy image data into that and save the image.
#ifdef DEBUG
#if 1 //16bit
            pCustomData->frame.create(height,width,CV_16UC(1));
            cv::Mat temp(height, width, CV_16UC(3));
            memcpy( pCustomData->frame.data, info.data, width*height*2);
            //cv::cvtColor(pCustomData->frame, temp, cv::COLOR_BayerRG2BGR);
#else
            pCustomData->frame.create(height,width,CV_8UC(1));
            cv::Mat temp(height, width, CV_8UC(3));
            memcpy( pCustomData->frame.data, info.data, width*height);
            //cv::cvtColor(pCustomData->frame, temp, cv::COLOR_BayerRG2BGR);
#endif
            cv::cvtColor(pCustomData->frame, temp, cv::COLOR_BayerRG2RGB);
            char ImageFileName[256];
            sprintf(ImageFileName,"image%05d.png", pCustomData->ImageCounter);
            //cv::imwrite(ImageFileName,pCustomData->frame);
            //cv::imwrite(ImageFileName, temp);
            std::string hoge = image_save_directory_name + std::string("/") + std::string(ImageFileName);
            cv::imwrite(hoge.c_str(), temp);
#if 0
            // save bayer data as xml file
            char XMLFileName[256];
            sprintf(XMLFileName,"image%05d.xml", pCustomData->ImageCounter);

            cv::FileStorage xml_file(XMLFileName, cv::FileStorage::WRITE);
            //xml_file.writeRaw(std::string("u"), pCustomData->frame.data, pCustomData->frame.elemSize());
            xml_file << "data" << pCustomData->frame;
#endif
#else
            pCustomData->frame.create(height,width,CV_8UC(4));
            memcpy( pCustomData->frame.data, info.data, width*height*4);
            char ImageFileName[256];
            sprintf(ImageFileName,"image%05d.jpg", pCustomData->ImageCounter);
            cv::imwrite(ImageFileName,pCustomData->frame);
#endif
        }

    }
    
    // Calling Unref is important!
    gst_buffer_unmap (buffer, &info);
    gst_sample_unref(sample);

    // Set our flag of new image to true, so our main thread knows about a new image.
    return GST_FLOW_OK;
}

void save_camera_parameters_to_config_file(std::string file_path, TISCameraCfg& cfg)
{
    assert("" != file_path);

    std::ofstream ofs(file_path);
    assert(ofs);

    ofs << "serial: "     << cfg.serial << std::endl;
    ofs << "type: "       << "video/x-" << cfg.type << std::endl;
    ofs << "format: "     << cfg.format << std::endl;
    ofs << "frame_size: " << "{" << cfg.frame_size.width     << ", " << cfg.frame_size.height      << "}" << std::endl;
    ofs << "frame_rate: " << "{" << cfg.frame_rate.numerator << ", " << cfg.frame_rate.denominator << "}" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    gst_init(&argc, &argv);
    // Declare custom data structure for the callback
    CUSTOMDATA CustomData;

    CustomData.ImageCounter = 0;
    CustomData.SaveNextImage = false;

    std::shared_ptr<Property> ExposureAuto = NULL;
    std::shared_ptr<Property> ExposureValue = NULL;
    std::shared_ptr<Property> GainAuto = NULL;
    std::shared_ptr<Property> GainValue = NULL;
    
    printf("Tcam OpenCV Image Sample\n");
//
    //std::string image_save_directory_name;
#if 0
    make_directory_with_current_date_as_name(image_save_directory_name);
    
    TISCameraCfg cfg{};
    cfg.serial                 = TISCAMERA_SERIAL_NO;
    cfg.format                 = TISCAMERA_IMAGE_FORMAT;
    cfg.type                   = TISCAMERA_IMAGE_TYPE;
    cfg.frame_size.width       = TISCAMERA_FRAME_SIZE_WIDTH;
    cfg.frame_size.height      = TISCAMERA_FRAME_SIZE_HEIGHT;
    cfg.frame_rate.numerator   = TISCAMERA_FRAME_RATE_NUMERATOR;
    cfg.frame_rate.denominator = TISCAMERA_FRAME_RATE_DENOMINATOR;

    std::string file_path = image_save_directory_name + std::string("/config.txt");
    save_camera_parameters_to_config_file(file_path, cfg);
//

    // Open camera by serial number
    TcamCamera cam(TISCAMERA_SERIAL_NO);

    // Set video format, resolution and frame rate
    cam.set_capture_format(TISCAMERA_IMAGE_TYPE, TISCAMERA_IMAGE_FORMAT,
                           FrameSize{TISCAMERA_FRAME_SIZE_WIDTH,     TISCAMERA_FRAME_SIZE_HEIGHT},
                           FrameRate{TISCAMERA_FRAME_RATE_NUMERATOR, TISCAMERA_FRAME_RATE_DENOMINATOR});

    // Comment following line, if no live video display is wanted.
    //cam.enable_video_display(gst_element_factory_make("ximagesink", NULL));

    // Register a callback to be called for each new frame
    printf("set_new_frame_callback\n");
    cam.set_new_frame_callback(new_frame_cb, &CustomData);
    
    // Start the camera
    printf("start\n");
    cam.start();

#if 0
    ExposureAuto = cam.get_property("Exposure Auto");
    ExposureAuto->set(cam,0);
    ExposureValue = cam.get_property("Exposure");
    //ExposureValue->set(cam,185100);
    ExposureValue->set(cam,50000);
#endif

    // Uncomment following line, if properties shall be listed. Many of the
    // properties that are done in software are available after the stream 
    // has started. Focus Auto is one of them.
    // ListProperties(cam);
    for( int i = 0; i< 10; i++)
    {
        CustomData.SaveNextImage = true; // Save the next image in the callcack call
        sleep(1);
    }

    // Simple implementation of "getch()"
    printf("Press Enter to end the program");
    char dummyvalue[10];
    scanf("%c",dummyvalue);

    cam.stop();
#endif
    CameraParam param;
    param.serial_num = TISCAMERA_SERIAL_NO;
    param.data_format = "rggb16";
    param.data_type   = "bayer";
    
    param.frame_rate = {15, 1};
    param.frame_size = {4096, 3000};

    param.is_color  = true;;
    param.bit_depth = 12;

    ImageSaver* saver = new ImageSaver(TISCAMERA_SERIAL_NO);
    saver->setParam(param);

    std::string file_path = saver->save_directory_name_ + std::string("/config.txt");
    saver->saveParamToConfigFile(param, file_path);
    saver->setCaptureCallback(&CustomData);
    saver->startCameraCapture();

    for( int i = 0; i< 10; i++)
    {
        CustomData.SaveNextImage = true; // Save the next image in the callcack call
        sleep(1);
    }

    saver->stopCameraCapture();

    return 0;
}