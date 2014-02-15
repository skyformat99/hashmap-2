#ifndef __MYHASHMAP_HPP__
#define __MYHASHMAP_HPP__

#include <list>
#include <vector>
#include <utility> // std::pair
#include <memory> // std::shared_ptr

using std::list;
using std::vector;
using std::pair;
using std::shared_ptr;

#define MAX_NR_SLOT 999983

namespace myutils {

template<typename KeyType, typename ValuePtrType>
class MyHashMap {

    public:

        class ForeachObject {

            public:

                virtual bool process(const KeyType&, shared_ptr<ValuePtrType>&) = 0;
        };

    public:

        MyHashMap(unsigned long slots = 0)
        {
            m_slots = (slots > 0) ? slots : MAX_NR_SLOT;

            m_table.resize(m_slots);
            for (auto i = m_table.begin(); i != m_table.end(); ++i)
                pthread_rwlock_init(&i->lock, nullptr);
        }

        virtual ~MyHashMap()
        {
            for (auto i = m_table.begin(); i != m_table.end(); ++i)
                pthread_rwlock_destroy(&i->lock);
        }

        bool insert(const KeyType& key, const shared_ptr<ValuePtrType>& value,
                    bool replace = false)
        {
            bool state;
            unsigned long slot = hash(key) % m_slots;

            pthread_rwlock_wrlock(&m_table[slot].lock);
            state = doInsert(key, value, slot, replace);
            pthread_rwlock_unlock(&m_table[slot].lock);

            return state;
        }

        void remove(const KeyType& key)
        {
            unsigned long slot = hash(key) % m_slots;

            pthread_rwlock_wrlock(&m_table[slot].lock);
            doRemove(key, slot);
            pthread_rwlock_unlock(&m_table[slot].lock);
        }

        shared_ptr<ValuePtrType> lookup(const KeyType& key)
        {
            unsigned long slot = hash(key) % m_slots;

            pthread_rwlock_rdlock(&m_table[slot].lock);
            auto ret = doLookup(key, slot);
            pthread_rwlock_unlock(&m_table[slot].lock);

            return ret;
        }

        bool foreach(const shared_ptr<ForeachObject>& o)
        {
            for (auto i = m_table.begin(); i != m_table.end(); ++i) {
                pthread_rwlock_rdlock(&i->lock);

                for (auto j = i->itemlist.begin(); j != i->itemlist.end(); ++j) {
                    if (o->process(j->first, j->second) == false) {
                        pthread_rwlock_unlock(&i->lock);
                        return false;
                    }
                }

                pthread_rwlock_unlock(&i->lock);
            }

            return true;
        }

    protected:

        virtual unsigned long hash(const KeyType& key) const = 0;

        virtual bool equal(const KeyType& a, const KeyType& b) const
        {
            return (a == b);
        }

    private:

        bool doInsert(const KeyType& key, const shared_ptr<ValuePtrType>& value,
                      unsigned long slot, bool replace)
        {
            list<pair<KeyType, shared_ptr<ValuePtrType>>>& itemlist = m_table[slot].itemlist;

            for (auto i = itemlist.begin(); i != itemlist.end(); ++i) {
                if (equal(key, i->first)) {
                    if (!replace)
                        return false;

                    i->second = value;
                    return true;
                }
            }

            itemlist.push_back(pair<KeyType, shared_ptr<ValuePtrType>>(key, value));
            return true;
        }

        void doRemove(const KeyType& key, unsigned long slot)
        {
            list<pair<KeyType, shared_ptr<ValuePtrType>>>& itemlist = m_table[slot].itemlist;

            for (auto i = itemlist.begin(); i != itemlist.end(); ++i) {
                if (equal(key, i->first)) {
                    itemlist.erase(i);
                    return;
                }
            }
        }

        shared_ptr<ValuePtrType> doLookup(const KeyType& key, unsigned long slot)
        {
            list<pair<KeyType, shared_ptr<ValuePtrType>>>& itemlist = m_table[slot].itemlist;

            for (auto i = itemlist.begin(); i != itemlist.end(); ++i) {
                if (equal(key, i->first))
                    return i->second;
            }

            return nullptr;
        }

    private:

        struct HashHead {
            pthread_rwlock_t lock;
            list<pair<KeyType, shared_ptr<ValuePtrType>>> itemlist;
        };

        unsigned long m_slots;
        vector<HashHead> m_table;
};

}

#endif
