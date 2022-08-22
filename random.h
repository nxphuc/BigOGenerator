#ifndef _BIGO_TEST_GENERATOR_H_
#define _BIGO_TEST_GENERATOR_H_

#define VERSION "0.1.0"

#ifdef _MSC_VER
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NO_VA_START_VALIDATION
#endif

#if (_WIN32 || __WIN32__ || __WIN32 || _WIN64 || __WIN64__ || __WIN64 || WINNT || __WINNT || __WINNT__ || __CYGWIN__)
#define I64 "%I64d"
#define U64 "%I64u"
#else
#define I64 "%lld"
#define U64 "%llu"
#endif

#include <cstdio>
#include <cctype>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <limits>
#include <stdarg.h>
#include <fcntl.h>
#include <functional>
#include <chrono>

template <typename... Args>
std::string __format(const char *format, Args... args) {
    int32_t size_s = std::snprintf( nullptr, 0, format, args ... ) + 1;
    if (size_s <= 0 ) {
        std::fprintf(stderr,"Error during formatting.");
        exit(0);
    }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format, args ... );
    return std::string( buf.get(), buf.get() + size - 1 );
}

static void __bigo_generator_fail(const char *fmt, ...) {
    va_list pvar;
    va_start(pvar, fmt);
    std::vfprintf(stderr, fmt, pvar);
    exit(0);
}

static void __bigo_generator_fail(const std::string& msg) {
    __bigo_generator_fail("%s\n", msg.data());
}

// https://en.cppreference.com/w/cpp/types/enable_if
template <bool B, class T = void>
struct __bigo_enable_if {};

template <class T>
struct __bigo_enable_if<true, T> {
    typedef T type;
};

template <class Dest, class Source>
Dest __bit_cast(const Source& source) {
    static_assert(sizeof(Dest) == sizeof(Source),
                "__bit_cast requires source and destination to be the same size");
    static_assert(std::is_trivially_copyable<Dest>::value,
                "__bit_cast requires the destination type to be copyable");
    static_assert(std::is_trivially_copyable<Source>::value,
                "__bit_cast requires the source type to be copyable");
    Dest dest;
    memcpy(&dest, &source, sizeof(dest));
    return dest;
}

constexpr inline bool __is_power_of_two(int32_t n) { return (n & -n) == n; }

// https://stackoverflow.com/a/29634934
// To allow ADL with custom begin/end
using std::begin;
using std::end;

template <typename T>
auto is_iterable_impl(int) -> decltype(
    begin(std::declval<T &>()) != end(std::declval<T &>()),   // begin/end and operator !=
    void(),                                                   // Handle evil operator ,
    ++std::declval<decltype(begin(std::declval<T &>())) &>(), // operator ++
    void(*begin(std::declval<T &>())),                        // operator*
    std::true_type{}
);

template <typename T>
std::false_type is_iterable_impl(...);

/* check if variable type is iterable */
// https://stackoverflow.com/a/29634934
template <typename T>
using is_iterable = decltype(is_iterable_impl<T>(0));

/* check if pointer is iterator */
// https://stackoverflow.com/a/4336298
template<class T, class Enable = void>
struct is_iterator {
    static T makeT();

    typedef void* twoptrs[2];

    static twoptrs& test(...);

    template<class R>
    static typename R::iterator_category* test(R);

    template<class R>
    static void* test(R*);

    static const bool value = sizeof(test(makeT())) == sizeof(void*);
};

template<class T>
struct is_iterator<T, typename __bigo_enable_if<std::is_array<T>::value>::type> {
    static const bool value = false;
};

template <typename T>
using is_string_like = std::is_same<std::string, std::decay_t<T>>;

/* check if iterator is a pair */
template <typename T>
struct is_pair : std::false_type { };

template <typename T, typename U>
struct is_pair<std::pair<T, U>> : std::true_type { };

template <typename T, typename U>
struct is_pair<const std::pair<T, U>> : std::true_type { };

template <typename T, typename U>
struct is_pair<std::pair<T, U>&> : std::true_type { };

template <typename T, typename U>
struct is_pair<const std::pair<T, U>&> : std::true_type { };

template <typename T>
typename __bigo_enable_if<!(is_iterable<T>::value || is_pair<T>::value), void>::type
__print_one(const T &t) {
    std::cout << t;
}

template <typename T>
typename __bigo_enable_if<is_iterable<T>::value, void>::type
__print_one(const T &t) {
    for (typename T::const_iterator it = t.begin(); it != t.end(); it++) {
        if (it != t.begin()) {
            if (is_string_like<decltype(*it)>::value) {
                std::cout << ' ';
            } else {
                std::cout << " \n"[(is_iterable<decltype(*it)>::value)];
            }
        }
        __print_one(*it);
    }
}

template <size_t N>
void __print_one(const char (&t)[N]) {
    std::cout << t;
}

template <>
typename __bigo_enable_if<is_iterable<std::string>::value, void>::type
__print_one<std::string>(const std::string &t) {
    std::cout << t;
}

