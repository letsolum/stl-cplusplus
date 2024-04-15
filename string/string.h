#pragma once

#include <algorithm>
#include <cstring>
#include <istream>
#include <ostream>

class String {
    size_t capacity_ = 0;
    size_t size_ = 0;
    char* string;
    static bool substr_equals(size_t i, const String& to_find,
                              const String& where_find);

  public:
    String();
    String(char c);
    String(const char* input);
    String(const String& cur);
    String(int n, char c);
    String& operator=(String str);
    String& operator+=(const String& second);
    String& operator+=(char c);
    char& operator[](size_t index);
    const char& operator[](size_t index) const;
    size_t length() const;
    void pop_back();
    void push_back(char c);
    size_t find(const String& to_find) const;
    size_t rfind(const String& to_find) const;
    bool empty();
    String substr(int start, int count) const;
    void clear();
    void shrink_to_fit();
    char& front();
    const char& front() const;
    char& back();
    const char& back() const;
    char* data() const;
    ~String();
    int size();
    int capacity() const;
};

bool operator==(const String& first, const String& second);
bool operator!=(const String& first, const String& second);
bool operator<(const String& first, const String& second);
bool operator>(const String& first, const String& second);
bool operator<=(const String& first, const String& second);
bool operator>=(const String& first, const String& second);
String operator+(const String& first, const String& second);
std::ostream& operator<<(std::ostream& os, const String& to_print);
std::istream& operator>>(std::istream& in, String& a);
