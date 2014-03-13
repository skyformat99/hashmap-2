unsigned long simple_hash11(const char* begin, unsigned int len)
{
    register const char* end;
    register unsigned long hash = 0;

    for (end = begin + len; begin < end; ++begin)
        hash += (hash << 3) + (hash << 1) + (*begin);

    return hash;
}

unsigned long simple_hash31(const char* begin, unsigned int len)
{
    register const char* end;
    register unsigned long hash = 0;

    for (end = begin + len; begin < end; ++begin)
        hash = (hash << 5) - hash + (*begin);

    return hash;
}

unsigned long djb_hash(const char* begin, unsigned int len)
{
    register const char* end;
    register unsigned long hash = 5381;

    for (end = begin + len; begin < end; ++begin)
        hash += (hash << 5) + (*begin);

    return hash;
}

unsigned long sdbm_hash(const char* begin, unsigned int len)
{
    register const char* end;
    register unsigned long hash = 0;

    for (end = begin + len; begin < end; ++begin)
        hash = (*begin) + (hash << 6) + (hash << 16) - hash;

    return hash;
}

unsigned long dek_hash(const char* begin, unsigned int len)
{
    register const char* end;
    register unsigned long hash = len;

    for (end = begin + len; begin < end; ++begin)
        hash = (hash << 5) ^ (hash >> 27) ^ (*begin);

    return hash;
}

unsigned long bp_hash(const char* begin, unsigned int len)
{
    register const char* end;
    register unsigned long hash = 0;

    for (end = begin + len; begin < end; ++begin)
        hash = hash << 7 ^ (*begin);

    return hash;
}

unsigned long pjw_hash(const char* begin, unsigned int len)
{
    register const char* end;
    register unsigned long g, hash = 0;

    for (end = begin + len; begin < end; ++begin) {
        hash = (hash << 4) + (*begin);
        g = hash & 0xf0000000;
        if (g) {
            hash ^= (g >> 24);
            hash ^= g;
        }
    }

    return hash;
}
