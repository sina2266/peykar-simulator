// No line break restriction after access modifiers

struct foo {
  private:
    int i;

  protected:
    int j;
    /* comment */
  public:
    foo() {}

  private:
  protected:
};