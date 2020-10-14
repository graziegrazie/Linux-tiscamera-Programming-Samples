#include <yaml-cpp/yaml.h>
//#include <yaml-cpp/node/node.h>
//#include <yaml.h>
//#include <node/node.h>
#include <tcamcamera.h>
#include <string>
#include <fstream>
#include <iostream>

#include <common.h>

struct Enemy{ 
    std::string name;
    int hp;
    int atk;
};
struct Info{ 
    std::vector<Enemy> enemy;
    std::string world;
    std::string player;
};

using TISCameraCfg = struct TISCameraCfg_
{
    std::string serial;   // serial number of a TIS camera
    std::string format;   // bayer or raw
    std::string type;     // RGB, YUV, RGGB, RGGB16 etc
    gsttcam::FrameSize frame_size;
    gsttcam::FrameRate frame_rate;
};
/*
void operator >> (const YAML::Node& node, Enemy& enemy){
    node["name"] >> enemy.name;
    node["hp"] >> enemy.hp;
    node["atk"] >> enemy.atk;
}
void operator >> (const YAML::Node& node, Info& info){
    const YAML::Node& enemies = node["enemy"];
    for(int i=0;i<enemies.size();i++){
        Enemy enemy;
        enemies[i] >> enemy;
        info.enemy.push_back(enemy);
    } 
    node["world"] >> info.world;
    node["player"] >> info.player;
} 
*/
/*
void loadYMLFile(string ymlpath){ 
    std::string name;
    int hp; 
    int atk;
    std::string world;
    std::string player;
    try{ 
        std::ifstream fin(ymlpath);
        YAML::Parser parser(fin);
        YAML::Node doc; 
        parser.GetNextDocument(doc);
        Info info;
        for(int i=0;i<doc.size();i++){
            Info info; doc[i] >> info;
            for(int j=0;j<info.enemy.size();j++){
                name = info.enemy[j].name;
                hp = info.enemy[j].hp;
                atk = info.enemy[j].atk;
                std::cout << "name:" << name << "\n" << "HP:" << hp << "\n" << "ATK:" << atk << std::endl;
            } 
            world = info.world;
            player = info.player;
            std::cout << "world:" << world << "\n" << "player:" << player << std::endl;
        }
    }catch(YAML::ParserException& e){ 
        std::cerr << e.what() << std::endl;
    }
} 
*/
int main(int argc, char* argv[])
{
    /*
    std::string directory_name;
    make_directory_with_current_date_as_name(directory_name);

    std::stringstream ss;
    ss << "cp ";
    ss << argv[1];
    ss << " ";
    ss << directory_name;
    ss << "/";
    std::cout << ss.str() << std::endl;

    system(ss.str().c_str());

    std::stringstream ss_cfg_file_name;
    //ss_cfg_file_name << directory_name;
    //ss_cfg_file_name << "/";
    ss_cfg_file_name << argv[1];
    std::cout << ss_cfg_file_name.str() << std::endl;

    */
    std::cout << "hoge 0" << std::endl;
    YAML::Node config = YAML::Load("[1, 2, 3]");//YAML::LoadFile("tis_camera_cfg.yaml");
    std::cout << "hoge 1" << std::endl;
    if(nullptr == config)
    {
        std::cout << "null" << std::endl;
    }
    else
    {
        std::cout << "not null" << std::endl;
    }
    //YAML::Node config = YAML::LoadFile("appveyor.yml");

    if ( true == config.IsNull() )
    {
        std::cout << "config is null" << std::endl;
    }
    else
    {
        std::cout << "config is valid" << std::endl;
    }
    std::cout << "hoge 1" << std::endl;
    //std::cout << "node num = " << config.size() << std::endl;
    //std::cout << "serial         " << config["serial"].as<std::string>() << std::endl;
    //config["serial"].as<std::string>();
    /*
    std::cout << "format         " << config["format"] << std::endl;
    std::cout << "type           " << config["type"]   << std::endl;
    std::cout << "FR numerator   " << config["FrameRate"]["numerator"]   << std::endl;
    std::cout << "FR denominator " << config["FrameRate"]["denominator"] << std::endl;
    std::cout << "FS width       " << config["FrameSize"]["width"]  << std::endl;
    std::cout << "FS height      " << config["FrameSize"]["height"] << std::endl;
    */
/*
    YAML::Node config = YAML::LoadFile(ymlpath);
    if (config["serial]"])
    {
        std::cout << "hoge" << std::endl;
    }
    else
    {

    }
*/
    return 0;
} 