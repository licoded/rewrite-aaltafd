class Foo
{
private:
    Foo *left_;
    Foo *right_;
public:
    Foo() = default;
    Foo(const Foo &foo) {
        *this = foo; // 浅拷贝
    }
    Foo(int x) {
        // left_ = Foo();
        // left_ = &Foo();
        left_ = new Foo();
    }
    ~Foo() = default;
};


int main()
{

    return 0;
}