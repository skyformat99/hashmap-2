#include <iostream>
using namespace std;

#include "myhashmap.hpp"
using namespace myutils;

/* ------------------------------------------------------------------------- */

class NumHashMap : public MyHashMap<int, int, MyPthreadSpinLock> {

    public:

        NumHashMap()
            : MyHashMap<int, int, MyPthreadSpinLock>(11)
        {}

    protected:

        unsigned long hash(const int& key) const { return key; }
        bool equal(const int& a, const int& b) const { return (a == b); }
};

class TestIterator : public NumHashMap::Iterator {

    public:

        bool process(const int& key, int& value)
        {
            cout << "get " << key << ":" << value << endl;
            return true;
        }
};

/* ------------------------------------------------------------------------- */

int main(void)
{
    int key, value = 0;
    NumHashMap m;

    for (key = 0; key < 10; ++key)
        m.insert(key, key);

    key = 5;
    bool state = m.lookup(key, &value);
    if (!state)
        cout << "cannot find -> " << key << endl;
    else {
        cout << "find -> " << value << endl;
        m.insert(key, key + key, true);
        m.lookup(key, &value);
        cout << "find -> " << value << endl;
    }

    TestIterator iter;
    m.foreach(iter);

    return 0;
}
