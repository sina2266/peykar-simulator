// line break before C++20 `requires` directive

template <typename It>
requires Iterator<It>
// clang-format off
void sort(It begin, It end) {
    //....
}

// clang-format on
