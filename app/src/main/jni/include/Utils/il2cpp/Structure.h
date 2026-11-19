#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <type_traits>
#include <vector>

#include "Vector3.h"

struct monoObject {
    Il2CppClass *klass;
    void *monitor;
};

template <typename T>
std::string ToStringValue(const T &value) {
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (std::is_same_v<U, std::string>) {
        return value;
    } else if constexpr (std::is_same_v<U, const char *> || std::is_same_v<U, char *>) {
        return value ? std::string(value) : std::string("null");
    } else if constexpr (std::is_same_v<U, Vector3>) {
        return Vector3::ToString(value);
    } else if constexpr (std::is_same_v<U, uintptr_t>) {
        return "uintptr_t";
    } else if constexpr (std::is_pointer_v<U>) {
        return "ptr";
    } else if constexpr (std::is_arithmetic_v<U>) {
        return std::to_string(value);
    } else {
        return "other";
    }
}

template <typename TKey, typename TValue>
struct KeyValuePair {
    TKey Key;
    TValue Value;

    KeyValuePair() = default;
    KeyValuePair(const TKey &key, const TValue &value) : Key(key), Value(value) {}

    bool operator==(const KeyValuePair &other) const {
        return Key == other.Key && Value == other.Value;
    }

    bool operator!=(const KeyValuePair &other) const {
        return !(*this == other);
    }

    std::string ToString() const {
        return "[" + ToStringValue(Key) + ", " + ToStringValue(Value) + "]";
    }
};

template <typename T>
struct monoArray : monoObject {
    void *bounds;
    int max_length;
    T vector[1];

    static monoArray<T> *Create(Il2CppClass *arrayClass, int size) {
        if (!arrayClass || size < 0)
            return nullptr;
        return reinterpret_cast<monoArray<T> *>(CreateArrayFromClass(arrayClass, size));
    }

    int getLength() const {
        return max_length;
    }

    T *getPointer() {
        return vector;
    }

    const T *getPointer() const {
        return vector;
    }

    std::vector<T> toVector() const {
        std::vector<T> vec;
        vec.reserve(max_length);
        for (int i = 0; i < max_length; ++i) {
            vec.push_back(vector[i]);
        }
        return vec;
    }

    bool IsValidIndex(int index) const {
        return index >= 0 && index < max_length;
    }

    T Get(int index) const {
        if (!IsValidIndex(index))
            return T{};
        return vector[index];
    }

    void Set(int index, const T &item) {
        if (!IsValidIndex(index))
            return;
        vector[index] = item;
    }

    monoArray<T> *Add(const T &item) {
        if (!this->klass)
            return nullptr;

        auto *newArray = Create(this->klass, max_length + 1);
        if (!newArray)
            return nullptr;

        for (int i = 0; i < max_length; ++i) {
            newArray->vector[i] = vector[i];
        }
        newArray->vector[max_length] = item;
        return newArray;
    }

    monoArray<T> *RemoveAt(int index) {
        if (!IsValidIndex(index) || !this->klass)
            return nullptr;

        auto *newArray = Create(this->klass, max_length - 1);
        if (!newArray)
            return nullptr;

        for (int i = 0, j = 0; i < max_length; ++i) {
            if (i == index)
                continue;
            newArray->vector[j++] = vector[i];
        }
        return newArray;
    }

    monoArray<T> *Remove(const T &item) {
        for (int i = 0; i < max_length; ++i) {
            if (vector[i] == item)
                return RemoveAt(i);
        }
        return nullptr;
    }

    void Sort() {
        std::sort(vector, vector + max_length);
    }

    template <typename Compare>
    void Sort(Compare comp) {
        std::sort(vector, vector + max_length, comp);
    }

    bool operator==(const monoArray<T> &other) const {
        if (max_length != other.max_length)
            return false;
        for (int i = 0; i < max_length; ++i) {
            if (!(vector[i] == other.vector[i]))
                return false;
        }
        return true;
    }

    bool operator!=(const monoArray<T> &other) const {
        return !(*this == other);
    }
};

template <typename T>
struct monoList : monoObject {
    monoArray<T> *items;
    int size;
    int version;
    void *syncRoot;
    
    void SetFloat2(int index, float value, Il2CppClass *singleClass) {
        if (!items || !singleClass)
            return;
        items->vector[index] = reinterpret_cast<T>(ObjectBoxV(singleClass, &value));
    }
    void SetFloat(int index, float value) {
    if (!items || index < 0 || index >= size)
        return;

    items->vector[index] = value;
}
    
