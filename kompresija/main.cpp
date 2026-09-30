#include"huffman.hpp"
#include"lz77.hpp"
#include"lz78.hpp"
#include"lzw.hpp"

#include<iostream>
#include<fstream>
#include<sstream>
#include<iomanip>
#include<cmath>
#include<ctime>
#include<vector>
#include<string>

using namespace std;

string encode(const string& algorithm, const string& text) {
    if(algorithm == "huffman")
        return huffman_encode(text);
    if(algorithm == "lz77")
        return lz77_encode(text, KMP);
    if(algorithm == "lz77-naive")
        return lz77_encode(text, NAIVE);
    if(algorithm == "lz78")
        return lz78_encode(text);
    if(algorithm == "lzw")
        return lzw_encode(text);
    throw runtime_error("nepoznat algoritam: " + algorithm);
}

string decode(const string& algorithm, const string& data) {
    if(algorithm == "huffman")
        return huffman_decode(data);
    if(algorithm == "lz77" || algorithm == "lz77-naive")
        return lz77_decode(data);
    if(algorithm == "lz78")
        return lz78_decode(data);
    if(algorithm == "lzw")
        return lzw_decode(data);
    throw runtime_error("nepoznat algoritam: " + algorithm);
}

string read_file(const string& path) {
    ifstream file(path.c_str(), ios::binary);
    if(!file) {
        throw runtime_error("ne mogu da otvorim " + path);
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void write_file(const string& path, const string& content) {
    ofstream file(path.c_str(), ios::binary);
    if(!file) {
        throw runtime_error("ne mogu da pisem u " + path);
    }
    file << content;
}

// Entropija H(X) u bitovima po bajtu, za empirijsku raspodjelu bajtova.
double entropy(const string& text) {
    vector<int> count(256, 0);
    for(int i = 0; i < (int)text.size(); i++) {
        count[(unsigned char)text[i]]++;
    }

    double h = 0;
    for(int c = 0; c < 256; c++) {
        if(count[c] > 0) {
            double p = (double)count[c] / text.size();
            h -= p * log2(p);
        }
    }
    return h;
}

// Uslovna entropija H(X_{k+1} | X_k) u bitovima po bajtu, za empirijsku raspodjelu
// parova susjednih bajtova: -sum p(a,b) * log2(p(a,b) / p(a)).
double conditional_entropy(const string& text) {
    int n = text.size();
    if(n < 2) {
        return 0;
    }

    vector<int> pairs(256 * 256, 0);
    vector<int> first(256, 0);
    for(int i = 0; i + 1 < n; i++) {
        int a = (unsigned char)text[i];
        int b = (unsigned char)text[i + 1];
        pairs[a * 256 + b]++;
        first[a]++;
    }

    double h = 0;
    for(int a = 0; a < 256; a++) {
        for(int b = 0; b < 256; b++) {
            int k = pairs[a * 256 + b];
            if(k > 0) {
                h -= (double)k / (n - 1) * log2((double)k / first[a]);
            }
        }
    }
    return h;
}

// Kompresuje svakim algoritmom, dekompresuje, provjerava tacnost i ispisuje
// odnos velicina i vrijeme kompresije.
void test(const string& path) {
    string text = read_file(path);
    cout << "Ulaz: " << path << " (" << text.size() << " B)" << endl;

    if(text.empty()) {
        cout << "prazan fajl" << endl;
        return;
    }

    cout << fixed << setprecision(3);
    cout << "Entropija H(X): " << entropy(text) << " bit/bajt, uslovna H(X2|X1): "
         << conditional_entropy(text) << " bit/bajt" << endl;

    string algorithms[] = {"huffman", "lz77", "lz77-naive", "lz78", "lzw"};
    for(int i = 0; i < 5; i++) {
        string name = algorithms[i];

        clock_t start = clock();
        string packed = encode(name, text);
        double ms = (clock() - start) * 1000.0 / CLOCKS_PER_SEC;

        bool ok = decode(name, packed) == text;

        cout << left << setw(11) << name << right
             << setw(9) << packed.size() << " B  "
             << setw(7) << 8.0 * packed.size() / text.size() << " bit/bajt  "
             << setw(7) << 100.0 * packed.size() / text.size() << "%  "
             << setw(9) << ms << " ms  "
             << (ok ? "OK" : "GRESKA") << endl;
    }
}

int main(int argc, char* argv[]) {
    try {
        string command = "";
        if(argc > 1) {
            command = argv[1];
        }

        if(command == "test" && argc == 3) {
            test(argv[2]);
            return 0;
        }

        if(argc == 5 && (string(argv[2]) == "c" || string(argv[2]) == "d")) {
            string input = read_file(argv[3]);
            string output;
            if(string(argv[2]) == "c") {
                output = encode(command, input);
            } else {
                output = decode(command, input);
            }
            write_file(argv[4], output);
            return 0;
        }

        cerr << "Upotreba:\n"
             << "  " << argv[0] << " <algoritam> c|d <ulaz> <izlaz>   (c = kompresija, d = dekompresija)\n"
             << "  " << argv[0] << " test <ulaz>                      (poredjenje algoritama)\n"
             << "Algoritmi: huffman lz77 lz77-naive lz78 lzw" << endl;
        return 1;
    } catch(const exception& e) {
        cerr << "Greska: " << e.what() << endl;
        return 1;
    }
}
