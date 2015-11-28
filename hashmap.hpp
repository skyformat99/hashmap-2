#ifndef __HASHMAP_HPP__
#define __HASHMAP_HPP__

#include <list>
#include <utility> // std::pair
#include <pthread.h>
#include <functional>

#define MAX_NR_SLOT 999983

namespace utils {

class HashLock {

    public:

        virtual void rd_lock() = 0;
        virtual void wr_lock() = 0;
        virtual void rd_unlock() = 0;
        virtual void wr_unlock() = 0;
};

class DummyHashLock : public HashLock {

    public:

        void rd_lock() {}
        void wr_lock() {}
        void rd_unlock() {}
        void wr_unlock() {}
};

class PthreadRWLock : public HashLock {

    public:

        PthreadRWLock()
        {
            pthread_rwlock_init(&m_lock, nullptr);
        }

        ~PthreadRWLock()
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

class PthreadMutexLock : public HashLock {

    public:

        PthreadMutexLock()
        {
            pthread_mutex_init(&m_lock, nullptr);
        }

        ~PthreadMutexLock()
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

class PthreadSpinLock : public HashLock {

    public:

        PthreadSpinLock()
        {
            pthread_spin_init(&m_lock, PTHREAD_PROCESS_PRIVATE);
        }

        ~PthreadSpinLock()
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

template<typename KeyType, typename ValueType, typename LockType = DummyHashLock>
class HashMap {

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

        HashMap(unsigned int slots = 0)
        {
            m_items = 0;
            m_slots = (slots > 0) ? slots : MAX_NR_SLOT;
            m_table = new HashHead [m_slots];
        }

        virtual ~HashMap()
        {
            delete [] m_table;
        }

        unsigned long size() const { return m_items; }

        unsigned int slots() const { return m_slots; }

        bool insert(const KeyType& key, const ValueType& value,
                    bool replace = false)
        {
            unsigned int slot = hash(key) % m_slots;

            WRLock lock(&(m_table[slot].lock));
            return doInsert(slot, key, value, replace);
        }

        template<typename T>
        bool insert(const KeyType& key, const T& value,
                    bool (*func)(ValueType* valuelist, const T& value))
        {
            unsigned int slot = hash(key) % m_slots;

            WRLock lock(&(m_table[slot].lock));
            return doInsert(slot, key, value, func);
        }

        bool remove(const KeyType& key, ValueType* value = nullptr)
        {
            unsigned int slot = hash(key) % m_slots;

            WRLock lock(&(m_table[slot].lock));
            return doRemove(slot, key, value);
        }

        bool lookup(const KeyType& key, ValueType* value = nullptr)
        {
            unsigned int slot = hash(key) % m_slots;

            RDLock lock(&(m_table[slot].lock));
            return doLookup(slot, key, value);
        }

        void clear(const std::function<bool (unsigned int slot,
                                             std::list<std::pair<const KeyType, ValueType>>& itemlist)>& func)
        {
            for (unsigned int i = 0; i < m_slots; ++i) {
                HashHead& head = m_table[i];

                if (!head.itemlist.empty()) {
                    WRLock lock(&(head.lock));
                    func(i, head.itemlist);
                    __sync_sub_and_fetch(&m_items, head.itemlist.size());
                    head.itemlist.clear();
                }
            }
        }

        void clear(const std::function<bool (const KeyType& key, const ValueType& value)>& func)
        {
            auto lambda = [&func] (unsigned int slot,
                                   std::list<std::pair<const KeyType, ValueType>>& itemlist) -> bool {
                for (auto o = itemlist.begin(); o != itemlist.end(); ++o) {
                    if (!func(o->first, o->second))
                        return false;
                }

                return true;
            };

            clear(lambda);
        }

        void clear()
        {
            auto dummy = [] (unsigned int slot,
                             std::list<std::pair<const KeyType, ValueType>>& itemlist) -> bool {
                return true;
            };

            clear(dummy);
        }

        void clear(Iterator& o)
        {
            auto func = [&o] (const KeyType& key, const ValueType& value) -> bool {
                return o.process(key, value);
            };

            clear(func);
        }

        void clear(SlotIterator& o)
        {
            auto func = [&o] (unsigned int slot,
                              std::list<std::pair<const KeyType, ValueType>>& itemlist) -> bool {
                return o.process(slot, itemlist);
            };

            clear(func);
        }

        bool foreach(const std::function<bool (unsigned int slot,
                                               std::list<std::pair<const KeyType, ValueType>>& itemlist)>& func)
        {
            for (unsigned int i = 0; i < m_slots; ++i) {
                HashHead& head = m_table[i];

                if (!head.itemlist.empty()) {
                    RDLock lock(&(head.lock));
                    if (!func(i, head.itemlist))
                        return false;
                }
            }

            return true;
        }

        bool foreach(SlotIterator& o)
        {
            auto func = [&o] (unsigned int slot,
                              std::list<std::pair<const KeyType, ValueType>>& itemlist) -> bool {
                return o.process(slot, itemlist);
            };

            return foreach(func);
        }

        bool foreach(const std::function<bool (const KeyType& key,
                                               const ValueType& value)>& func)
        {
            auto lambda = [&func] (unsigned int slot,
                                   std::list<std::pair<const KeyType, ValueType>>& itemlist) -> bool {
                for (auto o = itemlist.begin(); o != itemlist.end(); ++o) {
                    if (!func(o->first, o->second))
                        return false;
                }

                return true;
            };

            return foreach(lambda);
        }

        bool foreach(Iterator& o)
        {
            auto func = [&o] (const KeyType& key, const ValueType& value) -> bool {
                return o.process(key, value);
            };

            return foreach(func);
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

        template<typename T>
        bool doInsert(unsigned long slot, const KeyType& key, const T& value,
                      bool (*func)(ValueType* valuelist, const T& value))
        {
            std::list<std::pair<KeyType, ValueType>>& itemlist = m_table[slot].itemlist;

            for (auto i = itemlist.begin(); i != itemlist.end(); ++i) {
                if (equal(key, i->first))
                    return func(&i->second, value);
            }

            ValueType valuelist;
            bool ok = func(&valuelist, value);
            if (ok) {
                itemlist.push_front(std::pair<KeyType, ValueType>(key, valuelist));
                __sync_add_and_fetch(&m_items, 1);
            }

            return ok;
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
        HashHead* m_table;
};

}

#endif
