#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <unordered_set>
#include <algorithm>
#include <iterator>
#include <tuple>

class SequenceMerger {
 public:
  SequenceMerger() = default;
  ~SequenceMerger() = default;

  SequenceMerger(const SequenceMerger &other_) = delete;
  SequenceMerger &operator=(const SequenceMerger &rhs_) = delete;

  void merge(const std::vector<std::string> &seq_);
  std::vector<std::string> getMergeSeq() const {

    std::vector<std::string> result(_mergedSeq);
    return result;
  }

 private:
  std::vector<std::string> _mergedSeq;
};

void
SequenceMerger::merge(const std::vector<std::string> &seq_) {
  assert(!seq_.empty());

  if (!_mergedSeq.empty()) {
    std::vector<std::string> inputSeq(seq_);

    std::unordered_set<std::string> longerSeqSet;
    std::unordered_set<std::string> smallerSeqSet;

    if (inputSeq.size() > _mergedSeq.size()) {
      _mergedSeq.swap(inputSeq);
    }

    longerSeqSet.insert(_mergedSeq.begin(), _mergedSeq.end());
    smallerSeqSet.insert(inputSeq.begin(), inputSeq.end());

    std::vector<std::string> mergedSeq;

    for (auto lIt = _mergedSeq.begin(), sIt = inputSeq.begin();
         lIt != _mergedSeq.end() && sIt != inputSeq.end();) {

      if (*lIt == *sIt) {
        mergedSeq.push_back(*lIt);
        longerSeqSet.erase(*lIt);
        smallerSeqSet.erase(*sIt);
        ++lIt, ++sIt;

      } else {
        assert(!(longerSeqSet.find(*sIt) != longerSeqSet.end() && smallerSeqSet.find(*lIt) != smallerSeqSet.end()));

        if (longerSeqSet.find(*sIt) != longerSeqSet.end()) {
          mergedSeq.push_back(*lIt);
          longerSeqSet.erase(*lIt);
          ++lIt;

        } else if (smallerSeqSet.find(*lIt) != smallerSeqSet.end()) {
          mergedSeq.push_back(*sIt);
          smallerSeqSet.erase(*sIt);
          ++sIt;

        } else {
          std::pair<std::string, std::string> values(*sIt, *lIt);
          if (values.first > values.second) {
            std::swap(values.first, values.second);
          }

          mergedSeq.push_back(values.first);
          mergedSeq.push_back(values.second);
          longerSeqSet.erase(*lIt);
          smallerSeqSet.erase(*sIt);
          ++lIt, ++sIt;
        }
      }
    }

    _mergedSeq.swap(mergedSeq);

  } else {
    _mergedSeq.insert(_mergedSeq.end(), seq_.begin(), seq_.end());
  }
}

int main() {
  SequenceMerger sequenceMerger;

  std::vector<std::string> seqA = {"A", "B", "F", "E"};
//  std::vector<std::string> seqB = {"A", "C", "G", "E"};
//  std::vector<std::string> seqB = {"A", "C", "G", "B", "E"};
  std::vector<std::string> seqB = {"A", "F", "G", "C", "E"};

  std::cout << "seqA: ";
  std::copy(seqA.begin(), seqA.end(), std::ostream_iterator<std::string>(std::cout, "->"));
  std::cout << std::endl;
  std::cout << "seqB: ";
  std::copy(seqB.begin(), seqB.end(), std::ostream_iterator<std::string>(std::cout, "->"));
  std::cout << std::endl;

  sequenceMerger.merge(seqA);
  sequenceMerger.merge(seqB);

  auto merged = sequenceMerger.getMergeSeq();
  std::cout << "merged: ";
  std::copy(merged.begin(), merged.end(), std::ostream_iterator<std::string>(std::cout, "->"));
  std::cout << std::endl;

  return 0;
}
