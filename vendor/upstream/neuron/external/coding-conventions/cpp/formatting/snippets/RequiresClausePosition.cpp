// The C++20 `requires` clause is put in its own line

template <typename T>
concept Addable = requires(T t) { ++t; };

template <typename T>
requires Addable<T>
struct Foo {
    // ...
};

template <typename T>
requires Addable<T>
void bar(T t){
    // ...
};

template <typename T>
void baz(T t)
requires Addable<T>
{
    //...
};
