#include <vector>
#include <iterator>
#include <list>
#include <cstring>
#include <functional>

enum {
    EMPTY = 0,
    DELETED = 1,
    USED = 2
};

const size_t INIT_SIZE = 256;
const float INCREASE_RATIO = 2, DECREASE_RATIO = 7;

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>> class HashMap {
private:
    typedef std::list<std::pair<const KeyType, ValueType>> node_list;
    node_list l;
    std::vector<char> deleted;
    std::vector<typename node_list::iterator> table;
    std::conditional_t<std::is_function<Hash>::value, std::add_pointer_t<Hash>, Hash> hasher;
    size_t table_size = 0, elements_count = 0;

public:
    typedef typename std::list<std::pair<const KeyType, ValueType>>::iterator iterator;
    typedef typename std::list<std::pair<const KeyType, ValueType>>::const_iterator const_iterator;

    HashMap(Hash hh = Hash()): hasher(hh) {
        table_size = INIT_SIZE;
        table.resize(table_size);
        deleted.resize(table_size);
    }

    HashMap(const HashMap& other) {
        table_size = other.table_size;
        table.resize(table_size);
        deleted.resize(table_size);
        hasher = other.hasher;
        for (const auto &item : other) {
            insert(item);
        }
    }

    HashMap(const_iterator begin, const_iterator end, Hash hh = Hash()): hasher(hh) {
        table_size = INIT_SIZE;
        table.resize(table_size);
        deleted.resize(table_size);

        while (begin != end) {
            insert(*begin++);
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> init_list, Hash hh = Hash()): hasher(hh) {
        table_size = std::max(INIT_SIZE, init_list.size() * 2);
        table.resize(table_size);
        deleted.resize(table_size);

        for (auto &item : init_list) {
            insert(item);
        }
    }

    inline size_t size() const {
        return elements_count;
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
        table_size = size;
        table.resize(table_size);
        deleted.resize(table_size);
        memset(deleted.data(), 0, table_size);
        for (auto iter = l.begin(); iter != l.end(); ++iter) {
            for (int index = 0;; ++index) {
                int k = (hasher(iter->first) + index) % table_size;
                if (deleted[k] == EMPTY || deleted[k] == DELETED) {
                    deleted[k] = USED;
                    table[k] = iter;
                    break;
                }
            }
        }
    }

    inline void check() {
        if (size() * INCREASE_RATIO > table_size)
            reallocate(table_size * 2);
        else if (size() > INIT_SIZE && size() * DECREASE_RATIO < table_size)
            reallocate(table_size / 2);
    }

    void insert(const std::pair<KeyType, ValueType>& value) {
        size_t hash = hasher(value.first);
        for (size_t i = 0; i < table_size; ++i) {
            size_t index = (hash + i) % table_size;
            if (deleted[index] == EMPTY || deleted[index] == DELETED) {
                deleted[index] = USED;
                l.push_back(value);
                table[index] = (--l.end());
                check();
                ++elements_count;
                return;
            } else if (table[index]->first == value.first) {
                return;
            }
        }
    }

    void erase(const KeyType& key) {
        size_t hash = hasher(key);
        for (size_t i = 0; i < table_size; ++i) {
            size_t index = (hash + i) % table_size;
            if (deleted[index] == USED && table[index]->first == key) {
                l.erase(table[index]);
                deleted[index] = DELETED;
                --elements_count;
                return;
            }

            if (deleted[index] == EMPTY) {
                return;
            }
        }
    }

    void clear() {
        for (auto it = l.begin(); it != l.end(); ++it) {
            size_t index = hasher(it->first) % table_size;
            while (true) {
                if (deleted[index] == EMPTY) {
                    break;
                } else {
                    deleted[index] = EMPTY;
                }
                index = (index + 1) % table_size;
            }
        }
        elements_count = 0;
        l.clear();
    }

    iterator find(const KeyType& key) {
        size_t hash = hasher(key);
        for (size_t i = 0; i < table_size; ++i) {
            size_t index = (hash + i) % table_size;
            if (deleted[index] == USED && table[index]->first == key) {
                return table[index];
            }

            if (deleted[index] == EMPTY) {
                return l.end();
            }
        }
        return l.end();
    }

    const_iterator find(const KeyType& key) const {
        size_t hash = hasher(key);
        for (size_t i = 0; i < table_size; ++i) {
            size_t index = (hash + i) % table_size;
            if (deleted[index] == USED && table[index]->first == key) {
                return table[index];
            }

            if (deleted[index] == EMPTY) {
                return end();
            }
        }
        return end();
    }

    ValueType& operator[] (const KeyType& key) {
        size_t hash = hasher(key);
        for (size_t i = 0; i < table_size; ++i) {
            size_t index = (hash + i) % table_size;
            if (deleted[index] == USED && table[index]->first == key) {
                return table[index]->second;
            }

            if (deleted[index] == EMPTY) {
                insert(std::make_pair(key, ValueType()));
                return l.back().second;
            }
        }
        return l.back().second;
    }

    ValueType& at(const KeyType& key) {
        size_t hash = hasher(key);
        for (size_t i = 0; i < table_size; ++i) {
            size_t index = (hash + i) % table_size;
            if (deleted[index] == USED && table[index]->first == key) {
                return table[index]->second;
            }

            if (deleted[index] == EMPTY) {
                throw std::out_of_range("");
            }
        }
        throw std::out_of_range("");
    }

    const ValueType& at(const KeyType& key) const {
        size_t hash = hasher(key);
        for (size_t i = 0; i < table_size; ++i) {
            size_t index = (hash + i) % table_size;

            if (deleted[index] == USED && table[index]->first == key) {
                return table[index]->second;
            }

            if (deleted[index] == EMPTY) {
                throw std::out_of_range("");
            }
        }
        throw std::out_of_range("");
    }

    HashMap& operator = (const HashMap& other) {
        if (&other != this) {
            table_size = other.table_size;
            table.resize(table_size);
            deleted.resize(table_size);
            hasher = other.hasher;

            for (const auto &item : other) {
                insert(item);
            }
        }

        return *this;
    }
};