    static monoList<T>* Create(Il2CppClass *listClass, Il2CppClass* arrayClass, int capacity = 4) {
        if (!listClass || !arrayClass || capacity < 0)
            return nullptr;

        auto *list = reinterpret_cast<monoList<T>*>(CreateObjectFromClass(listClass));
        if (!list)
            return nullptr;

        auto *arr = monoArray<T>::Create(arrayClass, capacity);
        if (!arr)
            return nullptr;

        list->items = arr;
        list->size = 0;
        list->version = 0;
        list->syncRoot = nullptr;

        return list;
    }
    
    int getSize() const {
        return size;
    }

    int getVersion() const {
        return version;
    }

    T *getItems() {
        return items ? items->getPointer() : nullptr;
    }

    const T *getItems() const {
        return items ? items->getPointer() : nullptr;
    }

    std::vector<T> toVector() const {
        std::vector<T> vec;
        vec.reserve(size);
        if (!items)
            return vec;
        for (int i = 0; i < size; ++i) {
            vec.push_back(items->vector[i]);
        }
        return vec;
    }

    bool IsValidIndex(int index) const {
        return index >= 0 && index < size;
    }

    T Get(int index) const {
        if (!items || !IsValidIndex(index))
            return T{};
        return items->vector[index];
    }

    void EnsureCapacity(int minCapacity) {
        if (!items)
            return;
        if (items->max_length >= minCapacity)
            return;

        int oldCapacity = items->max_length;
        int newCapacity = oldCapacity <= 0 ? 4 : oldCapacity * 2;
        if (newCapacity < minCapacity)
            newCapacity = minCapacity;

        auto *newItems = monoArray<T>::Create(items->klass, newCapacity);
        if (!newItems)
            return;

        for (int i = 0; i < size; ++i) {
            newItems->vector[i] = items->vector[i];
        }
        items = newItems;
    }

    void Add(const T &item) {
        if (!items)
            return;
        EnsureCapacity(size + 1);
        if (!items || size >= items->max_length)
            return;
        items->vector[size++] = item;
        ++version;
    }

    bool RemoveAt(int index) {
        if (!items || !IsValidIndex(index))
            return false;

        for (int i = index; i < size - 1; ++i) {
            items->vector[i] = items->vector[i + 1];
        }
        items->vector[size - 1] = T{};
        --size;
        ++version;
        return true;
    }

    bool Remove(const T &item) {
        if (!items)
            return false;
        for (int i = 0; i < size; ++i) {
            if (items->vector[i] == item)
                return RemoveAt(i);
        }
        return false;
    }

    void Sort() {
        if (!items || size <= 1)
            return;
        std::sort(items->vector, items->vector + size);
        ++version;
    }

    template <typename Compare>
    void Sort(Compare comp) {
        if (!items || size <= 1)
            return;
        std::sort(items->vector, items->vector + size, comp);
        ++version;
    }

    bool operator==(const monoList<T> &other) const {
        if (size != other.size)
            return false;
        for (int i = 0; i < size; ++i) {
            if (!(items->vector[i] == other.items->vector[i]))
                return false;
        }
        return true;
    }

    bool operator!=(const monoList<T> &other) const {
        return !(*this == other);
    }
};

template <typename TKey, typename TValue>
struct monoDictionary : monoObject {
    struct Entry {
        int hashCode;
        int next;
        TKey key;
        TValue value;

        bool operator==(const Entry &other) const {
            return hashCode == other.hashCode && next == other.next && key == other.key && value == other.value;
        }
    };

    monoArray<int> *pBuckets;
    monoArray<Entry> *pEntries;
    int count;
    int version;
    int freeList;
    int freeCount;
    void *comparer;
    void *keys;
    void *values;
    void *_syncRoot;

    Entry *GetEntry() {
        return pEntries ? reinterpret_cast<Entry *>(pEntries->vector) : nullptr;
    }

    const Entry *GetEntry() const {
        return pEntries ? reinterpret_cast<const Entry *>(pEntries->vector) : nullptr;
    }

    TKey GetKeyByIndex(int iIndex) const {
        if (!pEntries || iIndex < 0 || iIndex >= count)
            return TKey{};
        return GetEntry()[iIndex].key;
    }

    TValue GetValueByIndex(int iIndex) const {
        if (!pEntries || iIndex < 0 || iIndex >= count)
            return TValue{};
        return GetEntry()[iIndex].value;
    }

    int getCount() const {
        return count;
    }

    std::vector<TKey> toVecKey() const {
        std::vector<TKey> vec;
        vec.reserve(count);
        for (int i = 0; i < count; ++i) {
            vec.push_back(GetKeyByIndex(i));
        }
        return vec;
    }

    std::vector<TValue> toVecValue() const {
        std::vector<TValue> vec;
        vec.reserve(count);
        for (int i = 0; i < count; ++i) {
            vec.push_back(GetValueByIndex(i));
        }
        return vec;
    }

