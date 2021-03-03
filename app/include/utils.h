#ifndef PARENTPROJECT_UTILS_H
#define PARENTPROJECT_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <pthread.h>

using namespace std;

struct Point 
{ 
	int x; 
	int y; 
};

class SourceProperties{
    public:
    string uri;
    int source_id;
    int polygon_sides;
    Point * polygon;
    int consecutiveMissedFrames = 0;
    bool firstFrame = 0;
};

class sourceManager{
    public:
    std::vector<SourceProperties*> allSources;
    std::vector<int> allSourcesStatus;
    int num_sources = 0;

    bool sourceExists(int source_id);
    SourceProperties* getSourceProperties(int source_id);

};

#endif
