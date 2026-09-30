#ifndef LZ77_HPP
#define LZ77_HPP

#include<string>

// Nacin traznja najduzeg poklapanja u kliznom prozoru.
//   NAIVE: za svaku pocetnu poziciju u prozoru poredi znak po znak, O(W * L)
//   KMP:   prefiks-funkcija trazenog dijela + jedan prolaz kroz prozor, O(W + L)
enum Lz77Search {NAIVE, KMP};

std::string lz77_encode(const std::string& text, Lz77Search search = KMP);
std::string lz77_decode(const std::string& data);

#endif
