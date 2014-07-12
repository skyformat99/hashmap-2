#ifndef __MYHASHMAP_HPP__
#define __MYHASHMAP_HPP__

#include <list>
#include <vector>
#include <utility> // std::pair

#define MAX_NR_SLOT 999983

namespace myutils {

class MyHashLock {

    public:

        virtual void rd_lock() = 0;
        virtual void wr_lock() = 0;
        virtual void rd_unlock() = 0;
        virtual void wr_unlock() = 0;
};

class MyNullHashLock : public MyHashLock {

    public:

        void rd_lock() {}
        void wr_lock() {}
        void rd_unlock() {}
        void wr_unlock() {}
};

template<typename KeyType, typename ValueType, typename LockType = MyNullHashLock>
class MyHashMap {

    public:

        class Iterator {

            public:

                virtual bool process(const KeyType&, ValueType&) = 0;
        };

        class SlotIterator {

            public:

                virtual bool process(unsigned int slot,
                                     const std::list<std::pair<KeyType, ValueType>>& itemlist) = 0;
        };

    public:

        MyHashMap(unsigned int slots = 0)
        {
            m_items = 0;
            m_slots = (slots > 0) ? slots : MAX_NR_SLOT;

            m_table.resize(m_slots);
        }

        virtual ~MyHashMap() {}

        unsigned long size() const { return m_items; }

        unsigned int slots() const { return m_slots; }

        bool insert(const KeyType& key, const ValueType& value,
                    bool replace = false)
        {
            unsigned int slot = hash(key) % m_slots;

            WRLock lock(m_table[slot].lock);
            return doInsert(slot, key, value, replace);
        }

        void remove(const KeyType& key)
        {
            unsigned int slot = hash(key) % m_slots;

            WRLock lock(m_table[slot].lock);
            doRemove(slot, key);
        }

        bool lookup(const KeyType& key, ValueType& value)
        {
            unsigned int slot = hash(key) % m_slots;

            RDLock lock(m_table[slot].lock);
            return doLookup(slot, key, value);
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

        bool foreach(SlotIterator& o)
        {
            for (unsigned int i = 0; i < m_table.size(); ++i) {
                HashHead& head = m_table[i];

                RDLock lock(head.lock);
                if (!o.process(i, head.itemlist))
                    return false;
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

        bool doInsert(unsigned long slot, const KeyType& key, const ValueType& value,
                      bool replace)
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
            __sync_add_and_fetch(&m_items, 1);
            return true;
        }

        void doRemove(unsigned long slot, const KeyType& key)
        {
            std::list<std::pair<KeyType, ValueType>>& itemlist = m_table[slot].itemlist;

            for (auto i = itemlist.begin(); i != itemlist.end(); ++i) {
                if (equal(key, i->first)) {
                    itemlist.erase(i);
                    __sync_sub_and_fetch(&m_items, 1);
                    return;
                }
            }
        }

        ValueType doLookup(unsigned long slot, const KeyType& key, ValueType& value)
        {
            std::list<std::pair<KeyType, ValueType>>& itemlist = m_table[slot].itemlist;

            for (auto i = itemlist.begin(); i != itemlist.end(); ++i) {
                if (equal(key, i->first)) {
                    value = i->second;
                    return true;
                }
            }

            return false;
        }

    private:

        class RDLock {

            public:

                RDLock(LockType& lock_)
                    : m_lock(lock_)
                {
                    m_lock.rd_lock();
                }

                ~RDLock()
                {
                    m_lock.rd_unlock();
                }

            private:

                LockType& m_lock;
        };

        class WRLock {

            public:

                WRLock(LockType& lock_)
                    : m_lock(lock_)
                {
                    m_lock.wr_lock();
                }

                ~WRLock()
                {
                    m_lock.wr_unlock();
                }

            private:

                LockType& m_lock;
        };

    private:

        struct HashHead {
            LockType lock;
            std::list<std::pair<KeyType, ValueType>> itemlist;
        };

    private:

        unsigned long m_items;
        unsigned int m_slots;
        std::vector<HashHead> m_table;
};

}

#endif
