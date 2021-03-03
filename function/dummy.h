//
// Created by alexkrudu on 14.10.2020.
//


#ifndef FUNCTION_DUMMY_H
#define FUNCTION_DUMMY_H

#endif //FUNCTION_DUMMY_H

#include <functional>
#include <iostream>

template <typename F>
void delete_func(void* obj)
{
    delete static_cast<F*>(obj);
}

template <typename F, typename Ret, typename... Args>
Ret invoker_func(void* obj, Args... args)
{
    return (*static_cast<F*>(obj))(args...);
}

template <typename Sig>
struct function;

template <typename Ret, typename... Args>
struct function<Ret (Args...)>
{
    typedef void (*deleter_t)(void*);
    typedef Ret (*invoker_t)(void*, Args...);

    function();

    template <typename F>
    function(F f)
            : obj(new F(f))
            , deleter(&delete_func<F>)
            , invoker(&invoker_func<F, Ret, Args...>)
    {}

    function(function const&) = delete;
//function& operator=(function const&) = delete;

    ~function()
    {
        deleter(obj);
    }

    Ret operator()(Args... args) const
    {
        return invoker(obj, args...);
    }

private:
    void* obj;
    deleter_t deleter;
    invoker_t invoker;
};

int main()
{
    int c = 44;
    function<void (int, int)> func = [c] (int a, int b)
    {
        std::cout « "a = " « a « ", b = " « b « ", c = " « c « std::endl;
    };

    func(42, 43);
}