template <typename T>
typename __bigo_enable_if<is_pair<T>::value, void>::type
__print_one(const T &p) {
    __print_one(p.first);
    std::cout << ' ';
    __print_one(p.second);
}

template<typename A, typename B>
void __print_range(A begin, B end) {
    bool first = true;
    for (B it = B(begin); it != end; it++) {
        if (first) {
            first = false;
        } else {
            if (is_string_like<decltype(*it)>::value) {
                std::cout << ' ';
            } else {
                std::cout << " \n"[(is_iterable<decltype(*it)>::value)];
            }
        }
        __print_one(*it);
    }
}

static bool __is_command_char(const std::string &s, size_t pos, char value) {
    if (pos >= s.length())
        return false;

    int32_t slashes = 0;

    int32_t before = int32_t(pos) - 1;
    while (before >= 0 && s[before] == '\\')
        before--, slashes++;

    return slashes % 2 == 0 && s[pos] == value;
}

static bool __is_slash(const std::string &s, size_t pos) {
    return s[pos] == '\\';
}

static char __get_char(const std::string &s, size_t &pos) {
    if (__is_slash(s, pos)) {
        pos += 2;
    } else {
        pos++;
    }

    return s[pos - 1];
}

static std::vector<char> __scan_char_set(const std::string &s, size_t &pos) {
    if (pos >= s.length())
        __bigo_generator_fail("Pattern: Illegal pattern (or part) \"" + s + "\"");

    std::vector<char> result;

    if (__is_command_char(s, pos, '[')) {
        pos++;
        bool negative = __is_command_char(s, pos, '^');
        if (negative)
            pos++;

        char prev = 0;

        while (pos < s.length() && !__is_command_char(s, pos, ']')) {
            if (__is_command_char(s, pos, '-') && prev != 0) {
                pos++;

                if (pos + 1 == s.length() || __is_command_char(s, pos, ']')) {
                    result.push_back(prev);
                    prev = '-';
                    continue;
                }

                char next = __get_char(s, pos);
                if (prev > next)
                    __bigo_generator_fail("Pattern: Illegal pattern (or part) \"" + s + "\"");

                for (char c = prev; c != next; c++)
                    result.push_back(c);
                result.push_back(next);

                prev = 0;
            } else {
                if (prev != 0)
                    result.push_back(prev);
                prev = __get_char(s, pos);
            }
        }

        if (prev != 0)
            result.push_back(prev);

        if (!__is_command_char(s, pos, ']'))
            __bigo_generator_fail("Pattern: Illegal pattern (or part) \"" + s + "\"");

        pos++;

        if (negative) {
            std::sort(result.begin(), result.end());
            std::vector<char> actuals;
            for (int32_t code = 0; code < 255; code++) {
                char c = char(code);
                if (!std::binary_search(result.begin(), result.end(), c))
                    actuals.push_back(c);
            }
            result = actuals;
        }

        std::sort(result.begin(), result.end());
    } else
        result.push_back(__get_char(s, pos));

    return result;
}

static void __scan_counts(const std::string &s, size_t &pos, int32_t &from, int32_t &to) {
    if (pos >= s.length()) {
        from = to = 1;
        return;
    }

    if (__is_command_char(s, pos, '{')) {
        std::vector<std::string> parts;
        std::string part;

        pos++;

        while (pos < s.length() && !__is_command_char(s, pos, '}')) {
            if (__is_command_char(s, pos, ',')) {
                parts.push_back(part);
                part = "";
                pos++;
            } else {
                part += __get_char(s, pos);
            }
        }

        if (part != "")
            parts.push_back(part);

        if (!__is_command_char(s, pos, '}'))
            __bigo_generator_fail("Pattern: Illegal pattern (or part) \"" + s + "\"");

        pos++;

        if (parts.size() < 1 || parts.size() > 2)
            __bigo_generator_fail("Pattern: Illegal pattern (or part) \"" + s + "\"");

        std::vector<int> numbers;

        for (const auto& part : parts) {
            if (part.length() == 0)
                __bigo_generator_fail("Pattern: Illegal pattern (or part) \"" + s + "\"");
            int32_t number;
            if (std::sscanf(part.data(), "%d", &number) != 1)
                __bigo_generator_fail("Pattern: Illegal pattern (or part) \"" + s + "\"");
            numbers.push_back(number);
        }

        if (numbers.size() == 1) {
            from = to = numbers[0];
        } else {
            from = numbers[0], to = numbers[1];
        }

        if (from > to)
            __bigo_generator_fail("Pattern: Illegal pattern (or part) \"" + s + "\"");
    } else {
        if (__is_command_char(s, pos, '?')) {
            from = 0, to = 1, pos++;
            return;
        }

        if (__is_command_char(s, pos, '*')) {
            from = 0, to = INT_MAX, pos++;
            return;
        }

        if (__is_command_char(s, pos, '+')) {
            from = 1, to = INT_MAX, pos++;
            return;
        }

        from = to = 1;
    }
}

