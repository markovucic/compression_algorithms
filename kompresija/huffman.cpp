#include"huffman.hpp"
#include"bitio.hpp"

#include<vector>
#include<queue>
#include<stdexcept>

using namespace std;

struct HuffmanNode {
    long long freq;
    int left, right;    // -1 kod lista
    int symbol;         // -1 kod unutrasnjeg cvora
};

// Gradi Hafmanovo stablo; cvorovi se cuvaju u vektoru, vraca se indeks korijena.
// Simboli istih frekvencija se razbijaju po indeksu cvora, pa je stablo jednoznacno
// odredjeno frekvencijama - dekoder ga gradi na isti nacin kao koder.
int build_tree(const vector<int>& freq, vector<HuffmanNode>& nodes) {
    typedef pair<long long, int> Item;
    priority_queue<Item, vector<Item>, greater<Item> > pq;

    for(int s = 0; s < 256; s++) {
        if(freq[s] > 0) {
            nodes.push_back(HuffmanNode{freq[s], -1, -1, s});
            pq.push(Item(freq[s], (int)nodes.size() - 1));
        }
    }

    while(pq.size() > 1) {
        Item a = pq.top();
        pq.pop();
        Item b = pq.top();
        pq.pop();

        nodes.push_back(HuffmanNode{a.first + b.first, a.second, b.second, -1});
        pq.push(Item(a.first + b.first, (int)nodes.size() - 1));
    }

    return pq.top().second;
}

// Dodjeljuje kodove listovima Hafmanovog stabla
void assign_codes(const vector<HuffmanNode>& nodes, int v, string& prefix,
                  vector<string>& codes) {
    if(nodes[v].left == -1) {
        codes[nodes[v].symbol] = prefix;
        return;
    }

    prefix.push_back('0');
    assign_codes(nodes, nodes[v].left, prefix, codes);
    prefix.pop_back();

    prefix.push_back('1');
    assign_codes(nodes, nodes[v].right, prefix, codes);
    prefix.pop_back();
}

// Enkoder
string huffman_encode(const string& text) {
    if(text.size() > MAX_SIZE) {
        throw runtime_error("tekst je predugacak");
    }

    // na pocetku zapisujemo kolika je duzina teksta - 
    // bitno jer duzina moze biti neporavnata s bajtom, pa u tom slucaju
    // dekoder moze da odbaci dopunjene nule
    int n = text.size();
    BitWriter out;
    out.write(n, 32);
    if(n == 0) {
        return out.bytes();
    }

    vector<int> freq(256, 0);
    for(int i = 0; i < n; i++) {
        freq[(unsigned char)text[i]]++;
    }

    int distinct = 0;
    for(int s = 0; s < 256; s++) {
        if(freq[s] > 0) {
            distinct++;
        }
    }

    // zapisivanje frkevencija simbola
    out.write(distinct - 1, 8);
    for(int s = 0; s < 256; s++) {
        if(freq[s] > 0) {
            out.write(s, 8);
            out.write(freq[s], 32);
        }
    }

    if(distinct == 1) {
        return out.bytes();
    }

    vector<HuffmanNode> nodes;
    int root = build_tree(freq, nodes);

    vector<string> codes(256);
    string prefix;
    assign_codes(nodes, root, prefix, codes);

    // pisanje koda
    for(int i = 0; i < n; i++) {
        const string& code = codes[(unsigned char)text[i]];
        for(int j = 0; j < (int)code.size(); j++) {
            out.writeBit(code[j] == '1');
        }
    }

    return out.bytes();
}

// Dekoder
string huffman_decode(const string& data) {
    BitReader in(data);

    // citanje duzine koda
    int length = in.read(32);
    if(length == 0) {
        return "";
    }

    // citanje mape frekvencija
    int distinct = in.read(8) + 1;
    vector<int> freq(256, 0);
    int last_symbol = 0;
    for(int i = 0; i < distinct; i++) {
        int s = in.read(8);
        freq[s] = in.read(32);
        last_symbol = s;
    }

    if(distinct == 1) {
        return string(length, (char)last_symbol);
    }

    vector<HuffmanNode> nodes;
    int root = build_tree(freq, nodes);

    // dekodiranje teksta
    string text;
    for(int i = 0; i < length; i++) {
        int v = root;
        while(nodes[v].left != -1) {
            if(in.readBit()) {
                v = nodes[v].right;
            } else {
                v = nodes[v].left;
            }
        }
        text.push_back((char)nodes[v].symbol);
    }

    return text;
}
