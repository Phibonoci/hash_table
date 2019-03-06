#include <vector>
#include <iterator>
#include <list>
#include <cstring>
#include <functional>

const size_t init_size = 256;
const float ratio = 0.7;

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>> class HashMap {
private:
    std::list<std::pair<const KeyType, ValueType>> l;
    std::vector<char> deleted;
    std::vector<typename std::list<std::pair<const KeyType, ValueType>>::iterator> table;
    std::conditional_t<std::is_function<Hash>::value, std::add_pointer_t<Hash>, Hash> hasher;
    size_t N, table_size = 0;

public:
    typedef typename std::list<std::pair<const KeyType, ValueType>>::iterator iterator;
    typedef typename std::list<std::pair<const KeyType, ValueType>>::const_iterator const_iterator;

    HashMap(Hash hh = Hash()): hasher(hh) {
        N = init_size;
        table.resize(N);
        deleted.resize(N);
    }

    HashMap(const HashMap& other) {
        N = other.N;
        table.resize(N);
        deleted.resize(N);
        hasher = other.hasher;
        for (const auto &i : other)
            insert(i);
    }

    HashMap(const_iterator begin, const_iterator end, Hash hh = Hash()): hasher(hh) {
        N = init_size;
        table.resize(N);
        deleted.resize(N);

        while (begin != end)
            insert(*begin++);
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> init_list, Hash hh = Hash()): hasher(hh) {
        N = std::max(init_size, init_list.size() * 2);
        table.resize(N);
        deleted.resize(N);

        for (auto &item : init_list)
            insert(item);
    }

    inline size_t size() const {
        return table_size;
    }

    bool empty() const {
        return l.empty();
    }

    auto hash_function() const {
        return hasher;
    }

    iterator begin() {
        return l.begin();
    }

    const_iterator begin() const {
        return l.cbegin();
    }

    iterator end() {
        return l.end();
    }

    const_iterator end() const {
        return l.cend();
    }

    void reallocate(size_t size) {
        N = size;
        table.resize(N);
        deleted.resize(N);
        memset(deleted.data(), 0, N);
        for (auto iter = l.begin(); iter != l.end(); ++iter) {
            for (int index = 0;; ++index) {
                int k = (hasher(iter->first) + index) % N;
                if (deleted[k] == 0 or deleted[k] == 1) {
                    deleted[k] = 2;
                    table[k] = iter;
                    break;
                }
            }
        }
    }

    inline void check() {
        if (size() * 2 > N)
            reallocate(N * 2);
        else if (size() > init_size and size() * 7 < N)
            reallocate(N / 2);
    }

    void insert(const std::pair<KeyType, ValueType>& value) {
        size_t hash = hasher(value.first);
        for (size_t i = 0; i < N; ++i) {
            size_t index = (hash + i) % N;
            if (deleted[index] == 0 || deleted[index] == 1) {
                deleted[index] = 2;
                l.push_back(value);
                table[index] = (--l.end());
                check();
                ++table_size;
                return;
            } else if (table[index]->first == value.first)
                return;
        }
    }

    void erase(const KeyType& key) {
        size_t hash = hasher(key);
        for (size_t i = 0; i < N; ++i) {
            size_t index = (hash + i) % N;
            if (deleted[index] == 2 && table[index]->first == key) {
                l.erase(table[index]);
                deleted[index] = 1;
                --table_size;
                return;
            }

            if (deleted[index] == 0)
                return;
        }
    }

    void clear() {
        for (auto it = l.begin(); it != l.end(); ++it) {
            size_t index = hasher(it->first) % N;
            while (true) {
                if (deleted[index] == 0)
                    break;
                else
                    deleted[index] = 0;
                index = (index + 1) % N;
            }
        }
        table_size = 0;
        l.clear();
    }

    iterator find(const KeyType& key) {
        size_t hash = hasher(key);
        for (size_t i = 0; i < N; ++i) {
            size_t index = (hash + i) % N;
            if (deleted[index] == 2 && table[index]->first == key)
                return table[index];

            if (deleted[index] == 0)
                return l.end();
        }
        return l.end();
    }

    const_iterator find(const KeyType& key) const {
        size_t hash = hasher(key);
        for (size_t i = 0; i < N; ++i) {
            size_t index = (hash + i) % N;
            if (deleted[index] == 2 && table[index]->first == key)
                return table[index];

            if (deleted[index] == 0)
                return end();
        }
        return end();
    }

    ValueType& operator[] (const KeyType& key) {
        size_t hash = hasher(key);
        for (size_t i = 0; i < N; ++i) {
            size_t index = (hash + i) % N;
            if (deleted[index] == 2 && table[index]->first == key)
                return table[index]->second;

            if (deleted[index] == 0) {
                insert(std::make_pair(key, ValueType()));
                return l.back().second;
            }
        }
        return l.back().second;
    }

    ValueType& at(const KeyType& key) {
        size_t hash = hasher(key);
        for (size_t i = 0; i < N; ++i) {
            size_t index = (hash + i) % N;
            if (deleted[index] == 2 && table[index]->first == key)
                return table[index]->second;

            if (deleted[index] == 0)
                throw std::out_of_range("");
        }
        throw std::out_of_range("");
    }

    const ValueType& at(const KeyType& key) const {
        size_t hash = hasher(key);
        for (size_t i = 0; i < N; ++i) {
            size_t index = (hash + i) % N;

            if (deleted[index] == 2 && table[index]->first == key)
                return table[index]->second;

            if (deleted[index] == 0)
                throw std::out_of_range("");
        }
        throw std::out_of_range("");
    }

    HashMap& operator = (const HashMap& other) {
        if (&other != this) {
            N = other.N;
            table.resize(N);
            deleted.resize(N);
            hasher = other.hasher;

            for (const auto &item : other)
                insert(item);
        }

        return *this;
    }
};
