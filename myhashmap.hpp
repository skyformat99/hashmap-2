#ifndef __MYHASHMAP_HPP__
#define __MYHASHMAP_HPP__

#include <list>
#include <vector>
#include <utility> // std::pair
#include <pthread.h>

#define MAX_NR_SLOT 999983

namespace myutils {

class MyHashLock {

    public:

        virtual void rd_lock() = 0;
        virtual void wr_lock() = 0;
        virtual void rd_unlock() = 0;
        virtual void wr_unlock() = 0;
};

class MyDummyHashLock : public MyHashLock {

    public:

        void rd_lock() {}
        void wr_lock() {}
        void rd_unlock() {}
        void wr_unlock() {}
};

class MyPthreadRWLock : public MyHashLock {

    public:

        MyPthreadRWLock()
        {
            pthread_rwlock_init(&m_lock, NULL);
        }

        ~MyPthreadRWLock()
        {
            pthread_rwlock_destroy(&m_lock);
        }

        void rd_lock()
        {
            pthread_rwlock_rdlock(&m_lock);
        }

        void wr_lock()
        {
            pthread_rwlock_wrlock(&m_lock);
        }

        void rd_unlock()
        {
            pthread_rwlock_unlock(&m_lock);
        }

        void wr_unlock()
        {
            pthread_rwlock_unlock(&m_lock);
        }

    private:

        pthread_rwlock_t m_lock;
};

class MyPthreadMutexLock : public MyHashLock {

    public:

        MyPthreadMutexLock()
        {
            pthread_mutex_init(&m_lock, NULL);
        }

        ~MyPthreadMutexLock()
        {
            pthread_mutex_destroy(&m_lock);
        }

        void rd_lock()
        {
            pthread_mutex_lock(&m_lock);
        }

        void wr_lock()
        {
            pthread_mutex_lock(&m_lock);
        }

        void rd_unlock()
        {
            pthread_mutex_unlock(&m_lock);
        }

        void wr_unlock()
        {
            pthread_mutex_unlock(&m_lock);
        }

    private:

        pthread_mutex_t m_lock;
};

class MyPthreadSpinLock : public MyHashLock {

    public:

        MyPthreadSpinLock()
        {
            pthread_spin_init(&m_lock, PTHREAD_PROCESS_PRIVATE);
        }

        ~MyPthreadSpinLock()
        {
            pthread_spin_destroy(&m_lock);
        }

        void rd_lock()
        {
            pthread_spin_lock(&m_lock);
        }

        void wr_lock()
        {
            pthread_spin_lock(&m_lock);
        }

        void rd_unlock()
        {
            pthread_spin_unlock(&m_lock);
        }

        void wr_unlock()
        {
            pthread_spin_unlock(&m_lock);
        }

    private:

        pthread_spinlock_t m_lock;
};

/* ------------------------------------------------------------------------- */

template<typename KeyType, typename ValueType, typename LockType = MyDummyHashLock>
class MyHashMap {

    public:

        class Iterator {

            public:

                virtual bool process(const KeyType&, ValueType&) = 0;
        };

        class SlotIterator {

            public:

                virtual bool process(unsigned int slot,
                                     std::list<std::pair<const KeyType, ValueType>>& itemlist) = 0;
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

            WRLock lock(&(m_table[slot].lock));
            return doInsert(slot, key, value, replace);
        }

        bool remove(const KeyType& key, ValueType* value = NULL)
        {
            unsigned int slot = hash(key) % m_slots;

            WRLock lock(&(m_table[slot].lock));
            return doRemove(slot, key, value);
        }

        bool lookup(const KeyType& key, ValueType* value = NULL)
        {
            unsigned int slot = hash(key) % m_slots;

            RDLock lock(&(m_table[slot].lock));
            return doLookup(slot, key, value);
        }

        void clear()
        {
            for (auto i = m_table.begin(); i != m_table.end(); ++i) {
                WRLock lock(&(i->lock));
                i->itemlist.clear();
            }
        }

        bool clear(SlotIterator& o)
        {
            for (unsigned int i = 0; i < m_table.size(); ++i) {
                HashHead& head = m_table[i];

                if (!head.itemlist.empty()) {
                    WRLock lock(&(head.lock));
                    if (!o.process(i, head.itemlist))
                        return false;

                    head.itemlist.clear();
                }
            }

            return true;
        }

        bool foreach(Iterator& o)
        {
            for (auto i = m_table.begin(); i != m_table.end(); ++i) {
                RDLock lock(&(i->lock));
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

                if (!head.itemlist.empty()) {
                    RDLock lock(&(head.lock));
                    if (!o.process(i, head.itemlist))
                        return false;
                }
            }

            return true;
        }

    protected:

        virtual unsigned long hash(const KeyType& key) const = 0;
        virtual bool equal(const KeyType& a, const KeyType& b) const = 0;

    private:

        bool doInsert(unsigned long slot, const KeyType& key, const ValueType& value,
                      bool replace)
        {
            std::list<std::pair<const KeyType, ValueType>>& itemlist = m_table[slot].itemlist;

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

        bool doRemove(unsigned long slot, const KeyType& key, ValueType* value)
        {
            std::list<std::pair<const KeyType, ValueType>>& itemlist = m_table[slot].itemlist;

            for (auto i = itemlist.begin(); i != itemlist.end(); ++i) {
                if (equal(key, i->first)) {
                    if (value)
                        *value = i->second;

                    itemlist.erase(i);
                    __sync_sub_and_fetch(&m_items, 1);
                    return true;
                }
            }

            return false; // not found
        }

        bool doLookup(unsigned long slot, const KeyType& key, ValueType* value)
        {
            std::list<std::pair<const KeyType, ValueType>>& itemlist = m_table[slot].itemlist;

            for (auto i = itemlist.begin(); i != itemlist.end(); ++i) {
                if (equal(key, i->first)) {
                    if (value)
                        *value = i->second;

                    return true;
                }
            }

            return false;
        }

    private:

        class RDLock {

            public:

                RDLock(LockType* lock_)
                {
                    m_lock = lock_;
                    lock_->rd_lock();
                }

                ~RDLock()
                {
                    m_lock->rd_unlock();
                }

            private:

                LockType* m_lock;
        };

        class WRLock {

            public:

                WRLock(LockType* lock_)
                {
                    m_lock = lock_;
                    lock_->wr_lock();
                }

                ~WRLock()
                {
                    m_lock->wr_unlock();
                }

            private:

                LockType* m_lock;
        };

        struct HashHead {
            LockType lock;
            std::list<std::pair<const KeyType, ValueType>> itemlist;
        };

    private:

        unsigned long m_items;
        unsigned int m_slots;
        std::vector<HashHead> m_table;
};

}

#endif
