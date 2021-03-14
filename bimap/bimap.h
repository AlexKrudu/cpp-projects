#pragma once

#include <cstddef>
#include <utility>
#include <functional>
#include <iostream>
#include <random>
#include <type_traits>


template<typename Left, typename Right, typename CompareLeft = std::less<Left>,
        typename CompareRight = std::less<Right>>
struct bimap {

    using left_t = Left;
    using right_t = Right;


    struct tag_key {
    };
    struct tag_value {
    };

    template<typename T, typename side>
    struct node_light {
        node_light *left = nullptr;
        node_light *right = nullptr;
        node_light *parent = nullptr;
        T data;
        long long priority = rand();

        explicit node_light(const T &val) noexcept: data(val) {};

        explicit node_light(T &&val) noexcept: data(std::move(val)) {};


        node_light() = default;

        node_light<T, side> *next() noexcept {
            auto temp = this;
            if (right != nullptr) {
                temp = temp->right;
                while (temp->left != nullptr) {
                    temp = temp->left;
                }
                return temp;
            }
            while (temp->parent != nullptr && temp->parent->left != temp) {
                temp = temp->parent;
            }
            return temp->parent;
        }

        node_light<T, side> *prev() noexcept {
            auto temp = this;
            if (left != nullptr) {
                temp = temp->left;
                while (temp->right != nullptr) {
                    temp = temp->right;
                }
                return temp;
            }
            while (temp->parent->left == temp) {
                temp = temp->parent;
            }
            return temp->parent;
        }


    };

    struct node_heavy : node_light<Left, tag_key>, node_light<Right, tag_value> {
        node_heavy(left_t key, right_t value) noexcept: node_light<Left, tag_key>(std::move(key)),
                                                        node_light<Right, tag_value>(std::move(value)) {}
    };

    template<typename T, typename side, typename Compare>
    struct Treap {
    public:
        using node_t = node_light<T, side> *;
        node_t root = nullptr;
        Compare cmp = Compare();

        Treap(Treap &&other) noexcept {
            std::swap(root, other.root);
        }


        Treap& operator=(const Treap& other) = default;

        node_t merge(node_t t1, node_t t2) noexcept {
            if (t1 == nullptr) {
                return t2;
            }
            if (t2 == nullptr) {
                return t1;
            }
            if (t1->priority > t2->priority) {
                t1->right = merge(t1->right, t2);
                t1->right->parent = t1;
                return t1;
            } else {
                t2->left = merge(t1, t2->left);
                t2->left->parent = t2;
                return t2;
            }
        }


        explicit Treap(const Compare &cmp) : cmp(cmp) {};

        std::pair<node_t, node_t> split(node_t t, const T &val) {
            std::pair<node_t, node_t> res;
            if (t == nullptr) {
                return res;
            }
            if (cmp(t->data, val)) {
                res = split(t->right, val);
                t->right = res.first;
                if (t->right != nullptr) {
                    t->right->parent = t;
                }
                res.first = t;
                if (res.first != nullptr) {
                    res.first->parent = nullptr;
                }
                if (res.second != nullptr) {
                    res.second->parent = nullptr;
                }
                return res;
            } else {
                res = split(t->left, val);
                t->left = res.second;
                if (t->left != nullptr) {
                    t->left->parent = t;
                }
                res.second = t;
                if (res.first != nullptr) {
                    res.first->parent = nullptr;
                }
                if (res.second != nullptr) {
                    res.second->parent = nullptr;
                }
                return res;
            }
        }

        void insert(node_t node) {
            root = insert_wrap(root, node);
        }

        node_t insert_wrap(node_t roott, node_t node) {
            if (roott == nullptr) {
                return node;
            }
            if (roott->priority < node->priority) {
                std::pair<node_t, node_t> res = split(roott, node->data);
                node->left = res.first;
                node->right = res.second;
                if (res.first != nullptr) {
                    res.first->parent = node;
                }
                if (res.second != nullptr) {
                    res.second->parent = node;
                }
                return node;
            } else {
                if (!cmp(roott->data, node->data)) {
                    roott->left = insert_wrap(roott->left, node);
                } else {
                    roott->right = insert_wrap(roott->right, node);
                }
                if (roott->left != nullptr) {
                    roott->left->parent = roott;
                }
                if (roott->right != nullptr) {
                    roott->right->parent = roott;
                }
                return roott;
            }
        }

