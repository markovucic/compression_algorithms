#include"lz77.hpp"
#include"bitio.hpp"

#include<vector>
#include<algorithm>
#include<stdexcept>

using namespace std;

const int OFFSET_BITS = 12;
const int LENGTH_BITS = 8;
const int WINDOW = (1 << OFFSET_BITS) - 1;      // najveci pomak
const int MAX_LENGTH = (1 << LENGTH_BITS) - 1;  // najveca duzina poklapanja

struct Match {
    int offset, length;
};

// Poklapanje sa pozicije i: trazimo najduzi prefiks od text[i, i+L) koji pocinje
// negdje u [i-WINDOW, i). Kod jednakih duzina bira se najranija pocetna pozicija.
Match naive_match(const string& text, int i, int L) {
    Match best = {0, 0};

    for(int s = max(0, i - WINDOW); s < i; s++) {
        int len = 0;
        while(len < L && text[s + len] == text[i + len]) {
            len++;
        }
        if(len > best.length) {
            best.offset = i - s;
            best.length = len;
        }
    }

    return best;
}

// Isto kao prefix_sufix sa predmeta (v10): pi[j] je duzina najduzeg pravog
// prefiksa koji je ujedno sufiks od s[0, j), a pi[0] = -1.
vector<int> prefix_sufix(const string& s, int start, int m) {
    vector<int> pi(m + 1);

    int i = 0, j = -1;
    pi[i] = j;

    while(i + 1 < m + 1) {
        while(j >= 0 && s[start + i] != s[start + j])
            j = pi[j];
        pi[++i] = ++j;
    }

    return pi;
}

// KMP pretraga uzorka text[i, i+L) kroz text[i-WINDOW, i+L). Nakon obrade
// znaka t, j je duzina najduzeg prefiksa uzorka koji se zavrsava u t, pa je
// pocetak tog poklapanja t-j+1. Vazi samo ako je pocetak < i, tj. j > t-i+1
// (inace je to samo uzorak sam sa sobom).
Match kmp_match(const string& text, int i, int L) {
    Match best = {0, 0};
    vector<int> pi = prefix_sufix(text, i, L);

    int j = 0;
    for(int t = max(0, i - WINDOW); t < i + L; t++) {
        while(j >= 0 && (j == L || text[t] != text[i + j]))
            j = pi[j];
        j++;

        if(j > t - i + 1 && j > best.length) {
            best.offset = i - (t - j + 1);
            best.length = j;
        }
    }

    return best;
}

string lz77_encode(const string& text, Lz77Search search) {
    if(text.size() > MAX_SIZE) {
        throw runtime_error("tekst je predugacak");
    }

    int n = text.size();
    BitWriter out;
    out.write(n, 32);

    int i = 0;
    while(i < n) {
        // Poklapanje ne smije pojesti zadnji znak, jer trojka nosi i sljedeci znak.
        int L = min(MAX_LENGTH, n - i - 1);

        Match m = {0, 0};
        if(L > 0) {
            if(search == NAIVE) {
                m = naive_match(text, i, L);
            } else {
                m = kmp_match(text, i, L);
            }
        }

        out.write(m.offset, OFFSET_BITS);
        out.write(m.length, LENGTH_BITS);
        out.write((unsigned char)text[i + m.length], 8);

        i += m.length + 1;
    }

    return out.bytes();
}

string lz77_decode(const string& data) {
    BitReader in(data);
    int length = in.read(32);

    string text;
    while((int)text.size() < length) {
        int offset = in.read(OFFSET_BITS);
        int len = in.read(LENGTH_BITS);
        char c = (char)in.read(8);

        if(offset > (int)text.size() || (len > 0 && offset == 0)) {
            throw runtime_error("neispravan pomak");
        }

        // Bajt po bajt, jer se izvor moze preklapati sa onim sto se upravo upisuje.
        int start = text.size() - offset;
        for(int k = 0; k < len; k++) {
            text.push_back(text[start + k]);
        }
        text.push_back(c);
    }

    text.resize(length);
    return text;
}
