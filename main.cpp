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

#include "helper.h"

// GLOBAL VARIABLES
static const std::string dataStr[] =  {
  "time",
  "magnititude", 
  "derivative" , 
  "difference" , 
  "vorticity"
};

static std::vector<vec4f> colorList = {  // transfer functions
  vec4f{0.0, 0.0, 1.0, 0.0},
  vec4f{0.86500299999999997, 0.86500299999999997, 0.86500299999999997, 0.5},
  vec4f{1.0, 0.0, 0.0, 1.0}
};

static size_t split_threshold = 4000;
static size_t dataIdx = 0;               // which feature to choose (0 -> time)
static std::vector<size_t>  fileIdx;     // the file id of each point
static std::vector<swcFile> outputDataList;
static bool write_velocity = false;

static std::vector<float> dataval;
static std::vector<float> velocity;

static const size_t histw = 1024;
static const size_t histh = 256;
static std::array<size_t, histw> hist = {size_t{0}};
static std::array<uint32_t, histw * histh> fb = {uint32_t{0}};

inline void time2velocity(std::vector<float> &velocity, const swcFile& outputData)
{
  std::vector<int> lines; // starting points of each line
  for (size_t i = 0; i < outputData.preIndex.size(); ++i) {
    if (outputData.preIndex[i] == -1) {
      lines.push_back(outputData.index[i]);
    }
  }
  lines.push_back(outputData.index.size() + 1); // a hack
  std::cout << "[velocity] computing velocity: found " << lines.size() 
	    << " lines" << std::endl;
  for(size_t i = 0; i < lines.size() - 1; ++i) {
    int start = lines[i];
    int end   = lines[i + 1] - 1;
    for (int j = start; j < end - 1; j++) {
      float x = outputData.position[j].x;
      float x_next = outputData.position[j + 1].x;
      float y = outputData.position[j].y;
      float y_next = outputData.position[j + 1].y;
      float z = outputData.position[j].z;
      float z_next = outputData.position[j + 1].z;
      float dis = sqrtf((x - x_next) * (x - x_next) + 
			(y - y_next) * (y - y_next) +
			(z - z_next) * (z - z_next));
      velocity.push_back(dis);
    }
    velocity.push_back(velocity.back()); // a hack
  }
}

