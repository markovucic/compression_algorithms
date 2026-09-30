#ifndef TRIE_HPP
#define TRIE_HPP

#include<unordered_map>

struct TrieNode {
    TrieNode(int index): index(index) {}
    ~TrieNode() {
        for(auto child : children) {
            delete child.second;
        }
    }

    int index;
    std::unordered_map<char, TrieNode*> children;
};

#endif
