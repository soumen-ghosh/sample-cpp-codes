#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <unordered_set>
#include <algorithm>
#include <iterator>
#include <tuple>
#include <ranges>
#include <chrono>

/*
 * The purpose here is to merge 2 DAGs as per below example:
 * DAG1: A->B->F->E->
 * DAG2: A->F->G->C->E->
 * Expected O/P: A->B->F->G->C->E-> (here B should precede F;
 *                                   F should be followed by G, C and C should precede E)
 *
 * DAG1: A->B->F->E->
 * DAG2: A->C->G->E->
 * Expected O/P: A->B->C->F->G->E-> (A & E should be 1st and last element;
 *                                   B, C should precede F, G;
 *                                   B, C should follow lexical order;
 *                                   F, G should follow lexical order)
 *
 * DAG1: A->B->F->E->
 * DAG2: A->C->G->B->T->Q->R->E->
 * Expected O/P: A->C->G->B->F->T->Q->R->E (C & G should precede B;
 *                                          F should follow B and precede E)
 *
 */

inline void printVec(const std::vector<std::string> &seq_, std::ostream &strm_, const std::string &printIdentifier_) {
  strm_ << printIdentifier_;
  std::ranges::copy(seq_, std::ostream_iterator<std::string>(strm_, "->"));
//  std::copy(seq_.begin(), seq_.end(), std::ostream_iterator<std::string>(strm_, "->"));
  strm_ << std::endl;
}

class SequenceMerger {
 public:
  explicit SequenceMerger(bool matchLast_ = true)
      : _matchLast(matchLast_), _mergedSeq() {}
  ~SequenceMerger() = default;

  SequenceMerger(const SequenceMerger &other_) = delete;
  SequenceMerger &operator=(const SequenceMerger &rhs_) = delete;

  void merge(const std::vector<std::string> &seq_);

  [[nodiscard]] std::vector<std::string> getMergeSeq() const {

    std::vector<std::string> result(_mergedSeq);
    return result;
  }

  inline void reset(bool matchLast_ = true) {
    _matchLast = matchLast_;
    _mergedSeq.clear();
  }

 private:
  bool _matchLast;
  std::vector<std::string> _mergedSeq;
};

void
SequenceMerger::merge(const std::vector<std::string> &seq_) {
  assert(!seq_.empty());

  if (_matchLast && !_mergedSeq.empty()) {
    assert(*_mergedSeq.rbegin() == *seq_.rbegin());
  }

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

    auto lIt = _mergedSeq.begin(), sIt = inputSeq.begin();

    for (; lIt != _mergedSeq.end() && sIt != inputSeq.end();) {

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

    assert(sIt == inputSeq.end());
    if (!_matchLast) {
      if (lIt != _mergedSeq.end()) {
        mergedSeq.insert(mergedSeq.end(), lIt, _mergedSeq.end());
      }
    } else {
      assert(lIt == _mergedSeq.end());
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
//  std::vector<std::string> seqB = {"A", "F", "G", "C", "E", "Q", "R", "T"};
  std::vector<std::string> seqB = {"A", "C", "G", "B", "T", "Q", "R", "E"};
  printVec(seqA, std::cout, "seqA:");
  printVec(seqB, std::cout, "seqB:");

  sequenceMerger.merge(seqA);
  sequenceMerger.merge(seqB);

  auto merged = sequenceMerger.getMergeSeq();
  printVec(merged, std::cout, "merged:");

  assert(merged == std::vector<std::string>({"A", "C", "G", "B", "F", "T", "Q", "R", "E"}));

  std::vector<std::vector<std::string> > complexSeq1 = {
      {"A", "Q", "B", "F", "E"},
      {"A", "C", "T", "E"},
      {"A", "C", "G", "B", "T", "R", "E"}
  };

  sequenceMerger.reset();

  auto start = std::chrono::high_resolution_clock::now();
  std::sort(complexSeq1.begin(),
            complexSeq1.end(),
            [](std::vector<std::string> &v1, std::vector<std::string> &v2) -> bool {
              return v1.size() > v2.size();
            });

  for (auto &v : complexSeq1) {
    printVec(v, std::cout, "v:");

    sequenceMerger.merge(v);
  }

  merged = sequenceMerger.getMergeSeq();
  auto end = std::chrono::high_resolution_clock::now();

  printVec(merged, std::cout, "merged:");
  std::cout << "Elapsed time (ms): " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
            << std::endl;

  assert(merged == std::vector<std::string>({"A", "C", "Q", "G", "B", "F", "T", "R", "E"}));

  std::vector<std::vector<std::string> > complexSeq2 = {
      {"XYZ", "DEF", "PQR"},
      {"ABC", "XYZ", "IJK", "PQR"},
      {"XYZ", "IJK", "LMO", "PQR"}
  };

  sequenceMerger.reset();

  start = std::chrono::high_resolution_clock::now();
  std::sort(complexSeq2.begin(),
            complexSeq2.end(),
            [](std::vector<std::string> &v1, std::vector<std::string> &v2) -> bool {
              return v1.size() > v2.size();
            });

  for (auto &v : complexSeq2) {
    printVec(v, std::cout, "v:");

    sequenceMerger.merge(v);
  }

  merged = sequenceMerger.getMergeSeq();
  end = std::chrono::high_resolution_clock::now();

  printVec(merged, std::cout, "merged:");
  std::cout << "Elapsed time (ms): " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
            << std::endl;

  assert(merged == std::vector<std::string>({"ABC", "XYZ", "DEF", "IJK", "LMO", "PQR"}));

  return 0;
}
