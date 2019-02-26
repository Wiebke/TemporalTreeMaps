#pragma once

#include <list>
#include <unordered_map>

namespace inviwo
{

/** Simple cache with Least Recently Used strategy.

    The cache holds dhared pointers to given elements.
    These pointers will be released upon removal from the cache.
    Hence, if no other party holds a pointer to that element,
    the destruction of the shared pointer will release the element itself.

    @author Tino Weinkauf
*/
template< typename TKey, typename TElement>
class SimpleLRUCache
{
public:
    SimpleLRUCache(const size_t argMaxCacheSize = 1)
    {
        SetMaxCacheSize(argMaxCacheSize);
    }

    virtual ~SimpleLRUCache() = default;

//Methods
public:
    /** Returns a shared pointer to a cached element, or NULL if not in cache.

        Records the usage of the element to enable Least Recently Used strategy.
    */
    std::shared_ptr< TElement > Find(const TKey& Key)
    {
        //Is it in the map?
        auto it = CacheMap.find(Key);
        if (it != CacheMap.end())
        {
            //Yes, it is cached!
            // - Record usage
            Cache.splice(Cache.begin(), Cache, it->second);
            // - Get it and return it.
            return it->second->second;
        }

        //Not cached!
        return NULL;
    }

    /** Adds an element to the cache.

        Makes sure to keep the cache size at bay.
        Cache removal is done before the element is added.

        Adding an element with an already used key will override that element,
        i.e., the previously added element will be removed from the cache.

        Adding an element puts it at the front of the cache, i.e., it is the \b most recently used element.
    */
    void Add(const TKey& Key, std::shared_ptr< TElement > pElement)
    {
        //Do we have that key already?
        auto it = CacheMap.find(Key);

        //We replace the element, if a matching key is found
        if (it != CacheMap.end())
        {
            //Delete the old one here, add the new one below.
            Cache.erase(it->second);
            CacheMap.erase(it);
        }

        //Make some space
        ReleaseLRUElements(MaxCacheSize - 1);

        //Add the new element
        Cache.push_front(std::make_pair(Key, pElement));
        CacheMap.insert(std::make_pair(Key, Cache.begin()));
    }

    ///Returns the current number of elements in the cache.
    size_t GetCacheSize() const
    {
        return Cache.size();
    }

    ///Returns the maximum size of the cache.
    size_t GetMaxCacheSize() const
    {
        return MaxCacheSize;
    }

    ///Sets the maximum size of the cache. May lead to releasing some elements.
    ///The cache has at least a maximum size of 1. First, this makes most sense.
    ///Second, the implementation requires it as there are calls that would otherwise fail.
    void SetMaxCacheSize(const size_t DesiredMaxCacheSize)
    {
        MaxCacheSize = DesiredMaxCacheSize > 0 ? DesiredMaxCacheSize : 1;
        ReleaseLRUElements(MaxCacheSize);
    }

private:
    void ReleaseLRUElements(const size_t DesiredMaxSize)
    {
        while (Cache.size() > DesiredMaxSize)
        {
            //Remove from the end of the list. That is the least recently used element.
            auto itLastElem = Cache.back();
            CacheMap.erase(itLastElem.first);
            Cache.pop_back();
        }
    }

//Attributes
private:
    ///Maximal size of the cache. It will never have more elements than that.
    size_t MaxCacheSize;

    ///Least Recently Used Cache for previously loaded files
    std::list< std::pair< TKey, std::shared_ptr< TElement > > > Cache;

    ///Fast access to the Cache based on the file index
    std::unordered_map< TKey, decltype(Cache.begin()) > CacheMap;
};

} // namespace