static int32_t __greedy_match(const std::string &s, size_t pos, const std::vector<char> chars) {
    int32_t result = 0;

    while (pos < s.length()) {
        char c = s[pos++];
        if (!std::binary_search(chars.begin(), chars.end(), c)) {
            break;
        } else {
            result++;
        }
    }

    return result;
}

#define CHECK_OP(a, b, OP, msg) if (!(a OP b)) __bigo_generator_fail(msg)
#define CHECK_EQ(a, b) CHECK_OP(a, b, ==, __format("BIG-O RANDOM ERROR: %d is not equal to %d", a, b))
#define CHECK_LT(a, b) CHECK_OP(a, b, <, __format("BIG-O RANDOM ERROR: %d is not less than %d", a, b))
#define CHECK_GT(a, b) CHECK_OP(a, b, >, __format("BIG-O RANDOM ERROR: %d is not greater than %d", a, b))
#define CHECK_NEQ(a, b) CHECK_OP(a, b, !=, __format("BIG-O RANDOM ERROR: %d is equal to %d", a, b))
#define CHECK_LEQ(a, b) CHECK_OP(a, b, <= __format("BIG-O RANDOM ERROR: %d is not less than or equal to %d", a, b))
#define CHECK_GEQ(a, b) CHECK_OP(a, b, >=, __format("BIG-O RANDOM ERROR: %d is not greater than or equal to %d", a, b))

#define MSG_CHECK_EQ(a, b, msg, ...) CHECK_OP(a, b, ==, __format(msg __VA_OPT__(,) __VA_ARGS__))
#define MSG_CHECK_LT(a, b, msg, ...) CHECK_OP(a, b, <, __format(msg __VA_OPT__(,) __VA_ARGS__))
#define MSG_CHECK_GT(a, b, msg, ...) CHECK_OP(a, b, >, __format(msg __VA_OPT__(, ) __VA_ARGS__))
#define MSG_CHECK_NEQ(a, b, msg, ...) CHECK_OP(a, b, !=, __format(msg __VA_OPT__(,) __VA_ARGS__))
#define MSG_CHECK_LEQ(a, b, msg, ...) CHECK_OP(a, b, <=, __format(msg __VA_OPT__(,) __VA_ARGS__))
#define MSG_CHECK_GEQ(a, b, msg, ...) CHECK_OP(a, b, >=, __format(msg __VA_OPT__(,) __VA_ARGS__))

class Random;

class Pattern {
private:
    int32_t from_;
    int32_t to_;
    std::string source_;
    std::vector<Pattern> children_;
    std::vector<char> chars_;

    bool Matches(const std::string& s, size_t pos) const;
public:
    /* Create pattern instance by string pattern */
    Pattern(std::string pattern);

    /* Generate new string by pattern and given Random. */
    std::string Next(Random &rnd) const;

    /* Checks if given string s match the pattern. */
    bool Matches(const std::string &s) const;

    /* Returns source string of the pattern. */
    std::string Source() const;
};

class Random {
/*
https://v8.dev/blog/math-random
*/
private:
    static const int32_t kMaxRangeLimit = 10000000;
    static const int32_t kMaxLoopWeight = 20;
    static const uint64_t kExponentBits = uint64_t{0x3FF0000000000000};
    static const int64_t kMultiplier = 0x5'deec'e66dLL;
    static const int64_t kAddend = 0xbLL;
    static const int64_t kMask = 0xffff'ffff'ffffLL;

    uint64_t state0_;
    uint64_t state1_;
    uint64_t initial_seed_;

    // https://vigna.di.unimi.it/ftp/papers/xorshiftplus.pdf
    static void XorShift128(uint64_t *state0, uint64_t *state1);
    static double ToDouble(uint64_t state0);
    static uint64_t MurmurHash3(uint64_t h);
    static uint64_t ComputeSeed(uint64_t initial_seed, const char* str);

    uint64_t NextBits(int32_t bits);
    

    template<typename T>
    T WeightedNext(T max, int32_t weight);
public:
    Random();
    Random(uint64_t seed);
    Random(const char* str);
    Random(const std::string& str);
    Random(const int32_t argc, const char *argv[]);

    int64_t initial_seed() const { return initial_seed_; }
    void SetSeed(uint64_t seed);

    /* Returns random 32-bit integer value in range [`0`, `max`) */
    int32_t Next(int32_t max);

    /* Returns random 32-bit integer value in range [`min`, `max`] (inclusive) */
    int32_t Next(int32_t min, int32_t max);

    /* Returns random 64-bit integer value in range [`0`, `max`) */
    int64_t Next(int64_t max);

    /* Returns random 64-bit integer value in range [`min`, `max`] (inclusive) */
    int64_t Next(int64_t min, int64_t max);

    /* Returns random double value in range [`0`, `1`) */
    double Next();

    /* Returns random double value in range [`0`, `max`) */
    double Next(double max);

