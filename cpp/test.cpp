#include <iostream>
using namespace std;

#include "myhashmap.hpp"
#include <mystring.hpp>

using MyUtils::MyHashMap;
using MyUtils::MyString;

/* ------------------------------------------------------------------------- */

template<typename NumKeyType, typename ValueType>
class NumHashMap : public MyHashMap<NumKeyType, ValueType> {

    protected:

        unsigned long hash(const NumKeyType& key) const { return key; }
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
    int key;
    NumHashMap<int, string> m;

    for (key = 0; key < 10; ++key)
        m.insert(key, MyString(key));

    key = 5;
    auto res = m.lookup(key);
    if (res.empty())
        cout << "cannot find -> " << key << endl;
    else {
        cout << "find -> " << res << endl;
        m.insert(key, MyString(key + key), true);
        res = m.lookup(key);
        cout << "find -> " << res << endl;
    }

    TestIterator<int, string> iter;
    m.foreach(iter);

    return 0;
}
