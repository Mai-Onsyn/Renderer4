#pragma once
#ifndef SETCALCULATOR_SET_HPP
#define SETCALCULATOR_SET_HPP
#include <memory>
#include <sstream>

template<typename T>
struct LinkNode {
    LinkNode* next = nullptr;
    T data;
    explicit LinkNode(T data): data(data) {}
};

template<typename T>
struct LinkHead {
    LinkNode<T>* next = nullptr;
};

template<typename T, typename Comparator = std::less<T>>
class LinkSet {
    LinkHead<T> head;
    size_t size;
    Comparator cmp;
public:
    LinkSet(): size(0), cmp(Comparator()) {}
    ~LinkSet() {
        clear();
    }

    template<typename... Args>
    explicit LinkSet(Args... args): LinkSet() {
        (insert(args), ...);
    }

    bool isEmpty() const {
        return size == 0;
    }

    template<typename F>
    void forEach(F&& f) const {
        LinkNode<T>* current = head.next;
        while (current != nullptr) {
            f(current->data);
            current = current->next;
        }
    }

    template<typename F>
    void forEachIndexed(F&& f) const {
        LinkNode<T>* current = head.next;
        for (int i = 0; current != nullptr; i++) {
            f(i, current->data);
            current = current->next;
        }
    }

    void clear() {
        if (!isEmpty()) {
            LinkNode<T> *current = head.next;
            while (current != nullptr) {
                LinkNode<T> *next = current->next;
                delete current;
                current = next;
            }
            head.next = nullptr;
            size = 0;
        }
    }

    void insertAll(LinkSet<T>& other) {
        other.forEach([this](const T& data) { insert(data); });
    }

    /**
     * 链表有序集合的插入函数
     * @param data to insert
     * @return 插入位置下标，-1为集合中已存在该元素
     */
    int insert(const T& data) {
        if (contains(data)) {
            return -1;
        }

        LinkNode<T>* newNode = new LinkNode<T>{data};
        int index = 0;
        LinkNode<T>* prev = reinterpret_cast<LinkNode<T>*>(&head);  // 结构体数据对齐 强制转换
        while (prev->next != nullptr && cmp(prev->next->data, data)) {
            prev = prev->next;
            index++;
        }
        newNode->next = prev->next;
        prev->next = newNode;
        size++;

        return index;
    }

    size_t getSize() const {
        return size;
    }

    Comparator getComparator() {
        return cmp;
    }

    LinkHead<T> getHead() {
        return head;
    }

    bool contains(const T& data) const {
        return indexOf(data) != -1;
    }

    /**
     * 查找元素在集合中的位置下标
     * @param target to find
     * @return 元素下标，若不存在返回-1
     */
    int indexOf(const T& target) const {
        int index = 0;
        for (LinkNode<T>* current = head.next;current!=nullptr;current = current->next) {
            if (!cmp(current->data, target)&&!cmp(target, current->data)) {
                return index;
            }
            index++;
        }
        return -1;
    }

    /**
     * 删除集合中的指定元素
     * @param target to remove
     * @return 被删除元素的下标，若不存在该元素则返回-1
     */
    int remove(const T& target) {
        LinkNode<T>** current = &head.next;
        int index = 0;
        while (*current != nullptr&&!(!cmp((*current)->data, target)&&!cmp(target, (*current)->data))) {
            current = &((*current)->next);
            index++;
        }
        if (*current == nullptr) {
            return -1;
        }
        LinkNode<T>* toDelete = *current;
        *current = (*current)->next;
        delete toDelete;
        size--;
        return index;
    }

    /**
     * 返回集合的字符串表示
     * @return 集合的字符串表示
     */
    [[nodiscard]] std::string toString() const {
        std::stringstream ss;
        ss << "{";
        LinkNode<T> *current = head.next;
        while (current != nullptr) {
            ss << current->data;
            if (current->next != nullptr) {
                ss << ", ";
            }
            current = current->next;
        }
        ss << "}";
        return ss.str();
    }


    /**
     * 求集合交集
     * @tparam T 集合元素类型
     * @param set1 A集合
     * @param set2 B集合
     * @return 交集
     */
    static LinkSet<T> intersectionSet(LinkSet<T> &set1, LinkSet<T> &set2) {
        LinkSet<T> result{};
        auto cmp = set1.getComparator();
        LinkNode<T>* p1 = set1.getHead().next;
        LinkNode<T>* p2 = set2.getHead().next;
        
        while (p1 != nullptr && p2 != nullptr) {
            if (cmp(p1->data, p2->data)) p1 = p1->next;
            else if (cmp(p2->data, p1->data)) p2 = p2->next;
            else {
                result.insert(p1->data);
                p1 = p1->next;
                p2 = p2->next;
            }
        }
        return result;
    }

    /**
     * 求集合并集
     * @tparam T 集合元素类型
     * @param set1 A集合
     * @param set2 B集合
     * @return 并集
     */
    static LinkSet<T> unionSet(const LinkSet<T> &set1, const LinkSet<T> &set2) {
        LinkSet<T> result{};
        LinkNode<T>* cur = set1.head.next;
        while (cur != nullptr)
        {
            result.insert(cur->data);
            cur = cur->next;
        }
        cur = set2.head.next;
        while (cur != nullptr)
        {
            result.insert(cur->data);
            cur = cur->next;
        }
        return result;
    }

    /**
     * 求集合差集
     * @tparam T 集合元素类型
     * @param set1 A集合
     * @param set2 B集合
     * @return 差集
     */
    static LinkSet<T> differenceSet(LinkSet<T> &set1, LinkSet<T> &set2) {
        LinkSet<T> result{};
        auto cmp= set1.getComparator();
        auto *p=set1.getHead().next;
        auto*q=set2.getHead().next;
        while (p!=nullptr&&q!=nullptr) {
            if (cmp(p->data,q->data)) {
                result.insert(p->data);
                p=p->next;
            } else if (cmp(q->data, p->data) )
            {
                q=q->next;
            } else {
                p=p->next;
                q=q->next;
            }

        }
        while (p!=nullptr) {
            result.insert(p->data);
            p=p->next;
        }
        return result;
    }

    /**
     * 求集合对称差集
     * @tparam T 集合元素类型
     * @param set1 A集合
     * @param set2 B集合
     * @return 对称差集
     */
    static LinkSet<T> symmetricDifference(const LinkSet<T> &set1, const LinkSet<T> &set2) {
        LinkSet<T> result{};
        auto cmp = set1.cmp;
        LinkNode<T>* p1 = set1.head.next;
        LinkNode<T>* p2 = set2.head.next;

        // 同时遍历两个有序链表
        while (p1 != nullptr && p2 != nullptr) {
            if (cmp(p1->data, p2->data)) {
                // p1->data < p2->data，只在 A 中
                result.insert(p1->data);
                p1 = p1->next;
            } else if (cmp(p2->data, p1->data)) {
                // p2->data < p1->data，只在 B 中
                result.insert(p2->data);
                p2 = p2->next;
            } else {
                // 相等：两集合共有，不加入结果
                p1 = p1->next;
                p2 = p2->next;
            }
        }

        // 剩余元素（不可能在对方集合中出现）
        while (p1 != nullptr) {
            result.insert(p1->data);
            p1 = p1->next;
        }
        while (p2 != nullptr) {
            result.insert(p2->data);
            p2 = p2->next;
        }
        return result;
    }
};

#endif