    /* Returns random double value in range [`min`, `max`) */
    double Next(double min, double max);

    /* Returns random element from container. */
    template <typename Container>
    typename Container::value_type Any(const Container &c);

    /* Returns random element from iterator range. */
    template <typename RandomAccessIterator>
    typename RandomAccessIterator::value_type Any(const RandomAccessIterator &first, const RandomAccessIterator &last);

    /* Returns random permutation of [`0`, `size - 1`]*/
    template <typename T>
    std::vector<T> Permutation(T size);

    /* Returns random permutation of [`first`, `first` + `size` - 1] */
    template <typename T, typename E>
    std::vector<E> Permutation(T size, E first);

    /* Returns a set of `size` NextSet numbers in range [`min`, `max`]. */
    template <typename T>
    std::vector<T> NextSet(int32_t size, T min, T max);

    /* Returns a set of `size` NextSet numbers in range [`0`, `max`). */
    template <typename T>
    std::vector<T> NextSet(int32_t size, T max);

    /* Returns random string value by given `pattern` */
    std::string NextString(const std::string& pattern);

    /* Rearranges the elements in the range [`first`, `last`) randomly */
    template <class RandomAccessIterator>
    void Shuffle(RandomAccessIterator first, RandomAccessIterator last);

    /**
     * Returns edges list of a tree has `size` vertices, index from `first`
     */
    std::vector<std::pair<int32_t, int32_t>> GenerateTree(int32_t size, int32_t first = 0, int32_t weight = 0);

    /**
     * Generate a rooted tree has `root`, index from `0`.
     * Returns vector parent with parent[`i`] is the parent of vetex (`i`),
     * parent of `root` is `-1`
     * */
    std::vector<int32_t> GenerateRootedTree(int32_t size, int root = 0, int32_t weight = 0);

    /**
     * Returns random 32-bit integer value in range [`0`, `max`)
     * If `weight` = 0 then it is Next()
     * If `weight` > 0, returns maximum value of `weight + 1` times Next()
     * If `weight` < 0, returns minimum value
     * */
    int32_t WeightedNext(int32_t max, int32_t weight);

    /**
     * Returns random 32-bit integer value in range [`min`, `max`] (inclusive)
     * If `weight` = 0 then it is Next()
     * If `weight` > 0, returns maximum value of `weight + 1` times Next()
     * If `weight` < 0, returns minimum value
     * */
    int32_t WeightedNext(int32_t min, int32_t max, int32_t weight);

    /**
     * Returns random 64-bit integer value in range [`0`, `max`)
     * If `weight` = 0 then it is Next()
     * If `weight` > 0, returns maximum value of `weight + 1` times Next()
     * If `weight` < 0, returns minimum value
     * */
    int64_t WeightedNext(int64_t max, int32_t weight);

    /**
     * Returns random 64-bit integer value in range [`min`, `max`] (inclusive)
     * If `weight` = 0 then it is Next()
     * If `weight` > 0, returns maximum value of `weight + 1` times Next()
     * If `weight` < 0, returns minimum value
     * */
    int64_t WeightedNext(int64_t min, int64_t max, int32_t weight);

    /**
     * Returns random double value in range [`0`, `1`)
     * If `weight` = 0 then it is Next()
     * If `weight` > 0, returns maximum value of `weight + 1` times Next()
     * If `weight` < 0, returns minimum value
     * */
    double WeightedNext(int32_t weight);

    /**
     * Returns random double value in range [`0`, `max`)
     * If `weight` = 0 then it is Next()
     * If `weight` > 0, returns maximum value of `weight + 1` times Next()
     * If `weight` < 0, returns minimum value
     * */
    double WeightedNext(double max, int32_t weight);

    /**
     * Returns random double value in range [`min`, `max`)
     * If `weight` = 0 then it is Next()
     * If `weight` > 0, returns maximum value of `weight + 1` times Next()
     * If `weight` < 0, returns minimum value
     * */
    double WeightedNext(double min, double max, int32_t weight);

    /**
     * Returns random element from container
     * If `weight` = 0 then it is Any()
     * If `weight` > 0, returns maximum value of `weight + 1` times Next()
     * If `weight` < 0, returns minimum value
     * */
    template <typename Container>
    typename Container::value_type WeightedAny(const Container &c, int32_t weight);

    /**
     * Returns random element from iterator range
     * If `weight` = 0 then it is Any()
     * If `weight` > 0, returns maximum value of `weight + 1` times Next()
     * If `weight` < 0, returns minimum value
     * */
    template <typename RandomAccessIterator>
    typename RandomAccessIterator::value_type WeightedAny(const RandomAccessIterator &first, const RandomAccessIterator &last, int32_t weight);

    // TODO CONSIDER
    // uint32_t next(uint32_t max);
    // uint32_t next(uint32_t min, uint32_t max);
    // uint64_t next(uint64_t max);
    // uint64_t next(uint64_t min, uint64_t max);
};

