//
//                       _oo0oo_
//                      o8888888o
//                      88" . "88
//                      (| -_- |)
//                      0\  =  /0
//                    ___/`---'\___
//                  .' \\|     |// '.
//                 / \\|||  :  |||// \
//                / _||||| -:- |||||- \
//               |   | \\\  -  /// |   |
//               | \_|  ''\---/''  |_/ |
//               \  .-\__  '-'  ___/-. /
//             ___'. .'  /--.--\  `. .'___
//          ."" '<  `.___\_<|>_/___.' >' "".
//         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//         \  \ `_.   \_ __\ /__ _/   .-` /  /
//     =====`-.____`.___ \_____/___.-`___.-'=====
//                       `=---='
//
//
//     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//               佛祖保佑         永无BUG
//
//
//
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "helper.h"

template<typename T>
T readBinary(std::ifstream& file) {
  T x; file.read((char*)&x, sizeof(T));
  return x;
}

// GLOBAL VARIABLES
static const std::string dataStr[] =  {
  "time",
  "magnititude", 
  "derivative" , 
  "difference" , 
  "vorticity"
};
size_t split_threshold = 4000;
size_t dataIdx = 0;               // which feature to choose (0 -> time)
std::vector<size_t>  fileIdx;     // the file id of each point
std::vector<swcFile> outputDataList;

std::vector<float> timeval;
std::vector<float> dataval;
std::vector<float> velocity;

bool write_velocity = false;

void writeToOutput(const swcFile &swcFileData, std::string filename)
{
  std::ofstream outfile (filename, std::ios::trunc);
  for (int i = 0; i < swcFileData.index.size(); i++) {
    outfile << swcFileData.index[i] << ' '
	    << swcFileData.component[i] << ' '
	    << swcFileData.position[i].x << ' '
	    << swcFileData.position[i].y << ' '
	    << swcFileData.position[i].z << ' '
	    << swcFileData.radius[i] << ' '
	    << swcFileData.preIndex[i] << ' '
	    << swcFileData.color[i].x << ' '
	    << swcFileData.color[i].y << ' '
	    << swcFileData.color[i].z << ' '
	    << swcFileData.color[i].w
	    << '\n';
  }
  outfile.close();
}