int main(int argc, const char** argv) 
{  
  // arguments
  std::vector<std::string> inputFiles;
  for (int i = 1; i < argc; ++i) {
    std::string str(argv[i]);
    if (str == "-idx") {
      dataIdx = std::max(0, std::atoi(argv[++i]));
    }
    else if (str == "-velocity") {
      write_velocity = true;
    }
    else if (str == "-split") {
      split_threshold = std::max(0, std::atoi(argv[++i]));
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
  split_threshold = split_threshold > 1 ? split_threshold : num_lines;
  size_t num_files = (num_lines + split_threshold - 1) / split_threshold;

  // debug
  PrintBuddha(std::cout);
  std::cout << std::endl
	    << "raw2swc (佛祖保佑无BUG)" << std::endl << std::endl
	    << "[info] parsing: " << file_in << std::endl
	    << "[info] compute velocity: " 
	    << (write_velocity ? "yes" : "no") << std::endl
	    << "[info] split threshold    = " << split_threshold << std::endl
	    << "[info] feature index      = " << dataIdx      << std::endl
	    << "[info] number of features = " << num_features << std::endl
	    << "[info] number of lines    = " << num_lines    << std::endl
	    << "[info] number of files    = " << num_files    << std::endl
	    << std::endl;

  // resize
  std::cout << "[i/o] start reading files" << std::endl;
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
	dataval.push_back(comps[dataIdx]);
      }
    }
  }
  std::cout << "[i/o] done reading files" << std::endl << std::endl;

  // compute velocity
  if (write_velocity) {
    // convert feature to velocity
    std::cout << "[velocity] start compute velocity" << std::endl;
    for (auto f = 0; f < num_files; ++f) {
      time2velocity(velocity, outputDataList[f]);
    }
    std::cout << "[velocity] done compute velocity: " 
	      << velocity.size() << " velocity found" << std::endl;
    std::cout << std::endl;
  }
  std::vector<float>& data = write_velocity ? velocity : dataval;

  // find the max and the min
  const float tmin = *std::max_element(data.begin(), data.end());
  const float tmax = *std::min_element(data.begin(), data.end());
  const float trdis = 1.0f / (tmax - tmin);

  // generate histogram && convert feature to color   
  for (size_t i = 0; i < data.size(); ++i) {
    // get value
    float t = data[i];
    const float ratio = (t - tmin) * trdis;

    // find the histogram index
    {
      const int N = std::round(ratio * (hist.size() - 1));
      if (N < 0) {
	++hist.front();
      }	else if (N > hist.size()-1) {
	++hist.back();
      } else {
	++hist[N];
      }      
    }

    // find the TFN index and the fraction
    {      
      const int    N = int(ratio * (colorList.size()-1));
      const float  R = ratio * (colorList.size()-1) - N;
      vec4f color;
      if (N < 0) {
	color = colorList.front();
      }	else if (N >= colorList.size()-1) {
	color = colorList.back();
      } else {
	color = lerp(colorList[N], colorList[N+1], R);
      }
      outputDataList[fileIdx[i]].color.push_back(color);
    }
  }

  // WRITE TO HISTOGRAM
  const auto histcount = *std::max_element(hist.begin(), hist.end());
  for (auto i = 0; i < histw; ++i) {
    const float ratio = (colorList.size()-1) * ((float)i / (histw-1));
    const size_t N = int(ratio);
    const float  R = ratio - N;
    vec4f color;
    if (N < 0) color = colorList.front();
    else if (N >= colorList.size() - 1) color = colorList.back();
    else {
      color = lerp(colorList[N], colorList[N+1], R);
    }
    const auto y = (std::pow((float)hist[i] / histcount, 0.5f)) * histh;
    const auto a = color.w  * histh;
    std::cout << y << std::endl;
    for (auto j = 0; j < histh; ++j) {
      const auto idx = i + j * histw;
      if (j < y) {
      	vec4f c = lerp(vec4f{1.f,1.f,1.f,1.f}, color, 0.1f);
      	uint8_t *d = (uint8_t*) &(fb[idx]);
      	d[0] = uint8_t(std::round(c.x * 255));
      	d[1] = uint8_t(std::round(c.y * 255));
      	d[2] = uint8_t(std::round(c.z * 255));
      	d[3] = uint8_t(std::round(c.w * 255));
      }
      else if (j < a) {
      	vec4f c = lerp(vec4f{0.5f,0.5f,0.5f,1.f}, color, 0.5f);
      	uint8_t *d = (uint8_t*) &(fb[idx]);
      	d[0] = uint8_t(std::round(c.x * 255));
      	d[1] = uint8_t(std::round(c.y * 255));
      	d[2] = uint8_t(std::round(c.z * 255));
      	d[3] = uint8_t(std::round(c.w * 255));
      }
      else {
	uint8_t *d = (uint8_t*) &(fb[idx]);
	d[0] = uint8_t(std::round(color.x * 255));
	d[1] = uint8_t(std::round(color.y * 255));
	d[2] = uint8_t(std::round(color.z * 255));
	d[3] = uint8_t(std::round(color.w * 255));
      }
    }
  }
  writePPM("hist.ppm", histw, histh, fb.data());
  return 0;
  // WRITE TO OUTPUT
  std::cout << "[i/o] start writing swc ..." << std::endl;    
  for (auto f = 0; f < num_files; ++f) {
    std::string fstr = num_files == 1 ? "" : "-" + std::to_string(f);
    std::string dstr = (write_velocity ? "velocity" : dataStr[dataIdx]);
    outputDataList[f].writeToOutput((file_out + fstr + 
				     "-" + dstr + ".swc").c_str()); 
  }
  std::cout << "[i/o] done writing swc ..." << std::endl; 
  std::cout << std::endl;

  return 0;
}