Random::Random() {
    this->SetSeed(std::chrono::steady_clock::now().time_since_epoch().count());
}

Random::Random(uint64_t seed) {
    this->SetSeed(seed);
}

Random::Random(const char *str){
    uint64_t seed = ComputeSeed(3203000719597029781LL, str);
    this->SetSeed(seed);
}

Random::Random(const std::string &str) {
    uint64_t seed = ComputeSeed(3203000719597029781LL, str.data());
    this->SetSeed(seed);
}

Random::Random(const int32_t argc, const char *argv[]){
    uint64_t seed = 3203000719597029781LL;
    for (int i = 0; i < argc; i++)
        seed = ComputeSeed(seed, argv[i]);
    this->SetSeed(seed);
}

uint64_t Random::ComputeSeed(uint64_t initial_seed, const char *str) {
    uint64_t seed = initial_seed;
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++)
        seed = seed * kMultiplier + str[i] + kAddend;
    return seed + kMultiplier / kAddend;
}

inline void Random::XorShift128(uint64_t *state0, uint64_t *state1) {
    uint64_t s1 = *state0;
    uint64_t s0 = *state1;
    *state0 = s0;
    s1 ^= s1 << 23;
    s1 ^= s1 >> 17;
    s1 ^= s0;
    s1 ^= s0 >> 26;
    *state1 = s1;
}

inline double Random::ToDouble(uint64_t state0) {
    // Exponent for double values for [1.0 .. 2.0)
    uint64_t random = (state0 >> 12) | kExponentBits;
    return __bit_cast<double>(random) - 1;
}

void Random::SetSeed(uint64_t seed) {
    initial_seed_ = seed;
    state0_ = MurmurHash3(__bit_cast<uint64_t>(seed));
    state1_ = MurmurHash3(~state0_);
    if (0ULL == state0_ && 0ULL == state1_)
        throw "Unexpedted, seed equal to zero";
}

// https://en.wikipedia.org/wiki/MurmurHash
uint64_t Random::MurmurHash3(uint64_t h) {
    h ^= h >> 33;
    h *= uint64_t{0xFF51AFD7ED558CCD};
    h ^= h >> 33;
    h *= uint64_t{0xC4CEB9FE1A85EC53};
    h ^= h >> 33;
    return h;
}

uint64_t Random::NextBits(int32_t bits) {
    XorShift128(&state0_, &state1_);
    return static_cast<uint64_t>((state0_ + state1_) >> (64 - bits));
}

int32_t Random::Next(int32_t max) {
    MSG_CHECK_LT(0, max, "Random::Next(int32_t max): max must be positive");


    if (__is_power_of_two(max))
        return static_cast<int>((max * static_cast<int64_t>(NextBits(31))) >> 31);

    int32_t rnd, limit = INT32_MAX / max * max;
    do {
        rnd = this->NextBits(31);
    } while (rnd >= limit);

    return rnd % max;
}

int32_t Random::Next(int32_t min, int32_t max) {
    if (min < 0 && min + INT32_MAX - 1 > max) {
        __bigo_generator_fail(
            "Random::Next(int32_t min, int32_t max): difference between max and min is too large: %d - %d", min, max);
    }
    return (int32_t)(this->Next((int64_t)max - min + 1)) + min;
}

int64_t Random::Next(int64_t max) {
    MSG_CHECK_LT(0LL, max, "Random::Next(int64_t max): max must be positive");

    int64_t rnd, limit = INT64_MAX / max * max;
    do {
        rnd = this->NextBits(63);
    } while (rnd >= limit);

    return rnd % max;
}

int64_t Random::Next(int64_t min, int64_t max) {
    if (min < 0 && min + INT64_MAX - 1 > max)
        __bigo_generator_fail("Random::Next(int64_t min, int64_t max): difference between max and min is too large: " I64 " - " I64, min, max);

    return this->Next(max - min + 1) + min;
}

double Random::Next() {
    XorShift128(&state0_, &state1_);

    return Random::ToDouble(state0_);
}

double Random::Next(double max) {
    MSG_CHECK_LT(0, max, "Random::Next(double max): max must be positive");
    return this->Next() * max;
}

double Random::Next(double min, double max) {
    MSG_CHECK_LT(min, max, "Random::Next(double min, double max): min should be strictly less than max");

    return this->Next(max - min) + min;
}

template <typename Container>
typename Container::value_type Random::Any(const Container &c) {
    int32_t size = int32_t(c.size());
    MSG_CHECK_LT(0, size, "Random::Any(const Container& c): c.size() must be positive");

    return *(c.begin() + this->Next(size));
}

template <typename RandomAccessIterator>
typename RandomAccessIterator::value_type Random::Any(const RandomAccessIterator &first, const RandomAccessIterator &last) {
    int32_t size = int32_t(last - first);
    if (size <= 0)
        __bigo_generator_fail("Random::Any(const Iter& first, const Iter& last): range must have positive length");
    return *(first + this->Next(size));
}