        void destroy() noexcept {
            destroy_wrap(root);
        }

        void destroy_wrap(node_t n) noexcept {
            if (n != nullptr) {
                destroy_wrap(n->left);
                destroy_wrap(n->right);
                delete static_cast<node_heavy *>(n);
            }
        }

        void erase(T val) {
            node_t prev = nullptr;
            node_t *roott = &(root);
            node_t cur = *roott;
            while (!(cur->data == val)) {
                if (!cmp(val, cur->data) && cur->right != nullptr) {
                    prev = cur;
                    cur = cur->right;
                } else if (cmp(val, cur->data) && cur->left != nullptr) {
                    prev = cur;
                    cur = cur->left;
                }
            }
            if (prev == nullptr) {
                *roott = merge((*roott)->left, (*roott)->right);
                if (*roott != nullptr) {
                    (*roott)->parent = nullptr;
                }
                return;
            }
            if (cur == prev->right) {
                prev->right = merge(cur->left, cur->right);
                if (prev->right != nullptr) {
                    prev->right->parent = prev;
                }
                return;
            }
            prev->left = merge(cur->left, cur->right);
            if (prev->left != nullptr)
                prev->left->parent = prev;
        }

        node_t exists(const T &val) const {
            return exists_wrap(root, val);
        }


        node_t exists_wrap(node_t t, const T &val) const {
            if (t == nullptr || t->data == val) {
                return t;
            }
            if (cmp(val, t->data)) {
                return exists_wrap(t->left, val);
            } else {
                return exists_wrap(t->right, val);
            }
        }
    };

private:
    Treap<Left, tag_key, CompareLeft> left_tree;
    Treap<Right, tag_value, CompareRight> right_tree;
    size_t pair_count = 0;
public:
    template<typename side>
    struct iterator {

        using type = typename std::conditional<std::is_same_v<side, tag_key>, left_t, right_t>::type;
        using inv_side = typename std::conditional<std::is_same_v<side, tag_key>, tag_value, tag_key>::type;

        node_light<type, side> *cur_node;

        node_heavy* tree_root;

        explicit iterator(node_light<type, side> *nodeLight, node_heavy* root) noexcept: cur_node(nodeLight), tree_root(root) {};

        explicit iterator(node_heavy *nodeHeavy, node_heavy* root) noexcept: cur_node(
                static_cast<node_light<type, side> *>(nodeHeavy)), tree_root(root) {};

        explicit iterator(nullptr_t, node_heavy* root) noexcept : cur_node(nullptr), tree_root(root) {};

        type const &operator*() const noexcept {
            return cur_node->data;
        };

        iterator &operator++() noexcept {
            cur_node = cur_node->next();
            return *this;
        };

        iterator operator++(int) noexcept {
            iterator prev = *this;
            ++*this;
            return prev;
        };

        iterator &operator--() noexcept {
            if (cur_node == nullptr){
                auto cur = static_cast<node_light<type, side>*>(tree_root);
                while (cur->right != nullptr){
                    cur = cur->right;
                }
                cur_node = cur;
                return *this;
            }
            cur_node = cur_node->prev();
            return *this;
        };

        iterator operator--(int) {
            iterator prev = *this;
            --*this;
            return prev;
        };

        friend bool operator==(iterator first, iterator second) noexcept {
            return first.cur_node == second.cur_node;
        }

        friend bool operator!=(iterator first, iterator second) noexcept {
            return first.cur_node != second.cur_node;
        }

        iterator<inv_side> flip() const noexcept {
            return iterator<inv_side>(static_cast<node_heavy *>(cur_node), tree_root);
        }

    };

    using left_iterator = iterator<tag_key>;
    using right_iterator = iterator<tag_value>;

    // Создает bimap не содержащий ни одной пары.
    explicit bimap(CompareLeft compare_left = CompareLeft(),
          CompareRight compare_right = CompareRight()) : left_tree(compare_left), right_tree(compare_right) {};

    //bimap() = default;

    // Конструкторы от других и присваивания
    bimap(bimap const &other) : left_tree(other.left_tree.cmp), right_tree(other.right_tree.cmp) {
        auto it = other.begin_left();
        while (it != other.end_left()) {
            this->insert(*it, *it.flip());
            ++it;
        }
    };

