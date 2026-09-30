#include"lzw.hpp"
#include"bitio.hpp"
#include"trie.hpp"

#include<vector>
#include<algorithm>
#include<stdexcept>

using namespace std;

string lzw_encode(const string& text) {
    if(text.size() > MAX_SIZE) {
        throw runtime_error("tekst je predugacak");
    }

    int n = text.size();
    BitWriter out;
    out.write(n, 32);

    // koren nema svoj kod; njegova djeca su fraze od jednog bajta (kodovi 0-255)
    TrieNode* root = new TrieNode(-1);
    for(int b = 0; b < 256; b++) {
        root->children[(char)b] = new TrieNode(b);
    }
    int dict_size = 256;

    if(n > 0) {
        TrieNode* current = root->children[text[0]];

        for(int i = 1; i < n; i++) {
            char c = text[i];

            if(current->children.find(c) != current->children.end()) {
                current = current->children[c];
                continue;
            }

            // Upisuje se samo kod fraze; znak c ne ide u izlaz, on postaje
            // prvi znak sljedece fraze.
            out.write(current->index, index_bits(dict_size));
            current->children[c] = new TrieNode(dict_size);
            dict_size++;
            current = root->children[c];
        }

        out.write(current->index, index_bits(dict_size));
    }

    delete root;
    return out.bytes();
}

// Fraza i = fraza `parent` + znak `last`; osnovni unosi (bajtovi) nemaju roditelja.
struct LzwEntry {
    int parent;
    char last;
};

// Fraza sa datim kodom, dobijena penjanjem do korijena.
string expand(const vector<LzwEntry>& dict, int code) {
    string phrase;
    for(int i = code; i != -1; i = dict[i].parent) {
        phrase.push_back(dict[i].last);
    }
    reverse(phrase.begin(), phrase.end());
    return phrase;
}

string lzw_decode(const string& data) {
    BitReader in(data);
    int length = in.read(32);

    vector<LzwEntry> dict;
    for(int b = 0; b < 256; b++) {
        dict.push_back(LzwEntry{-1, (char)b});
    }

    string text;
    // kod prethodne fraze (-1 dok nismo procitali nijedan kod)
    int prev = -1;
    int k = 0;   // broj dosad procitanih kodova
    while((int)text.size() < length) {
        // pri k-tom kodu koder ima 256 + k unosa u rjecniku, pa toliko bitova citamo
        int code = in.read(index_bits(256 + k));
        k++;

        string phrase;
        if(prev == -1) {
            if(code >= 256) {
                throw runtime_error("neispravan kod");
            }
            phrase = expand(dict, code);
        } else {
            if(code < (int)dict.size()) {
                phrase = expand(dict, code);
            } else if(code == (int)dict.size()) {
                // Kod fraze koju koder upravo dodaje, a dekoder je jos nema:
                // to je prethodna fraza + njen prvi znak.
                phrase = expand(dict, prev);
                phrase.push_back(phrase[0]);
            } else {
                throw runtime_error("neispravan kod");
            }
            // nova fraza = prethodna fraza + prvi znak trenutne (koder ju je dodao
            // jos prije nego sto je poslao ovaj kod)
            dict.push_back(LzwEntry{prev, phrase[0]});
        }

        text += phrase;
        prev = code;
    }

    text.resize(length);
    return text;
}
