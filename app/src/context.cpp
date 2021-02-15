#include "context.h"

ProjectContext::ProjectContext()
{
    MUXER_OUTPUT_WIDTH = 1280;
    MUXER_OUTPUT_HEIGHT = 720;
    MUXER_BATCH_TIMEOUT_USEC = 125000;
    TILED_OUTPUT_WIDTH = 1280;
    TILED_OUTPUT_HEIGHT = 720;
    char temp_char[4];
    loadConfig((char *)"MUXER_BATCH_SIZE", temp_char);
    MUXER_BATCH_SIZE = atoi(temp_char);
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
    temp_str1 = "polygon_sides" + to_string(i);
    loadConfig((char *) temp_str1.c_str(), temp_char);
    properties->polygon_sides = atoi(temp_char);

    properties->polygon = new Point [properties->polygon_sides];
    temp_str1 = "coordinates" + to_string(i);
    loadStrConfig((char *) temp_str1.c_str(), temp_str2);

    vector<string> v; 
    stringstream ss(temp_str2); 
    while (ss.good()) {
        string substr;
        getline(ss, substr, ';');
        substr.erase(substr.begin());
        substr.erase(substr.end() - 1);
        v.push_back(substr);
    }
 
    for (size_t i = 0; i < v.size(); i++){
        stringstream ss1(v[i]);
        int j = 0;
        Point p;
        while (ss1.good()) {
            string substr;
            getline(ss1, substr, ',');
            if (j == 0)
                p.x = atoi(substr.c_str());
            else
                p.y = atoi(substr.c_str());
            j++;
        }
        properties->polygon[i] = p;
    }
//    for(int i = 0; i < properties->polygon_sides; i++){
//        cout<<properties->polygon[i].x <<"\t" <<properties->polygon[i].y<<endl;
//    }

}

bool sourceManager::sourceExists(int source_id){
}

SourceProperties* sourceManager::getSourceProperties(int source_id){
    return nullptr;
}