    bimap(bimap &&other) noexcept: left_tree(std::move(other.left_tree)), right_tree(std::move(other.right_tree)),
                                   pair_count(std::move(other.pair_count)) {};

    bimap &operator=(bimap const &other) {
        auto it = begin_left();
        while (it != end_left()) {
            auto tmp = it;
            ++tmp;
            erase_left(it);
            it = tmp;
        }
        // this->left_tree(other.left_tree.cmp);
        // this->right_tree(other.right_tree.cmp);
        //  this->pair_count = other.pair_count;
        auto it1 = other.begin_left();
        while (it1 != other.end_left()) {
            this->insert(*it1, *it1.flip());
            ++it1;
        }
        return *this;
    }

    bimap &operator=(bimap &&other) {
        auto it = begin_left();
        while (it != end_left()) {
            auto tmp = it;
            ++tmp;
            erase_left(it);
            it = tmp;
        }
        std::swap(this->pair_count, other.pair_count);
        std::swap(this->left_tree, other.left_tree);
        std::swap(this->right_tree, other.right_tree);
        return *this;
    };

    // Деструктор. Вызывается при удалении объектов bimap.
    // Инвалидирует все итераторы ссылающиеся на элементы этого bimap
    // (включая итераторы ссылающиеся на элементы следующие за последними).
    ~bimap() {
        left_tree.destroy();
    };
private:
    void erase_test(left_t const &left) {
        auto fake = static_cast<node_heavy *>(left_tree.exists(left));
        left_tree.erase(fake->node_light<Left, tag_key>::data);
        right_tree.erase(fake->node_light<Right, tag_value>::data);
        delete fake;
        pair_count--;
    }

    left_iterator inner_insert(node_heavy *node) {
        if (left_tree.exists(*left_iterator(node, static_cast<node_heavy*>(left_tree.root))) != nullptr || right_tree.exists(*right_iterator(node, static_cast<node_heavy*>(left_tree.root))) != nullptr) {
            delete node;
            return end_left();
        }
        // auto fake = new node_heavy(left, right);
        left_tree.insert(static_cast<node_light<Left, tag_key> *>(node));
        right_tree.insert(static_cast<node_light<Right, tag_value> *>(node));
        pair_count++;
        return left_iterator(node, static_cast<node_heavy*>(left_tree.root));
    }

public:

    // Вставка пары (left, right), возвращает итератор на left.
    // Если такой left или такой right уже присутствуют в bimap, вставка не
    // производится и возвращается end_left().
    left_iterator insert(left_t const &left, right_t const &right) {
        auto fake = new node_heavy(left, right);
        return inner_insert(fake);
    };

    left_iterator insert(left_t const &left, right_t &&right) {
        auto fake = new node_heavy(left, std::move(right));
        return inner_insert(fake);
    };

    left_iterator insert(left_t &&left, right_t const &right) {
        auto fake = new node_heavy(std::move(left), right);
        return inner_insert(fake);
    };

    left_iterator insert(left_t &&left, right_t &&right) {
        auto fake = new node_heavy(std::move(left), std::move(right));
        return inner_insert(fake);
    };

private:
    template<typename side, typename type, typename cmp>
    iterator<side> erase_it(iterator<side> it, Treap<type, side, cmp> &t,
                            Treap<typename std::conditional<std::is_same_v<type, left_t>, right_t, left_t>::type,
                                    typename std::conditional<std::is_same_v<side, tag_key>, tag_value, tag_key>::type,
                                    typename std::conditional<std::is_same_v<cmp, CompareLeft>, CompareRight, CompareLeft>::type> &t_inv) {
        auto node = static_cast<node_heavy *>(it.cur_node);
        auto tmp = it;
        type val = *it;
        ++tmp;
        t.erase(val);
        t_inv.erase(*it.flip());
        pair_count--;
        delete node;
        return tmp;
    }

public:
    // Удаляет элемент и соответствующий ему парный.
    // erase невалидного итератора неопределен.
    // erase(end_left()) и erase(end_right()) неопределены.
    // Пусть it ссылается на некоторый элемент e.
    // erase инвалидирует все итераторы ссылающиеся на e и на элемент парный к e.
    left_iterator erase_left(left_iterator it) {
        return erase_it(it, left_tree, right_tree);
    };

    right_iterator erase_right(right_iterator it) {
        return erase_it(it, right_tree, left_tree);
    };

