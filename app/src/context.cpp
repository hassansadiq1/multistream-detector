#include "context.h"

ProjectContext::ProjectContext()
{
    MUXER_OUTPUT_WIDTH = 1280;
    MUXER_OUTPUT_HEIGHT = 720;
    MUXER_BATCH_SIZE = 4;
    MUXER_BATCH_TIMEOUT_USEC = 40000;
    TILED_OUTPUT_WIDTH = 1920;
    TILED_OUTPUT_HEIGHT = 1080;
}

void ProjectContext::loadConfig(char *str1, char *str2) {
    std::ifstream fin(SOURCE_PROPERTIES);
    std::string line;
    while (getline(fin, line)) {
        std::istringstream sin(line.substr(line.find("=") + 1));
        if(line[0] == '#')
            continue;

        if (line.find(str1) != -1){
            sin >> str2;
            return;
        }
    }
    std::cout<<"Could not find "<<str1<<" property. Exiting"<<std::endl;
    exit(0);
}

void ProjectContext::loadStrConfig(char *str1, string& str2) {
    std::ifstream fin(SOURCE_PROPERTIES);
    std::string line;
    while (getline(fin, line)) {
        std::istringstream sin(line.substr(line.find("=") + 1));
        if(line[0] == '#')
            continue;

        if (line.find(str1) != -1){
            sin >> str2;
            return;
        }
    }
    std::cout<<"Could not find "<<str1<<" property. Exiting"<<std::endl;
    exit(0);
}

void ProjectContext::loadSourceProperties(SourceProperties *properties, int i){
    i+=1;
    char temp_char[150];
    string temp_str1, temp_str2;
    
    temp_str1 = "uri" + to_string(i);
    loadStrConfig((char *) temp_str1.c_str(), temp_str2);
    properties->uri = temp_str2;
    temp_str1 = "source_id" + to_string(i);
    loadConfig((char *) temp_str1.c_str(), temp_char);
    properties->source_id = atoi(temp_char);

    //polygon pareameters
    temp_str1 = "TopLeftX" + to_string(i);
    loadConfig((char *) temp_str1.c_str(), temp_char);
    properties->TopLeftX = atoi(temp_char);
    temp_str1 = "TopLeftY" + to_string(i);
    loadConfig((char *) temp_str1.c_str(), temp_char);
    properties->TopLeftY = atoi(temp_char);
    temp_str1 = "BottomRightX" + to_string(i);
    loadConfig((char *) temp_str1.c_str(), temp_char);
    properties->BottomRightX = atoi(temp_char);
    temp_str1 = "BottomRightY" + to_string(i);
    loadConfig((char *) temp_str1.c_str(), temp_char);
    properties->BottomRightY = atoi(temp_char);
}

bool sourceManager::sourceExists(int source_id){
}

SourceProperties* sourceManager::getSourceProperties(int source_id){
    return nullptr;
}