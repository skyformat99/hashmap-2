#ifndef __MYHASHMAP_HPP__
#define __MYHASHMAP_HPP__

#include <list>
#include <vector>
#include <utility> // std::std::pair
#include <pthread.h>

#define MAX_NR_SLOT 999983

namespace MyUtils {

template<typename KeyType, typename ValueType>
class MyHashMap {

    public:

        class Iterator {

            public:

                virtual bool process(const KeyType&, ValueType&) = 0;
        };

    public:

        MyHashMap(unsigned int slots = 0)
        {
            m_slots = (slots > 0) ? slots : MAX_NR_SLOT;

            m_table.resize(m_slots);
            for (auto i = m_table.begin(); i != m_table.end(); ++i)
                pthread_rwlock_init(&i->lock, nullptr);

            m_nr_item = 0;
        }

        virtual ~MyHashMap()
        {
            for (auto i = m_table.begin(); i != m_table.end(); ++i)
                pthread_rwlock_destroy(&i->lock);
        }

        unsigned long size() const { return m_nr_item; }

        bool insert(const KeyType& key, const ValueType& value,
                    bool replace = false)
        {
            unsigned int slot = hash(key) % m_slots;

            WRLock lock(m_table[slot].lock);
            return doInsert(key, value, slot, replace);
        }

        void remove(const KeyType& key)
        {
            unsigned int slot = hash(key) % m_slots;

            WRLock lock(m_table[slot].lock);
            doRemove(key, slot);
        }

        ValueType lookup(const KeyType& key)
        {
            unsigned int slot = hash(key) % m_slots;

            RDLock lock(m_table[slot].lock);
            return doLookup(key, slot);
        }

        bool foreach(Iterator& o)
        {
            for (auto i = m_table.begin(); i != m_table.end(); ++i) {
                RDLock lock(i->lock);
                for (auto j = i->itemlist.begin(); j != i->itemlist.end(); ++j) {
                    if (!o.process(j->first, j->second))
                        return false;
                }
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

        bool doInsert(const KeyType& key, const ValueType& value,
                      unsigned long slot, bool replace)
        {
            std::list<std::pair<KeyType, ValueType>>& itemlist = m_table[slot].itemlist;

            for (auto i = itemlist.begin(); i != itemlist.end(); ++i) {
                if (equal(key, i->first)) {
                    if (!replace)
                        return false;

                    i->second = value;
                    return true;
                }
            }

            itemlist.push_front(std::pair<KeyType, ValueType>(key, value));
            __sync_add_and_fetch(&m_nr_item, 1);
            return true;
        }

        void doRemove(const KeyType& key, unsigned long slot)
        {
            std::list<std::pair<KeyType, ValueType>>& itemlist = m_table[slot].itemlist;

            for (auto i = itemlist.begin(); i != itemlist.end(); ++i) {
                if (equal(key, i->first)) {
                    itemlist.erase(i);
                    __sync_sub_and_fetch(&m_nr_item, 1);
                    return;
                }
            }
        }

        ValueType doLookup(const KeyType& key, unsigned long slot)
        {
            std::list<std::pair<KeyType, ValueType>>& itemlist = m_table[slot].itemlist;

            for (auto i = itemlist.begin(); i != itemlist.end(); ++i) {
                if (equal(key, i->first))
                    return i->second;
            }

            return ValueType();
        }

    private:

        class RDLock {

            public:

                RDLock(pthread_rwlock_t& lock_)
                    : m_lock(lock_)
                {
                    pthread_rwlock_rdlock(&lock_);
                }

                ~RDLock()
                {
                    pthread_rwlock_unlock(&m_lock);
                }

            private:

                pthread_rwlock_t& m_lock;
        };

        class WRLock {

            public:

                WRLock(pthread_rwlock_t& lock_)
                    : m_lock(lock_)
                {
                    pthread_rwlock_wrlock(&lock_);
                }

                ~WRLock()
                {
                    pthread_rwlock_unlock(&m_lock);
                }

            private:

                pthread_rwlock_t& m_lock;
        };

        struct HashHead {
            pthread_rwlock_t lock;
            std::list<std::pair<KeyType, ValueType>> itemlist;
        };

    private:

        unsigned long m_nr_item;
        unsigned int m_slots;
        std::vector<HashHead> m_table;
};

}

#endif
