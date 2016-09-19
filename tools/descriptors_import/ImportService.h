#ifndef IMPORT_SERVICE_H
#define IMPORT_SERVICE_H

#include "BKTree.h"
#include "HammingDistancePolicy.h"
#include "CachePolicy.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

struct ChildrenKeyPolicyImpl {
  static void insert(std::string& keys, const std::string& newKey, std::uint32_t pos) {
    keys.insert(keys.begin() + pos * 32, newKey.cbegin(), newKey.cend());
  }

  template<typename Container>
  static void split(const std::string& keys, Container& splitKeys) {
    for (auto it = keys.cbegin(); it != keys.cend(); it += 32) {
      splitKeys.emplace_back(it, it + 32);
    }
  }
};

using MatchTree = std::shared_ptr<BKTree<HammingDistancePolicy, NoCachePolicy, ChildrenKeyPolicyImpl>>;

class ImportService {
private:
  MatchTree _tree;

public:
  ImportService(const MatchTree& tree)
    : _tree { tree }
  {}

  ImportService(const ImportService& s)
    : _tree { s._tree }
  {}

public:
  void put(const std::string& path, const std::string& value) {
    // read image
    auto image = cv::imread(path, CV_LOAD_IMAGE_GRAYSCALE);

    if (image.empty()) {
      return;
    }

    auto des = getDescriptors(image);

    for (auto i = 0; i != des.rows; ++i) {
      _tree->insert(std::string(des.ptr<const char>(i), des.cols), value);
    }
  }

private:
  cv::Mat getDescriptors(const cv::Mat& image) {
    cv::ORB orb;
    std::vector<cv::KeyPoint> keyPoints;
    cv::Mat descriptors;
    orb(image, cv::noArray(), keyPoints, descriptors);

    return descriptors;
  }
};


#endif // IMPORT_SERVICE_H
