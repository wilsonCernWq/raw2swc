//
// Created by 韩孟娇 on 2/18/18.
//

#ifndef RAW2SWC_HELPER_H
#define RAW2SWC_HELPER_H

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cmath>

inline void PrintBuddha (std::ostream& os) 
{
  os << "                                                       \n"
     << "                       _oo0oo_                         \n"
     << "                      o8888888o                        \n"
     << "                      88\" . \"88                        \n"
     << "                      (| -_- |)                        \n"
     << "                      0\\  =  /0                        \n"
     << "                    ___/`---'\\___                      \n"
     << "                  .' \\\\|     |// '.                    \n"
     << "                 / \\\\|||  :  |||// \\                   \n"
     << "                / _||||| -:- |||||- \\                  \n"
     << "               |   | \\\\\\  -  /// |   |                 \n"      
     << "               | \\_|  ''\\---/''  |_/ |                 \n"
     << "               \\  .-\\__  '-'  ___/-. /                 \n"
     << "             ___'. .'  /--.--\\  `. .'___               \n"
     << "          .\"\" '<  `.___\\_<|>_/___.' >' \"\".             \n"
     << "         | | :  `- \\`.;`\\ _ /`;.`/ - ` : | |           \n"
     << "         \\  \\ `_.   \\_ __\\ /__ _/   .-` /  /           \n"
     << "     =====`-.____`.___ \\_____/___.-`___.-'=====        \n"
     << "                       `=---='                         \n"
     << "                                                       \n"
     << "                                                       \n"
     << "     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~       \n"
     << "                                                       \n"
     << "               佛祖保佑         永无BUG                \n"
     << "                                                       \n"
     << std::endl;
}

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

template<typename T>
T readBinary(std::ifstream& file) {
  T x; file.read((char*)&x, sizeof(T));
  return x;
}

template<typename T> T lerp(const T& a, const T& b, const double r)
{
  return (a * (1.0 - r) + b * r);
}

template<>
vec4f lerp<vec4f>(const vec4f& a, const vec4f& b, const double r)
{
  return vec4f{
    lerp(a.x, b.x, r),
    lerp(a.y, b.y, r),
    lerp(a.z, b.z, r),
    lerp(a.w, b.w, r)
  };
}

struct swcFile 
{
  std::vector<vec3f> position;
  std::vector<int> preIndex;
  std::vector<int> index;
  std::vector<float> radius;
  std::vector<vec4f> color;
  std::vector<int> component;
  void writeToOutput(const std::string& filename)
  {
    std::cout << "writting file: " << filename << std::endl;
    std::ofstream outfile (filename, std::ios::trunc);
    for (int i = 0; i < index.size(); i++) {
      outfile << index[i]      << ' '
	      << component[i]  << ' '
	      << position[i].x << ' '
	      << position[i].y << ' '
	      << position[i].z << ' '
	      << radius[i]     << ' '
	      << preIndex[i]   << ' '
	      << color[i].x   << ' '
	      << color[i].y   << ' '
	      << color[i].z   << ' '
	      << color[i].w
	      << '\n';
    }
    outfile.close();
  }
};

#endif //RAW2SWC_HELPER_H