template <typename T>
T Random::WeightedNext(T max, int32_t weight) {
    if (max < T(0))
        __bigo_generator_fail("Random::WeightedNext(max, weight): max must be positive");
    if (std::abs(weight) <= Random::kMaxLoopWeight) {
        T result = this->Next(max);
        for (int i = 0; i < weight; i++)
            result = std::max(result, this->Next(max));
        for (int i = 0; i < -weight; i++)
            result = std::min(result, this->Next(max));
        return result;
    }
    // weight > kMaxLoopWeight, use pow to reduce complexity
    double value = std::pow(this->Next(), 1.0 / (abs(weight) + 1));
    if (weight < 0) value = 1 - value;
    return T(max * value);
}

int32_t Random::WeightedNext(int32_t max, int32_t weight) {
    MSG_CHECK_LT(0, max, "Random::WeightedNext(int32_t max, int32_t weight): max must be positive");

    return this->WeightedNext<int32_t>(max, weight);
}

int32_t Random::WeightedNext(int32_t min, int32_t max, int32_t weight) {
    if (min < 0 && min + INT32_MAX - 1 > max)
        __bigo_generator_fail("Random::Next(int32_t min, int32_t max, int32_t weight): difference between max and min is too large: %d - %d", min, max);
    return this->WeightedNext<int32_t>(max - min + 1, weight) + min;
}

int64_t Random::WeightedNext(int64_t max, int32_t weight) {
    MSG_CHECK_LT(0, max, "Random::WeightedNext(int64_t max, int32_t weight): max must be positive");

    return this->WeightedNext<int64_t>(max, weight);
}

int64_t Random::WeightedNext(int64_t min, int64_t max, int32_t weight) {
    if (min < 0 && min + INT64_MAX - 1 > max)
        __bigo_generator_fail("Random::Next(int64_t min, int64_t max, int32_t weight): difference between max and min is too large: " I64 " - " I64, min, max);
    return this->WeightedNext<int64_t>(max - min + 1, weight) + min;
}

double Random::WeightedNext(int32_t weight) {
    return this->WeightedNext<double>(1.0, weight);
}

double Random::WeightedNext(double max, int32_t weight) {
    MSG_CHECK_LT(0, max, "Random::Next(double max): max must be positive");
    return this->WeightedNext<double>(max, weight);
}

double Random::WeightedNext(double min, double max, int32_t weight) {
    return this->WeightedNext<double>(max - min, weight) + min;
}

template <typename Container>
typename Container::value_type Random::WeightedAny(const Container &c, int32_t weight) {
    int32_t size = int32_t(c.size());
    MSG_CHECK_LT(0, size, "Random::WeightedAny(const Container& c): c.size() must be positive");

    return *(c.begin() + this->WeightedNext(size, weight));
}

template <typename RandomAccessIterator>
typename RandomAccessIterator::value_type Random::WeightedAny(const RandomAccessIterator &first, const RandomAccessIterator &last, int32_t weight) {
    int32_t size = int32_t(last - first);
    if (size <= 0)
        __bigo_generator_fail("Random::Any(const Iter& first, const Iter& last): range must have positive length");
    return *(first + this->WeightedNext(size, weight));
}

template <typename T>
std::vector<T> Random::Permutation(T size)
{
    if (size > Random::kMaxRangeLimit)
        __bigo_generator_fail("Random::Permutation: size must not graeter than 10000000");
    return Permutation(size, T(0));
}

template <typename T, typename E>
std::vector<E> Random::Permutation(T size, E first) {
    if (size < 0)
        __bigo_generator_fail("Random::Permutation: size must non-negative");
    if (size > Random::kMaxRangeLimit)
        __bigo_generator_fail("Random::Permutation: size must not graeter than 10000000");
    if (size == 0)
        return std::vector<E>();
    
    std::vector<E> p(size);
    E current = first;
    for (T i = 0; i < size; i++)
        p[i] = current++;
    if (size > 1) {
        for (T i = 1; i < size; i++)
            std::swap(p[i], p[Next(i + 1)]);
    }
    return p;
}

template <typename T>
std::vector<T> Random::NextSet(int32_t size, T min, T max) {
    if (min > max)
        __bigo_generator_fail("Random::NextSet expected min <= max");

    if (size < 0)
        __bigo_generator_fail("Random::NextSet expected size >= 0");

    if (size > Random::kMaxRangeLimit)
        __bigo_generator_fail("Random::NextSet: size must not graeter than 10000000");

    uint64_t n = max - min + 1;
    if (uint64_t(size) > n)
        __bigo_generator_fail("Random::NextSet expected size <= max - min + 1");

    std::vector<T> result;
    if (size == 0)
        return result;

    double expected = 0;
    for (int32_t i = 0; i < size; i++)
        expected += double(n) / double(n - i);

    if (expected < double(n)) {
        std::set<T> vals;
        while (int32_t(vals.size()) < size) {
            T x = T(Next(min, max));
            if (vals.insert(x).second)
                result.push_back(x);
        }
    } else {
        if (n > 1000000000)
            __bigo_generator_fail("Random::NextSet here expected max - min + 1 <= 10^9");
        std::vector<T> p(Permutation(int32_t(n), min));
        result.insert(result.end(), p.begin(), p.begin() + size);
    }

    return result;
}

