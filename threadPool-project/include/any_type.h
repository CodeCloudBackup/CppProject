#ifndef ANY_H
#define ANY_H

#include<memory>

class AnyType {
public:
    AnyType() = default;
    ~AnyType() = default;
    AnyType(const AnyType& other) = delete;
    AnyType& operator=(const AnyType& other) = delete;
    // 1. 显式启用默认的移动构造函数和移动赋值运算符
    AnyType(AnyType&& other) noexcept = default;
    AnyType& operator=(AnyType&& other) noexcept = default;
    
    // 模版构造函数
    template<typename T> 
    AnyType(T data) : base_(std::make_unique<Derived<T>>(data)) {}

    template<typename T> 
    T GetData() const {
        // 基类对象指针转换为派生类对象指针 RTTI
        Derived<T> *pd =  dynamic_cast<Derived<T>*>(base_.get());
        if(!pd) {
            throw std::bad_cast();
        }
        return pd->value;
    }
private:
    class Base {
    public:
        virtual ~Base() = default;
    };
    
    template<typename T>
    class Derived : public Base {
    public:
        Derived(T data) : value(data) {};
        T value;
    };

private:
    std::unique_ptr<Base> base_;
};

#endif
