// Specifies the use of empty lines to separate definition blocks, including classes, structs,
// enums, and functions.


#include <cstring>

struct Foo {
    int a, b, c;
};

namespace Ns {
class Bar {
  public:
    struct Foobar {
        int a;
        int b;
    };
  private:
    int t;

    int method1() {
        // ...
    }

    enum List { ITEM1, ITEM2 };

    template <typename T>
    int method2(T x) {
        // ...
    }

    int i, j, k;

    int method3(int par) {
        // ...
    }
};

class C {};
}  // namespace Ns
