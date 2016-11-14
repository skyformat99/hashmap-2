#include <iostream>
using namespace std;

#include "hashmap.hpp"
using namespace utils;

/* ------------------------------------------------------------------------- */

class NumHashMap : public HashMap<int, int, PthreadSpinLock> {

    public:

        NumHashMap()
            : HashMap<int, int, PthreadSpinLock>(7)
        {}

    protected:

        unsigned long hash(const int& key) const { return key; }
        bool equal(const int& a, const int& b) const { return (a == b); }
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


    auto processor = [] (unsigned int slot,
                         list<pair<const int, int>>& itemlist) -> bool {
        cout << slot << " ->";
        for (auto x = itemlist.begin(); x != itemlist.end(); ++x)
            cout << " (" << x->first << ", " << x->second << ")";
        cout << endl;

        return true;
    };
    m.foreach(processor);

    return 0;
}
