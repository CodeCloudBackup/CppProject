#ifndef ANY_H
#define ANY_H

#include<memory>

class AnyType {
public:
    AnyType() = default;
    ~AnyType() = default;
    AnyType(AnyType&& other) noexcept = default;
    AnyType& operator=(AnyType&& other) noexcept = default;

    template<typename T> 
    AnyType(T data) : base(std::make_unique<Derived<T>>(data)) {}

    template<typename T> 
    T GetData() const {
        Derived<T> *pd =  dynamic_cast<Derived<T>*>(base.get());
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
    AnyType(const AnyType& other) = delete;
    AnyType& operator=(const AnyType& other) = delete;
private:
    std::unique_ptr<Base> base;
};

#endif
