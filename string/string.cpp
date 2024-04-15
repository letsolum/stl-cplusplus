#include "string.h"

String::String() : capacity_(0), string(new char[capacity_ + 1]) {
    string[0] = '\0';
}

String::String(char c)
    : capacity_(1), size_(1), string(new char[capacity_ + 1]) {
    string[0] = c;
    string[1] = '\0';
}

String::String(const char* input)
    : capacity_(strlen(input) + 1),
      size_(strlen(input)),
      string(new char[capacity_ + 1]) {
    std::copy(input, input + size_, string);
    string[size_] = '\0';
}

String::String(int n, char c)
    : capacity_(n + 1), size_(n), string(new char[capacity_ + 1]) {
    std::fill(string, string + size_, c);
    string[size_] = '\0';
}

String::String(const String& cur)
    : capacity_(cur.capacity()),
      size_(cur.length()),
      string(new char[capacity_ + 1]) {
    std::copy(cur.data(), cur.data() + size_, string);
}

size_t String::length() const {
    return size_;
}

char& String::operator[](size_t index) {
    return string[index];
}

const char& String::operator[](size_t index) const {
    return string[index];
}

bool operator==(const String& first, const String& second) {
    if (first.length() != second.length()) {
        return false;
    }
    return strcmp(first.data(), second.data()) == 0;
}

bool operator!=(const String& first, const String& second) {
    return !(first == second);
}

bool operator<(const String& first, const String& second) {
    return strcmp(first.data(), second.data()) < 0;
}

bool operator>(const String& first, const String& second) {
    return second < first;
}

bool operator<=(const String& first, const String& second) {
    return !(first > second);
}

bool operator>=(const String& first, const String& second) {
    return !(first < second);
}

String& String::operator=(String str) {
    std::swap(size_, str.size_);
    std::swap(capacity_, str.capacity_);
    std::swap(string, str.string);
    return *this;
}

String& String::operator+=(char c) {
    push_back(c);
    return *this;
}

String& String::operator+=(const String& second) {
    if (capacity_ < size_ + second.length()) {
        capacity_ = size_ + second.length();
        char* new_string = new char[capacity_ + 1];
        std::copy(string, string + size_, new_string);
        std::copy(second.string, second.string + second.length(),
                  new_string + size_);
        std::swap(string, new_string);
        delete[] new_string;
        size_ = capacity_;
    } else {
        std::copy(second.string, second.string + second.length(),
                  string + size_);
        size_ += second.length();
    }
    string[size_] = '\0';
    return *this;
}

String operator+(const String& first, const String& second) {
    String answer(first);
    answer += second;
    return answer;
}

void String::pop_back() {
    --size_;
    string[size_] = '\0';
}

void String::push_back(char c) {
    if (size_ == capacity_) {
        capacity_ += std::max(static_cast<size_t>(1), capacity_);
        char* new_string = new char[capacity_ + 1];
        std::copy(string, string + size_, new_string);
        std::swap(string, new_string);
        delete[] new_string;
    }
    string[size_] = c;
    string[++size_] = '\0';
}

bool String::empty() {
    return size_ == 0;
}

bool String::substr_equals(size_t i, const String& to_find,
                           const String& where_find) {
    return memcmp(to_find.data(), where_find.data() + i, to_find.length()) == 0;
}

size_t String::find(const String& to_find) const {
    if (to_find.length() > size_) {
        return size_;
    }
    for (size_t i = 0; i < (size_ - to_find.length() + 1); ++i) {
        int cur_answer = static_cast<int>(substr_equals(i, to_find, *this));
        if (cur_answer != 0) {
            return i;
        }
    }
    return size_;
}

size_t String::rfind(const String& to_find) const {
    for (size_t i = size_ - to_find.length(); i != 0; --i) {
        int cur_answer = static_cast<int>(substr_equals(i, to_find, *this));
        if (cur_answer != 0) {
            return i;
        }
    }
    return size_;
}

String String::substr(int start, int count) const {
    String answer(count, ' ');
    std::copy(string + start, string + start + count, answer.string);
    return answer;
}

void String::clear() {
    string[0] = '\0';
    size_ = 0;
}

void String::shrink_to_fit() {
    capacity_ = size_;
    char* new_string = new char[capacity_ + 1];
    std::copy(string, string + size_, new_string);
    std::swap(string, new_string);
    delete[] new_string;
}

char* String::data() const {
    return string;
}

char& String::front() {
    return string[0];
}

const char& String::front() const {
    return string[0];
}

char& String::back() {
    return string[size_ - 1];
}

const char& String::back() const {
    return string[size_ - 1];
}

std::ostream& operator<<(std::ostream& os, const String& to_print) {
    return os << to_print.data();
}

std::istream& operator>>(std::istream& in, String& a) {
    a.clear();
    char c;
    while (in.get(c) && !in.eof() && std::isspace(c) != 0) {}
    if (in.eof()) {
        return in;
    }
    a += c;
    while (in.get(c) && !in.eof() && std::isspace(c) == 0) {
        a += c;
    }
    return in;
}

String::~String() {
    delete[] string;
}

int String::size() {
    return size_;
}

int String::capacity() const {
    return capacity_;
}