int main(int argc, const char** argv) 
{  
  // arguments
  std::vector<std::string> inputFiles;
  for (int i = 1; i < argc; ++i) {
    std::string str(argv[i]);
    if (str == "-idx") {
      dataIdx = std::atoi(argv[++i]);
    }
    else if (str == "-velocity") {
      write_velocity = true;
    }
    else if (str == "-split") {
      split_threshold = std::atoi(argv[++i]);
    } 
    else {
      inputFiles.push_back(argv[i]);
    }
  }
  std::string file_in  = inputFiles[0];
  std::string file_out = inputFiles[1];

  // open file
  std::ifstream infile;
  infile.open(file_in, std::ios::binary);

  // read meta data
  size_t num_lines    = 0;
  size_t num_features = 0;
  num_features = readBinary<int>(infile);
  num_lines    = readBinary<int>(infile);
  size_t num_files = (num_lines + split_threshold - 1) / split_threshold;

  // debug
  PrintBuddha(std::cout);
  std::cout << std::endl
	    << "raw2swc (佛祖保佑无BUG)" << std::endl
	    << "Parsing: " << file_in << std::endl
	    << "compute velocity? " << write_velocity << std::endl
	    << "split threshold = "    << split_threshold << std::endl
	    << "feature index   = "    << dataIdx      << std::endl
	    << "number of features = " << num_features << std::endl
	    << "number of lines    = " << num_lines    << std::endl
	    << "number of files    = " << num_files    << std::endl;

  // resize
  outputDataList.resize(num_files);

  for (auto f = 0; f < num_files; ++f) { 

    auto& outputData = outputDataList[f];
    auto N = std::min(num_lines, f * split_threshold + split_threshold);

    for (auto i = f * split_threshold; i < N; ++i) {

      const int num_points = readBinary<int>(infile);

      for (auto j = 0; j < num_points; ++j) {
	
	// record file id
	fileIdx.push_back(f);
	// coordinate
	const float x = readBinary<float>(infile);
	const float y = readBinary<float>(infile);
	const float z = readBinary<float>(infile);
	const vec3f p = {x, y, z};
	outputData.position.push_back(p);
	// features
	std::vector<float> comps(num_features + 1);
	for (auto k = 0; k < num_features + 1; ++k) {
	  comps[k] = readBinary<float>(infile);
	}
	// index
	outputData.index.push_back(outputData.position.size());
	// radius
	outputData.radius.push_back(0.15);
	// preIndex
	if (j == 0) {
	  outputData.preIndex.push_back(-1);
	} else {
	  outputData.preIndex.push_back(outputData.position.size() - 1);
	}
	// component
	outputData.component.push_back(0);
	// record features
	timeval.push_back(comps[0]);
	dataval.push_back(comps[dataIdx]);
      }
    }
  }
  std::cout << "done reading files" << std::endl;

  // transfer functions
  vec4f color0 = {0.0, 0.0, 1.0, 0.0};
  vec4f color1 = {0.86500299999999997, 0.86500299999999997,0.86500299999999997, 0.5};
  vec4f color2 = {1.0, 0.0, 0.0, 1.0};

  // convert feature to color
  if (write_velocity) {

    // // convert feature to velocity
    // for (int ii = 0; ii < (line_number + line_thres - 1) / line_thres; ++ii) {
    //   time2velocity(velocity, outputDataList[ii]);
    // }
    // // map velocity to color
    // std::vector<float>::iterator v_max_iterator, v_min_iterator;
    // v_max_iterator = std::max_element(velocity.begin(), velocity.end());
    // v_min_iterator = std::min_element(velocity.begin(), velocity.end());
    // float v_min = 5000, v_max, v_average;
    // // v_min = velocity[std::distance(velocity.begin(), v_min_iterator)];

    // for (size_t i = 0; i < velocity.size(); ++i) {
    //   if (v_min > velocity[i] && velocity[i] != 0) {
    // 	v_min = velocity[i];
    //   }
    // }

    // v_max = velocity[std::distance(velocity.begin(), v_max_iterator)];
    // v_average = (v_max + v_min) / (float) 2;

    // std::cout << "max = " << v_max << " min = " << v_min << std::endl;

    // float pre_halfDiff = v_average - v_min;
    // float next_halfDiff = v_max - v_average;

    // for(size_t i = 0; i < velocity.size(); ++i) {
    //   vec4f color;
    //   float temp_v = velocity[i];
    //   if (temp_v <= v_average) {
    // 	color.x = color0.x + ((color1.x - color0.x) / pre_halfDiff) * (temp_v - v_min);
    // 	color.y = color0.y + ((color1.y - color0.y) / pre_halfDiff) * (temp_v - v_min);
    // 	color.z = color0.z + ((color1.z - color0.z) / pre_halfDiff) * (temp_v - v_min);
    // 	color.w = 1.0;
    // 	// color.w = color0.w + ((color1.w - color0.w) / pre_halfDiff) * (temp_v - v_min);
    //   } else {
    // 	color.x = color1.x - ((color1.x - color2.x) / next_halfDiff) * (temp_v - v_average);
    // 	color.y = color1.y - ((color1.y - color2.y) / next_halfDiff) * (temp_v - v_average);
    // 	color.z = color1.z - ((color1.z - color2.z) / next_halfDiff) * (temp_v - v_average);
    // 	color.w = 1.0;
    // 	// color.w = color1.w + ((color2.w - color1.w) / next_halfDiff) * (temp_v - v_average);
    //   }
    //   outputDataList[(i + line_thres - 1)/ line_thres].color.push_back(color);
    // }

  } else {

    // find the max and min
    std::vector<float>::iterator time_max_iterator, time_min_iterator;
    time_max_iterator = std::max_element(dataval.begin(), dataval.end());
    time_min_iterator = std::min_element(dataval.begin(), dataval.end());

    // what is this ?
    float time_min, time_max, time_average;
    time_min = dataval[std::distance(dataval.begin(), time_min_iterator)];
    time_max = dataval[std::distance(dataval.begin(), time_max_iterator)];
    time_average = (time_max + time_min) / (float) 2;
    float pre_halfDiff = time_average - time_min;
    float next_halfDiff = time_max - time_average;

    for (auto i = 0; i < dataval.size(); ++i) {
      vec4f color;
      float temp_time = dataval[i];
      if (temp_time <= time_average) {
	color.x = color0.x + ((color1.x - color0.x) / pre_halfDiff) * (temp_time - time_min);
	color.y = color0.y + ((color1.y - color0.y) / pre_halfDiff) * (temp_time - time_min);
	color.z = color0.z + ((color1.z - color0.z) / pre_halfDiff) * (temp_time - time_min);
	//color.w = 1.0;
	color.w = color0.w + ((color1.w - color0.w) / pre_halfDiff) * (temp_time - time_min);
      } else {
	color.x = color1.x - ((color1.x - color2.x) / next_halfDiff) * (temp_time - time_average);
	color.y = color1.y - ((color1.y - color2.y) / next_halfDiff) * (temp_time - time_average);
	color.z = color1.z - ((color1.z - color2.z) / next_halfDiff) * (temp_time - time_average);
	//color.w = 1.0;
	color.w = color1.w + ((color2.w - color1.w) / next_halfDiff) * (temp_time - time_average);

      }
      outputDataList[fileIdx[i]].color.push_back(color);
    }

  }

  std::cout << std::endl;
  std::cout << "start writing swc ..." << std::endl;    
  for (int ii = 0; ii < (num_lines + split_threshold - 1) / split_threshold; ++ii) {
    // WRITE TO OUTPUT
    writeToOutput(outputDataList[ii], 
		  (file_out + "-" + 
		   std::to_string(ii) + "-" +
		   dataStr[dataIdx] + 
		   ".swc").c_str()); 
  }
  std::cout << "done writing swc ..." << std::endl; 
  std::cout << std::endl;

  return 0;
}
