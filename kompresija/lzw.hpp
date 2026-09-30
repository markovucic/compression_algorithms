#ifndef LZW_HPP
#define LZW_HPP

#include<string>

std::string lzw_encode(const std::string& text);
std::string lzw_decode(const std::string& data);

#endif
