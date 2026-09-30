# Algoritmi kompresije podataka

Implementacije algoritama kompresije teksta u C++-u, uradjene uz seminarski rad iz Dizajna i analize algoritama: Hafmanovo kodiranje (`huffman`), LZ77 (`lz77`, sa naivnom i KMP pretragom poklapanja), LZ78 (`lz78`) i LZW (`lzw`). Svaki algoritam je poseban modul sa funkcijama `*_encode` i `*_decode` koje rade nad obicnim `string`-om. `bitio.hpp` sadrzi zapisivanje i citanje pojedinacnih bitova, a `trie.hpp` prefiksno drvo koje koriste LZ78 i LZW. `main.cpp` je komandna linija za kompresiju, dekompresiju i poredjenje algoritama.

```
g++ -std=c++17 -O2 -static -o kompresija main.cpp huffman.cpp lz77.cpp lz78.cpp lzw.cpp

./kompresija <algoritam> c <ulaz> <izlaz>   # kompresija (c) 
./kompresija <algoritam> d <ulaz> <izlaz>   # dekompresija (d)
./kompresija test <ulaz>                    # poredi sve algoritme na datom fajlu
```

Algoritmi su `huffman`, `lz77`, `lz77-naive`, `lz78` i `lzw`. Komanda `test` kompresuje fajl svakim algoritmom, dekompresuje ga, provjerava da se dobija isti sadrzaj i ispisuje velicinu, odnos prema originalu i vrijeme, uz entropiju fajla.
