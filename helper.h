//
// Created by 韩孟娇 on 2/18/18.
//

#ifndef RAW2SWC_HELPER_H
#define RAW2SWC_HELPER_H

#include <iostream>
#include <vector>
#include <cmath>
#include <math.h>

struct vec3f {
  float x, y, z;
};

struct vec4f {
  float x, y, z, w;
};

struct CommandLine {
  CommandLine(int argc, const char** argv);
  std::vector<std::string> inputFiles;
  std::string outputFile;
};

inline CommandLine::CommandLine(int argc, const char** argv) {
  for(int i = 1; i < argc; i++){
    const std::string arg = argv[i];
    if (arg == "--output") {
      outputFile = argv[++i];
    } else {
      inputFiles.push_back(arg);
    }
  }
}

struct swcFile {
  std::vector<vec3f> position;
  std::vector<int> preIndex;
  std::vector<int> index;
  std::vector<float> radius;
  std::vector<vec4f> color;
  std::vector<int> component;
};

void time2velocity(std::vector<float> &velocity, swcFile outputData)
{
  std::vector<int> lines;
  for (size_t i = 0; i< outputData.preIndex.size(); ++i) {
    if(outputData.preIndex[i] == -1){
      lines.push_back(outputData.index[i]);
    }
  }
  lines.push_back(outputData.index.size());

  for(size_t i = 0; i < lines.size() - 1; ++i) {
    if (i == lines.size() - 2) {
      // last line
      int start = lines[i];
      int end = lines[i + 1];
      for (int j = start; j < end - 1; j++) {
	float x = outputData.position[j].x;
	float x_next = outputData.position[j + 1].x;
	float y = outputData.position[j].y;
	float y_next = outputData.position[j + 1].y;
	float z = outputData.position[j].z;
	float z_next = outputData.position[j + 1].z;
	float dis = sqrtf(abs(x - x_next) * abs(x - x_next) + 
			  abs(y - y_next) * abs(y - y_next) +
			  abs(z - z_next) * abs(z - z_next));
	velocity.push_back(dis / 10.f);
      }
      velocity.push_back(velocity[velocity.size() - 1]);
    } else {
      int start = lines[i];
      int end = lines[i + 1] - 1;
      for (int j = start; j < end - 1; j++) {
	float x = outputData.position[j].x;
	float x_next = outputData.position[j + 1].x;
	float y = outputData.position[j].y;
	float y_next = outputData.position[j + 1].y;
	float z = outputData.position[j].z;
	float z_next = outputData.position[j + 1].z;
	float dis = sqrtf(abs(x - x_next) * abs(x - x_next) + 
			  abs(y - y_next) * abs(y - y_next) +
			  abs(z - z_next) * abs(z - z_next));
	velocity.push_back(dis / 10.f);
      }
      velocity.push_back(velocity[velocity.size() - 1]);
    }

  }

}


#endif //RAW2SWC_HELPER_H
