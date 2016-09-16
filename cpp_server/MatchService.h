#ifndef MATCH_SERVICE_H
#define MATCH_SERVICE_H

#include "BKTree.h"
#include "HammingDistancePolicy.h"
#include "CachePolicy.h"

#include <map>
#include <chrono>
#include <iostream>
#include <unordered_map>

class CacheEntryContainer {
private:
  std::map<std::uint32_t, std::string> &_entry;
  std::vector<std::uint32_t>::const_iterator _it;

public:
  CacheEntryContainer(std::map<std::uint32_t, std::string> &entry, std::vector<std::uint32_t>::const_iterator begin)
  :_entry(entry)
  , _it(begin)
  {}
  
public:
  void emplace_back(std::string::const_iterator begin, std::string::const_iterator end) {
    _entry[*_it++] = std::string(begin, end);
  }
};

class ChildrenKeysCacheImpl : public ChildrenKeysCache {
private:
  std::unordered_map<std::string, std::map<std::uint32_t, std::string>> _cache;

public:
  bool get(const std::string& key, std::queue<std::string>& keys, const std::pair<std::uint32_t, std::uint32_t>& range) {
    auto found = _cache.find(key);
    if (found != _cache.end()) {
      // hit
      auto lowerBound = found->second.lower_bound(range.first);
      auto upperBound = found->second.upper_bound(range.second);

      for (; lowerBound != upperBound; ++lowerBound) {
        keys.push(lowerBound->second);
      }

      return true;
    }

    return false;
  }

  template<typename Callable, typename LoadAllCallable>
  void update(const std::string& key, const std::vector<std::uint32_t>& distances, Callable&& loadChildKey, LoadAllCallable&& loadAll) {
    std::map<std::uint32_t, std::string> cacheEntry;
    CacheEntryContainer container {cacheEntry, distances.begin()};

    loadAll(key, container);

    _cache[key] = std::move(cacheEntry);
  }
};

struct ChildrenKeyPolicyImpl {
  static void insert(std::string& keys, const std::string& newKey, std::uint32_t pos) {
    keys.insert(keys.cbegin() + pos * 32, newKey.cbegin(), newKey.cend());
  }

  template<typename Container>
  static void split(const std::string& keys, Container& splitKeys) {
    for (auto it = keys.cbegin(); it != keys.cend(); it += 32) {
      splitKeys.emplace_back(it, it + 32);
    }
  }
};

using MatchTree = std::shared_ptr<BKTree<HammingDistancePolicy, ChildrenKeysCacheImpl, ChildrenKeyPolicyImpl>>;

class MatchService {
private:
  MatchTree _tree;

public:
  MatchService(const MatchTree& tree)
    : _tree { tree }
  {}

  MatchService(const MatchService& s)
    : _tree { s._tree }
  {}

public:
  template<typename InputIterator>
  std::vector<std::string> match(InputIterator begin, InputIterator end) {
    std::map<std::string, std::uint32_t> counter;

    auto t1 = std::chrono::high_resolution_clock::now();

    for (; begin != end; ++begin) {
      auto&& q = _tree->query(*begin, 400, 1, 20);
      for (auto e : q) {
        counter[e] += 1;
      }
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "elapsed: " << std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count() << " s." << std::endl;

    return sort(counter);
  }

private:
  std::vector<std::string> sort(const std::map<std::string, std::uint32_t>& counter) {
    std::vector<std::pair<std::string, std::uint32_t>> entries {counter.begin(), counter.end()};

      std::sort(entries.begin(), entries.end(), [](const std::pair<std::string, std::uint32_t>& l, const std::pair<std::string, std::uint32_t>& r) {
      return l.second < l.second;
    });

    std::vector<std::string> result;
    for (auto e : entries) {
      std::cout << e.second << ":" << e.first << std::endl;
        
      result.push_back(e.first);
    }

    return result;
  }
};

#endif // MATCH_SERVICE_H