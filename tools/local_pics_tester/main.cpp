#include <algorithm>
#include <iterator>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

#include "Client.hpp"

cv::Mat getDescriptors(const cv::Mat& image) {
   cv::ORB orb;
   std::vector<cv::KeyPoint> keyPoints;
   cv::Mat descriptors;
   orb(image, cv::noArray(), keyPoints, descriptors);

   return descriptors;
}


int main(int argc, char* argv[]) {

   if (argc < 2) {
      // missing required arguments
      return 1;
   }

   std::string pictPath = argv[1];

   auto image = cv::imread(pictPath, CV_LOAD_IMAGE_GRAYSCALE);

   if (image.empty()) {
      // invalid image
      return 2;
   }

   auto des = getDescriptors(image);

   Client c;

   std::vector<std::string> inputDesc;
   inputDesc.reserve(des.rows);

   for (auto i = 0; i != des.rows; ++i) {
      inputDesc.emplace_back(des.ptr<const char>(i), 32);
   }

   auto result = std::get<0>(c.search(inputDesc));
    
   std::copy(result.begin(), result.end(), std::ostream_iterator<std::string>(std::cout, " "));

   return 0;
}