    // Аналогично erase, но по ключу, удаляет элемент если он присутствует, иначе
    // не делает ничего Возвращает была ли пара удалена
    bool erase_left(left_t const &left) {
        node_light<Left, tag_key> *cur = left_tree.exists(left);
        if (cur != nullptr) {
            auto fake = static_cast<node_heavy *>(cur);
            left_tree.erase(left);
            right_tree.erase(fake->node_light<Right, tag_value>::data); // todo prolly should use iterator
            pair_count--;
            delete fake;
            return true;
        }
        return false;
    };


    bool erase_right(right_t const &right) {
        node_light<Right, tag_value> *cur = right_tree.exists(right);
        if (cur != nullptr) {
            return erase_left(*(right_iterator(cur, static_cast<node_heavy*>(left_tree.root)).flip()));
        }
        return false;
    };

    template<typename side, typename type, typename cmp>
    iterator<side> erase_range(iterator<side> first, iterator<side> last) {
        auto it = first;
        while (it != last) {
            auto next = ++it;
            if constexpr (std::is_same_v<side, tag_key>) {
                erase_left(--it);
            } else {
                erase_right(--it);
            }
            it = next;
        }
        return it;
    }


    // erase от ренжа, удаляет [first, last), возвращает итератор на последний
    // элемент за удаленной последовательностью
    left_iterator erase_left(left_iterator first, left_iterator last) {
        return erase_range<tag_key, left_t, CompareLeft>(first, last);
    };

    right_iterator erase_right(right_iterator first, right_iterator last) {
        return erase_range<tag_value, right_t, CompareRight>(first, last);
    };

    template<typename side, typename type, typename cmp>
    iterator<side> find(type const &key, const Treap<type, side, cmp> &t, iterator<side> end) const {
        auto node = t.exists(key);
        if (node == nullptr)
            return end;
        return iterator<side>(node, static_cast<node_heavy*>(left_tree.root));
    }

    // Возвращает итератор по элементу. Если не найден - соответствующий end()
    left_iterator find_left(left_t const &left) const {
        return find(left, left_tree, end_left());
    }

    right_iterator find_right(right_t const &right) const {
        return find(right, right_tree, end_right());
    };


    template<typename side, typename type, typename cmp, typename inv_type>
    inv_type const &at(type const &key, const Treap<type, side, cmp> &t) const {
        auto node = t.exists(key);
        if (node == nullptr) {
            throw std::out_of_range("Bruh");
        }
        return *iterator<side>(node, static_cast<node_heavy*>(left_tree.root)).flip();
    }

    // Возвращает противоположный элемент по элементу
    // Если элемента не существует -- бросает std::out_of_range
    right_t const &at_left(left_t const &key) const {
        return at<tag_key, left_t, CompareLeft, right_t>(key, left_tree);
    };

    left_t const &at_right(right_t const &key) const {
        return at<tag_value, right_t, CompareRight, left_t>(key, right_tree);
    };

    // Возвращает противоположный элемент по элементу
    // Если элемента не существует, добавляет его в bimap и на противоположную
    // сторону кладет дефолтный элемент, ссылку на который и возвращает
    // Если дефолтный элемент уже лежит в противоположной паре - должен поменять
    // соответствующий ему элемент на запрашиваемый (смотри тесты)
    template<typename T, typename = std::enable_if_t<
            std::is_same_v<T, right_t> &&
            std::is_default_constructible_v<left_t>>>
    right_t const &at_left_or_default(T const &key) {
        auto node = this->left_tree.exists(key);
        if (node == nullptr) {
            auto default_right = right_t();
            auto n_right = this->right_tree.exists(default_right);
            if (n_right != nullptr) {
                erase_right(right_iterator(n_right, static_cast<node_heavy*>(left_tree.root)));
            }
            return *(insert(key, default_right).flip());
        } else {
            return *left_iterator(node, static_cast<node_heavy*>(left_tree.root)).flip();
        }
    }

    template<typename T, typename = std::enable_if_t<
            std::is_same_v<T, left_t> &&
            std::is_default_constructible_v<right_t>>>
    left_t const &at_right_or_default(T const &key) {
        auto node = this->right_tree.exists(key);
        if (node == nullptr) {
            auto default_left = left_t();
            auto n_left = this->left_tree.exists(default_left);
            if (n_left != nullptr) {
                erase_left(left_iterator(n_left, static_cast<node_heavy*>(left_tree.root)));
            }
            return *(insert(default_left, key));
        } else {
            return *right_iterator(node, static_cast<node_heavy*>(left_tree.root)).flip();
        }
    }

