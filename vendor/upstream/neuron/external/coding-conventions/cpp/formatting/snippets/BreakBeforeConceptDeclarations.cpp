// New line before C++20 `concept` directive

template <typename T>
concept Hashable = requires(T a) {
                       { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
                   };
