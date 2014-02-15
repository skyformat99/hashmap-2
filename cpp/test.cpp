#include <iostream>
using namespace std;

#include "myhashmap.hpp"
#include <mystring.hpp>

using myutils::MyHashMap;
using myutils::MyString;

/* ------------------------------------------------------------------------- */

template<typename NumKeyType, typename ValuePtrType>
class NumHashMap : public MyHashMap<NumKeyType, ValuePtrType> {

    public:

        NumHashMap(unsigned long slots = 0) : MyHashMap<NumKeyType, ValuePtrType>(slots) {}

    protected:

        unsigned long hash(const NumKeyType& key) const { return key; }
};

template<typename KeyType, typename ValuePtrType>
class TestForeachObject : public MyHashMap<KeyType, ValuePtrType>::ForeachObject {

    public:

        bool process(const KeyType& key, shared_ptr<ValuePtrType>& value)
        {
            cout << "get " << key << ":" << *value << endl;
            return true;
        }
};

/* ------------------------------------------------------------------------- */

int main(void)
{
    int key;
    NumHashMap<int, string> m;

    for (key = 0; key < 10; ++key)
        m.insert(key, shared_ptr<string>(new MyString(key)));

    key = 5;
    auto res = m.lookup(key);
    if (!res)
        cout << "cannot find -> " << key << endl;
    else {
        cout << "find -> " << *res << endl;
        m.insert(key, shared_ptr<string>(new MyString(key + key)), true);
        res = m.lookup(key);
        cout << "find -> " << *res << endl;
    }

    m.foreach(shared_ptr<MyHashMap<int, string>::ForeachObject>(new TestForeachObject<int, string>()));

    return 0;
}