template <typename T>
std::vector<T> Random::NextSet(int32_t size, T max) {
    if (size < 0)
        __bigo_generator_fail("Random::NextSet expected size >= 0");
    if (size > Random::kMaxRangeLimit)
        __bigo_generator_fail("Random::NextSet: size must not graeter than 10000000");
    if (max <= 0)
        __bigo_generator_fail("Random::NextSet expected max > 0");
    if (size > max)
        __bigo_generator_fail("Random::NextSet expected size <= max");

    if (size == 0)
        return std::vector<T>();

    return this->NextSet(size, T(0), max - 1);
}

std::string Random::NextString(const std::string &pattern) {
    Pattern p(pattern);
    return p.Next(*this);
}

template <typename RandomAccessIterator>
void Random::Shuffle(RandomAccessIterator first, RandomAccessIterator last) {
    typename std::iterator_traits<RandomAccessIterator>::difference_type i, n;
    n = (last-first);
    for (i = n - 1; i > 0; --i) {
        std::swap(first[i], first[Next((int32_t)i + 1)]);
    }
}

std::vector<std::pair<int32_t, int32_t>> Random::GenerateTree(int32_t size, int32_t first, int32_t weight) {
    if (size > Random::kMaxRangeLimit)
        __bigo_generator_fail("Random::GenerateTree: size must not graeter than 10000000");

        std::vector<int32_t> p(size);

    // p[i] is parent of the i-th vertex in permutation
    for (int i = 1; i < size; i++)
        p[i] = this->WeightedNext(i, weight);
    
    std::vector<int32_t> perm = this->Permutation(size);

    std::vector<std::pair<int32_t, int32_t>> edges;
    for (int i = 1; i < size; i++)
        if (this->Next(2)) {
            edges.push_back({perm[i] + first, perm[p[i]] + first});
        } else {
            edges.push_back({perm[p[i]] + first, perm[i] + first});
        }

    this->Shuffle(edges.begin(), edges.end());
    return edges;
}

std::vector<int32_t> Random::GenerateRootedTree(int32_t size, int32_t root, int32_t weight) {
    if (size > Random::kMaxRangeLimit)
        __bigo_generator_fail("Random::GenerateRootedTree: size must not graeter than 10000000");

    std::vector<int32_t> p(size);
    std::vector<int32_t> perm(size);

    // p[i] is parent of the i-th vertex in permutation
    for (int i = 1; i < size; i++) {
        p[i] = this->WeightedNext(i, weight);
        perm[i] = i;
    }

    std::swap(perm[root], perm[0]);

    this->Shuffle(perm.begin() + 1, perm.end());

    std::vector<int32_t> result(size, -1);
    for (int i = 1; i < size; i++)
        result[perm[i]] = perm[p[i]];
    return result;
}

Pattern::Pattern(std::string pattern) : source_(pattern), from_(0), to_(0) {
    std::string t;
    for (size_t i = 0; i < source_.length(); i++)
        if (!__is_command_char(source_, i, ' '))
            t += source_[i];
    source_ = t;

    int32_t opened = 0;
    int32_t firstClose = -1;
    std::vector<int> seps;

    for (size_t i = 0; i < source_.length(); i++) {
        if (__is_command_char(source_, i, '(')) {
            opened++;
            continue;
        }

        if (__is_command_char(source_, i, ')')) {
            opened--;
            if (opened == 0 && firstClose == -1)
                firstClose = int32_t(i);
            continue;
        }

        if (opened < 0)
            __bigo_generator_fail("Pattern: Illegal pattern (or part) \"" + source_ + "\"");

        if (__is_command_char(source_, i, '|') && opened == 0)
            seps.push_back(int32_t(i));
    }

    if (opened != 0)
        __bigo_generator_fail("Pattern: Illegal pattern (or part) \"" + source_ + "\"");

    if (seps.size() == 0 && firstClose + 1 == (int32_t)source_.length() &&
        __is_command_char(source_, 0, '(') &&
        __is_command_char(source_, source_.length() - 1, ')')) {
        children_.push_back(Pattern(source_.substr(1, source_.length() - 2)));
    } else {
        if (seps.size() > 0) {
            seps.push_back(int32_t(source_.length()));
            int32_t last = 0;

            for (size_t i = 0; i < seps.size(); i++) {
                children_.push_back(Pattern(source_.substr(last, seps[i] - last)));
                last = seps[i] + 1;
            }
        } else {
            size_t pos = 0;
            chars_ = __scan_char_set(source_, pos);
            __scan_counts(source_, pos, from_, to_);
            if (pos < source_.length())
                children_.push_back(Pattern(source_.substr(pos)));
        }
    }
}

