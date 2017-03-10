#include "BTreePage.h"
#include <cassert>
using namespace std;

BTreePage::BTreePage() {
  this->parent = nullptr;
}

void BTreePage::addChild(BTreePage* c) {
  assert(children.size() < fan_out);
  children.push_back(c);
}

BTreePage::~BTreePage() {

}

void BTreePage::setParent(BTreePage* p) {
  this->parent = p;
}

void BTreePage::addKey(DataEntry key) {
  assert(keys.size() < MAX_KEY_PER_PAGE);
  keys.push_back(key);
}
bool BTreePage::isFull() {
  return keys.size() == MAX_KEY_PER_PAGE;
}



