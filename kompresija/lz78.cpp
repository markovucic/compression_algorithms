#include"lz78.hpp"
#include"bitio.hpp"
#include"trie.hpp"

#include<vector>
#include<algorithm>
#include<stdexcept>

using namespace std;

string lz78_encode(const string& text) {
    if(text.size() > MAX_SIZE) {
        throw runtime_error("tekst je predugacak");
    }

    int n = text.size();
    BitWriter out;
    out.write(n, 32);

    TrieNode* root = new TrieNode(0);   // prazna fraza
    int dict_size = 1;
    TrieNode* current = root;

    for(int i = 0; i < n; i++) {
        char c = text[i];

        // pokusavamo da se nadovezemo na trie fraza 
        if(current->children.find(c) != current->children.end()) {
            current = current->children[c];
            continue;
        }

        // ispisujemo par: indeks poznate fraze (na onoliko bitova koliko rjecnik
        // trenutno trazi) i znak c koji je ne produzava
        out.write(current->index, index_bits(dict_size));
        out.write((unsigned char)c, 8);

        current->children[c] = new TrieNode(dict_size);
        dict_size++;
        // ponovo se postavljamo na pocetak trie fraza
        current = root;
    }

    // tekst se zavrsio usred fraze koja vec postoji u rjecniku: upisujemo je
    // sa proizvoljnim bajtom, a dekoder ga odbacuje jer zna duzinu teksta.
    if(current != root) {
        out.write(current->index, index_bits(dict_size));
        out.write(0, 8);
    }

    delete root;
    return out.bytes();
}

// fraza = fraza sa indeksom `parent` + znak `last` (unos 0 je prazna fraza)
struct Lz78Entry {
    int parent;
    char last;
};

string lz78_decode(const string& data) {
    BitReader in(data);
    int length = in.read(32);

    vector<Lz78Entry> dict;
    dict.push_back(Lz78Entry{0, 0});

    string text;
    while((int)text.size() < length) {
        // indeks citamo na onoliko bitova koliko je koder koristio u ovom koraku
        int index = in.read(index_bits(dict.size()));
        char c = (char)in.read(8);

        if(index >= (int)dict.size()) {
            throw runtime_error("neispravan indeks fraze");
        }

        // ispisujemo frazu `index`: penjemo se do korijena (znakovi dolaze unazad),
        // pa dio koji smo dodali okrecemo
        int start = text.size();
        for(int i = index; i != 0; i = dict[i].parent) {
            text.push_back(dict[i].last);
        }
        reverse(text.begin() + start, text.end());
        text.push_back(c);

        dict.push_back(Lz78Entry{index, c});
    }

    // odsijecamo eventualni suvisni znak sa kraja (vidi koder)
    text.resize(length);
    return text;
}