bool Pattern::Matches(const std::string &s, size_t pos) const {
    std::string result;

    if (to_ > 0) {
        int32_t size = __greedy_match(s, pos, chars_);
        if (size < from_)
            return false;
        if (size > to_)
            size = to_;
        pos += size;
    }

    if (children_.size() > 0) {
        for (size_t child = 0; child < children_.size(); child++)
            if (children_[child].Matches(s, pos))
                return true;
        return false;
    } else {
        return pos == s.length();
    }
}

std::string Pattern::Next(Random &rnd) const {
    std::string result;
    result.reserve(20);

    if (to_ == INT_MAX)
        __bigo_generator_fail("Pattern::Next(Random& rnd): can't process character '*' for generation");

    if (to_ > 0) {
        int32_t count = rnd.Next(to_ - from_ + 1) + from_;
        for (int32_t i = 0; i < count; i++)
            result += chars_[rnd.Next(int32_t(chars_.size()))];
    }

    if (children_.size() > 0) {
        int32_t child = rnd.Next(int32_t(children_.size()));
        result += children_[child].Next(rnd);
    }

    return result;
}

bool Pattern::Matches(const std::string &s) const {
    return Matches(s, 0);
}

std::string Pattern::Source() const {
    return source_;
}

namespace printer {
    template<typename A, typename B>
    typename __bigo_enable_if<!is_iterator<B>::value, void>::type
    Print(const A &a, const B &b) {
        __print_one(a);
        std::cout << ' ';
        __print_one(b);
    }

    template<typename A, typename B>
    typename __bigo_enable_if<is_iterator<B>::value, void>::type
    Print(const A &a, const B &b) {
        __print_range(a, b);
    }

    template<typename A>
    void Print(const A *a, const A *b) {
        __print_range(a, b);
    }

    template<>
    void Print<char>(const char *a, const char *b) {
        __print_one(a);
        std::cout << ' ';
        __print_one(b);
    }

    template<typename T>
    void Print(const T &x) {
        __print_one(x);
    }

    template <size_t N, typename T>
    void Print(const T (&arr)[N], int32_t size) {
        for (size_t i = 0; i < size; i++)
            std::cout << arr[i] << ' ';
    }

    template <size_t N>
    void Print(const char (&s)[N]) {
        std::cout << s;
    }

    template<typename A, typename B>
    typename __bigo_enable_if<!is_iterator<B>::value, void>::type
    PrintLine(const A &a, const B &b) {
        __print_one(a);
        std::cout << ' ';
        __print_one(b);
        std::cout << '\n';
    }

    template<typename A, typename B>
    typename __bigo_enable_if<is_iterator<B>::value, void>::type
    PrintLine(const A &a, const B &b) {
        __print_range(a, b);
        std::cout << '\n';
    }

    template<typename A>
    void PrintLine(const A *a, const A *b) {
        __print_range(a, b);
        std::cout << '\n';
    }

    template<>
    void PrintLine<char>(const char *a, const char *b) {
        __print_one(a);
        std::cout << ' ';
        __print_one(b);
        std::cout << '\n';
    }

    template<typename T>
    void PrintLine(const T &x) {
        __print_one(x);
        std::cout << '\n';
    }

    template<typename A, typename B, typename C>
    void PrintLine(const A &a, const B &b, const C &c) {
        __print_one(a);
        std::cout << ' ';
        __print_one(b);
        std::cout << ' ';
        __print_one(c);
        std::cout << '\n';
    }

    template<typename A, typename B, typename C, typename D>
    void PrintLine(const A &a, const B &b, const C &c, const D &d) {
        __print_one(a);
        std::cout << ' ';
        __print_one(b);
        std::cout << ' ';
        __print_one(c);
        std::cout << ' ';
        __print_one(d);
        std::cout << '\n';
    }

    template<typename A, typename B, typename C, typename D, typename E>
    void PrintLine(const A &a, const B &b, const C &c, const D &d, const E &e) {
        __print_one(a);
        std::cout << ' ';
        __print_one(b);
        std::cout << ' ';
        __print_one(c);
        std::cout << ' ';
        __print_one(d);
        std::cout << ' ';
        __print_one(e);
        std::cout << '\n';
    }

    template <size_t N, typename T>
    void PrintLine(const T (&arr)[N], int32_t size) {
        for (size_t i = 0; i < size; i++)
            std::cout << arr[i] << " \n"[i == size-1];
    }

    template <size_t N>
    void PrintLine(const char (&s)[N]) {
        std::cout << s << "\n";
    }
}

template <typename... Args>
std::string Format(const char *format, Args... args) {
    return __format(format, args...);
}

void OpenTestFile(int32_t test_number, std::string ext="in") {
    const std::string testFileName = __format("%d.%s", test_number, ext.data());
    if (!freopen(testFileName.c_str(), "wt", stdout))
        __bigo_generator_fail("Unable to write file '" + testFileName + "'");
}

#endif