    // lower и upper bound'ы по каждой стороне
    // Возвращают итераторы на соответствующие элементы
    // Смотри std::lower_bound, std::upper_bound.
private:
    template<typename side, typename type, typename cmp>
    iterator<side> lower_bound(iterator<side> begin, const Treap<type, side, cmp> &t, type key) const {
        int dist = size();
        auto first = begin;
        while (dist > 0) {
            auto it = first;
            auto hop = dist / 2;
            for (int i = 0; i < hop; i++) {
                ++it;
            }
            if (t.cmp(*it, key)) {
                first = ++it;
                dist -= (hop + 1);
            } else {
                dist = hop;
            }
        }
        return first;
    }

    template<typename side, typename type, typename cmp>
    iterator<side> upper_bound(iterator<side> begin, const Treap<type, side, cmp> &t, type key) const {
        int dist = size();
        auto first = begin;
        while (dist > 0) {
            auto it = first;
            auto hop = dist / 2;
            for (int i = 0; i < hop; i++) {
                ++it;
            }
            if (!t.cmp(key, *it)) {
                first = ++it;
                dist -= (hop + 1);
            } else {
                dist = hop;
            }
        }
        return first;
    }

public:
    left_iterator lower_bound_left(const left_t &left) const {
        return lower_bound(begin_left(), left_tree, left);
    };

    left_iterator upper_bound_left(const left_t &left) const {
        return upper_bound(begin_left(), left_tree, left);
    };

    right_iterator lower_bound_right(const right_t &right) const {
        return lower_bound(begin_right(), right_tree, right);
    };

    right_iterator upper_bound_right(const right_t &right) const {
        return upper_bound(begin_right(), right_tree, right);
    };

    // Возващает итератор на минимальный по порядку left.
    left_iterator begin_left() const noexcept {
        node_light<Left, tag_key> *cur = left_tree.root;
        if (cur != nullptr) {
            while (cur->left != nullptr) {
                cur = cur->left;
            }
            return left_iterator(cur, static_cast<node_heavy*>(left_tree.root));
        }
        return end_left();
    };

    // Возващает итератор на следующий за последним по порядку left.
    left_iterator end_left() const noexcept {
        return left_iterator(nullptr, static_cast<node_heavy*>(left_tree.root));
    }

    // Возващает итератор на минимальный по порядку right.
    right_iterator begin_right() const noexcept {
        node_light<Right, tag_value> *cur = right_tree.root;
        if (cur != nullptr) {
            while (cur->left != nullptr) {
                cur = cur->left;
            }
            return right_iterator(cur, static_cast<node_heavy*>(left_tree.root));
        }
        return end_right();
    };

    // Возващает итератор на следующий за последним по порядку right.
    right_iterator end_right() const noexcept {
        return right_iterator(nullptr, static_cast<node_heavy*>(left_tree.root));
    };

    // Проверка на пустоту
    [[nodiscard]] bool empty() const noexcept {
        return pair_count == 0;
    };

    // Возвращает размер бимапы (кол-во пар)
    [[nodiscard]] std::size_t size() const noexcept {
        return pair_count;
    };

};

// операторы сравнения
template<typename Left, typename Right, typename CompareLeft = std::less<Left>,
        typename CompareRight = std::less<Right>>
bool operator==(bimap<Left, Right, CompareLeft, CompareRight> const &a,
                bimap<Left, Right, CompareLeft, CompareRight> const &b) {
    if (a.size() != b.size()) {
        return false;
    }
    auto it_a = a.begin_left();
    auto it_b = b.begin_left();
    while (it_a != a.end_left()) {
        if (*it_a != *it_b || *(it_a.flip()) != *(it_b.flip())) {
            return false;
        }
        ++it_a;
        ++it_b;
    }
    return true;
}

template<typename Left, typename Right, typename CompareLeft = std::less<Left>,
        typename CompareRight = std::less<Right>>
bool operator!=(bimap<Left, Right, CompareLeft, CompareRight> const &a,
                bimap<Left, Right, CompareLeft, CompareRight> const &b) {
    return !(a == b);
}
