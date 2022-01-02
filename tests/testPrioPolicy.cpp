#include <iostream>
#include <map>
#include <stdint.h>
#include <stdlib.h>
#include <vector>

using namespace std;

const uint32_t kPriorityCount = 101;
const uint32_t kNumProc = 4;

class HeaterPriorityQueue;

class Heater {
  static HeaterPriorityQueue sPriorityQueue;
  static vector<Heater *> sHeaters;

  uint32_t mNum;

public:
  uint32_t mPriority;
  Heater() {
    mNum = sHeaters.size();
    sHeaters.push_back(this);
  }
  void changePriority(const uint32_t inNewPriority);
  static void startScheduling();
  static const HeaterPriorityQueue &priorityQueue() { return sPriorityQueue; }
  static void roundRobin(const uint32_t kNumProc);
  static void adjustPriorities() {
    for (auto it = sHeaters.begin(); it != sHeaters.end(); it++) {
      uint32_t delta = rand() % 3 - 1;
      int32_t priority = (*it)->mPriority;
      priority += delta;
      if (priority < 0)
        priority = 0;
      if (priority >= kPriorityCount)
        priority = kPriorityCount - 1;
      (*it)->changePriority((uint32_t)priority);
    }
  }

  uint32_t num() const { return mNum; }
};

class HeaterPrioritySlot {
  vector<Heater *> mHeaters;

public:
  HeaterPrioritySlot() {}
  bool isEmpty() { return mHeaters.size() == 0; }
  void addHeater(Heater *const inHeater) { mHeaters.push_back(inHeater); }
  void cleanUp() { mHeaters.clear(); }
  void removeHeater(Heater *const inHeater) {
    for (auto it = mHeaters.begin(); it != mHeaters.end(); it++) {
      if (*it == inHeater) {
        mHeaters.erase(it);
        break;
      }
    }
  }
  void roundRobin() {
    if (mHeaters.size() > 1) {
      Heater *const first = *(mHeaters.begin());
      mHeaters.erase(mHeaters.begin());
      mHeaters.push_back(first);
    }
  }
  uint32_t size() const { return mHeaters.size(); }
  friend ostream &operator<<(ostream &, const HeaterPrioritySlot &);
};

ostream &operator<<(ostream &s, const HeaterPrioritySlot &hps) {
  bool first = true;
  for (auto it = hps.mHeaters.begin(); it != hps.mHeaters.end(); it++) {
    if (!first) {
      s << ", ";
    }
    s << (*it)->num();
    first = false;
  }
  s << endl;
  return s;
}

class HeaterPriorityQueue {
  map<uint32_t, HeaterPrioritySlot *> mQueueEntries;

public:
  HeaterPriorityQueue() {}
  void addHeater(Heater *const inHeater) {
    auto slot = mQueueEntries.find(inHeater->mPriority);
    if (slot != mQueueEntries.end()) {
      (slot->second)->addHeater(inHeater);
    } else {
      auto newSlot = new HeaterPrioritySlot;
      newSlot->addHeater(inHeater);
      mQueueEntries.insert({inHeater->mPriority, newSlot});
    }
  }
  void updateHeater(Heater *const inHeater, const uint32_t inNewPriority) {
    auto oldSlot = mQueueEntries.find(inHeater->mPriority);
    if (oldSlot != mQueueEntries.end()) {
      (oldSlot->second)->removeHeater(inHeater);
      if ((oldSlot->second)->size() == 0) {
        mQueueEntries.erase(oldSlot);
      }
    }
    auto newSlot = mQueueEntries.find(inNewPriority);
    if (newSlot != mQueueEntries.end()) {
      (newSlot->second)->addHeater(inHeater);
    } else {
      auto slot = new HeaterPrioritySlot;
      slot->addHeater(inHeater);
      mQueueEntries.insert({inNewPriority, slot});
    }
  }
  void roundRobin(const uint32_t inNumProc) {
    uint32_t heaterCount = 0;
    for (auto it = mQueueEntries.rbegin(); it != mQueueEntries.rend(); it++) {
      heaterCount += (it->second)->size();
      if (heaterCount >= inNumProc) {
        (it->second)->roundRobin();
        break;
      }
    }
  }
  friend ostream &operator<<(ostream &, const HeaterPriorityQueue &);
};

ostream &operator<<(ostream &s, const HeaterPriorityQueue &q) {
  for (auto it = q.mQueueEntries.rbegin(); it != q.mQueueEntries.rend(); it++) {
    s << '[' << it->first << "](" << (it->second)->size() << ") "
      << *(it->second);
  }
  return s;
}

vector<Heater *> Heater::sHeaters;
HeaterPriorityQueue Heater::sPriorityQueue;

void Heater::changePriority(const uint32_t inNewPriority) {
  if (inNewPriority != mPriority) {
    sPriorityQueue.updateHeater(this, inNewPriority);
    mPriority = inNewPriority;
  }
}

void Heater::startScheduling() {
  for (auto it = sHeaters.begin(); it != sHeaters.end(); it++) {
    sPriorityQueue.addHeater(*it);
  }
}

void Heater::roundRobin(const uint32_t kNumProc) {
  sPriorityQueue.roundRobin(kNumProc);
}

int main() {
  Heater *h;
  for (int i = 0; i < 20; i++) {
    h = new Heater;
    h->mPriority = kPriorityCount - 1;
  }
  Heater::startScheduling();
  cout << Heater::priorityQueue();

  for (int i = 0; i < 1000; i++) {
    Heater::adjustPriorities();
    Heater::roundRobin(kNumProc);
    cout << string(40, '=') << endl;
    cout << Heater::priorityQueue();
    cout << string(40, '-') << endl;
  }

  return 0;
}