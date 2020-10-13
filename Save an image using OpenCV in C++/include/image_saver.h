#include <iostream>
#include <tcamcamera.h>

namespace gsttcam
{

struct CameraParam
{
    std::string serial_num;
    std::string data_format;
    std::string data_type;
    
    gsttcam::FrameRate frame_rate;
    gsttcam::FrameSize frame_size;

    bool is_color;
    int  bit_depth;
};

class ImageSaver
{
public:
    std::string save_directory_name_;

    ImageSaver(std::string serial_number)
    {
        this->p_cam_ = new TcamCamera(serial_number);
        this->makeDirectoryWithCurrentDateAsName(this->save_directory_name_);
    }

    ~ImageSaver()
    {
        this->p_cam_->stop();
    };

    bool setParam(CameraParam& param);
    bool saveParamToConfigFile(CameraParam& param, std::string file_path);
    bool setCaptureCallback(gpointer data);

    bool startCameraCapture()
    {
        bool result = true;

        this->p_cam_->start();

        return result;
    }

    bool stopCameraCapture()
    {
        bool result = true;

        this->p_cam_->stop();

        return result;
    }

private:
    TcamCamera* p_cam_;
    CameraParam param_;

    bool makeDirectoryWithCurrentDateAsName(std::string& directory_name);
    
    GstFlowReturn captureCallback(GstAppSink *appsink, gpointer data);
};

};