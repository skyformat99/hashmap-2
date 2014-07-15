#include <iostream>
using namespace std;

#include "myhashmap.hpp"
using namespace myutils;

/* ------------------------------------------------------------------------- */

template<typename NumKeyType, typename ValueType>
class NumHashMap : public MyHashMap<NumKeyType, ValueType> {

    protected:

        unsigned long hash(const NumKeyType& key) const { return key; }
        bool equal(const NumKeyType& a, const NumKeyType& b) const { return (a == b); }
};

template<typename KeyType, typename ValueType>
class TestIterator : public MyHashMap<KeyType, ValueType>::Iterator {

    public:

        bool process(const KeyType& key, ValueType& value)
        {
            cout << "get " << key << ":" << value << endl;
            return true;
        }
};

/* ------------------------------------------------------------------------- */

int main(void)
{
    int key, value = 0;
    NumHashMap<int, int> m;

    for (key = 0; key < 10; ++key)
        m.insert(key, key);

    key = 5;
    bool state = m.lookup(key, value);
    if (!state)
        cout << "cannot find -> " << key << endl;
    else {
        cout << "find -> " << value << endl;
        m.insert(key, key + key, true);
        m.lookup(key, value);
        cout << "find -> " << value << endl;
    }

    TestIterator<int, int> iter;
    m.foreach(iter);

    return 0;
}