    int FindIndex(const TKey &key) const {
        const Entry *entries = GetEntry();
        if (!entries)
            return -1;
        for (int i = 0; i < count; ++i) {
            if (entries[i].hashCode >= 0 && entries[i].key == key)
                return i;
        }
        return -1;
    }

    void EnsureCapacity(int minCapacity) {
        if (!pEntries)
            return;
        if (pEntries->max_length >= minCapacity)
            return;

        int oldCapacity = pEntries->max_length;
        int newCapacity = oldCapacity <= 0 ? 4 : oldCapacity * 2;
        if (newCapacity < minCapacity)
            newCapacity = minCapacity;

        auto *newEntries = monoArray<Entry>::Create(pEntries->klass, newCapacity);
        if (!newEntries)
            return;

        Entry *oldData = GetEntry();
        Entry *newData = reinterpret_cast<Entry *>(newEntries->vector);
        for (int i = 0; i < count; ++i) {
            newData[i] = oldData[i];
        }
        pEntries = newEntries;

        if (pBuckets && pBuckets->klass) {
            auto *newBuckets = monoArray<int>::Create(pBuckets->klass, newCapacity);
            if (newBuckets) {
                for (int i = 0; i < newCapacity; ++i) {
                    newBuckets->vector[i] = -1;
                }
                pBuckets = newBuckets;
            }
        }
    }

    void Add(const TKey &key, const TValue &value) {
        if (!pEntries)
            return;

        int index = FindIndex(key);
        if (index >= 0) {
            GetEntry()[index].value = value;
            ++version;
            return;
        }

        EnsureCapacity(count + 1);
        Entry *entries = GetEntry();
        if (!entries || count >= pEntries->max_length)
            return;

        entries[count].hashCode = 0;
        entries[count].next = -1;
        entries[count].key = key;
        entries[count].value = value;
        ++count;
        ++version;
    }

    bool Remove(const TKey &key) {
        Entry *entries = GetEntry();
        if (!entries)
            return false;

        int index = FindIndex(key);
        if (index < 0)
            return false;

        for (int i = index; i < count - 1; ++i) {
            entries[i] = entries[i + 1];
        }
        entries[count - 1] = Entry{};
        --count;
        ++version;
        return true;
    }

    void Sort() {
        Entry *entries = GetEntry();
        if (!entries || count <= 1)
            return;
        std::sort(entries, entries + count, [](const Entry &a, const Entry &b) { return a.key < b.key; });
        ++version;
    }

    template <typename Compare>
    void Sort(Compare comp) {
        Entry *entries = GetEntry();
        if (!entries || count <= 1)
            return;
        std::sort(entries, entries + count, comp);
        ++version;
    }

    bool operator==(const monoDictionary<TKey, TValue> &other) const {
        if (count != other.count)
            return false;
        for (int i = 0; i < count; ++i) {
            int otherIndex = other.FindIndex(GetKeyByIndex(i));
            if (otherIndex < 0)
                return false;
            if (!(GetValueByIndex(i) == other.GetValueByIndex(otherIndex)))
                return false;
        }
        return true;
    }

    bool operator!=(const monoDictionary<TKey, TValue> &other) const {
        return !(*this == other);
    }
};

template <typename T>
struct HashSet : monoObject {
    struct Slot {
        int HashCode;
        int Next;
        T value;
    };

    monoArray<int> *_buckets;
    monoArray<Slot> *_slots;
    int _count;
    int _lastIndex;
    int _freeList;
    void *_comparer;
    int _version;
    void *_siInfo;

    Slot *GetSlot() {
        return _slots ? reinterpret_cast<Slot *>(_slots->vector) : nullptr;
    }

    T GetValueByIndex(const int iIndex) {
        T tValue{};
        Slot *pSlot = GetSlot();
        if (pSlot)
            tValue = pSlot[iIndex].value;
        return tValue;
    }

    int getCount() {
        return _count;
    }

    std::vector<T> toVector() {
        std::vector<T> vec;
        for (int i = 0; i < _count; ++i) {
            vec.push_back(GetValueByIndex(i));
        }
        return vec;
    }
};

struct Vec3 : monoObject {
    float x;
    float y;
    float z;

    Vector3 getVector3() {
        return Vector3(x, y, z);
    }

    std::string getName() {
        return Vector3::ToString(getVector3());
    }
};

struct ShortVec3 : monoObject {
    short x;
    short y;
    short z;
    
    float getFloat(short a) {
        return static_cast<float>(a);
    }
    
    Vector3 getVector3() {
        return Vector3(getFloat(x), getFloat(y), getFloat(z));
    }

    std::string getName() {
        return Vector3::ToString(getVector3());
    }
};