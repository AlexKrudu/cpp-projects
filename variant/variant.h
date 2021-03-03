//
// Created by alexkrudu on 06.01.2021.
//

#include <bits/enable_special_members.h>
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <functional>
#include <experimental/array>

#pragma once

using std::true_type;
using std::false_type;


inline constexpr size_t variant_npos = -1;

template<typename Visitor, typename... Variants>
constexpr decltype(auto) visit(Visitor &&visitor, Variants &&... variants);

template<typename Visitor, typename... Variants>
constexpr decltype(auto) visit_ind(Visitor &&visitor, Variants &&... variants);

struct bad_variant_access final : std::exception {

    const char *what() const noexcept override { return "bad variant access"; }
};

template<typename T, bool = std::is_trivially_destructible_v<T>>
struct type_wrap {
    using storage_t = typename std::aligned_storage_t<sizeof(T), alignof(T)>;
    storage_t storage;

    template<typename... Args>
    constexpr explicit type_wrap(std::in_place_index_t<0>, Args &&... args) {
        new(&storage) T(std::forward<Args>(args)...);
    }

    constexpr const T &m_get() const & noexcept { return *reinterpret_cast<const T *>(&storage); }

    constexpr T &m_get() & noexcept { return *reinterpret_cast<T *>(&storage); }

    constexpr const T &&m_get() const && noexcept { return std::move(*reinterpret_cast<const T *>(&storage)); }

    constexpr T &&m_get() && noexcept { return std::move(*reinterpret_cast<T *>(&storage)); }
};

template<typename T>
struct type_wrap<T, true> {
    T storage;

    template<typename... Args>
    constexpr explicit type_wrap(std::in_place_index_t<0>, Args &&... args) : storage(std::forward<Args>(args)...) {}

    constexpr const T &m_get() const & noexcept { return storage; }

    constexpr T &m_get() & noexcept { return storage; }

    constexpr const T &&m_get() const && noexcept { return std::move(storage); }

    constexpr T &&m_get() && noexcept { return std::move(storage); }
};

template<typename... Types>
struct variadic_union {
    void destroy(std::size_t index) {}

    void copy(std::size_t index, const variadic_union &other) {}

    void move(std::size_t index, variadic_union &&other) {}

    void move_construct(std::size_t index, variadic_union &&other) {}

    void copy_construct(std::size_t index, const variadic_union &other) {}

    void swap(std::size_t index, variadic_union &other) {}
};

template<typename Head, typename... Tail>
struct variadic_union<Head, Tail...> {
    union {
        type_wrap<Head> head;
        variadic_union<Tail...> rest;
    };

    constexpr variadic_union() : rest() {}

    template<typename... Args>
    constexpr explicit variadic_union(std::in_place_index_t<0>, Args &&... args)
            : head(std::in_place_index<0>, std::forward<Args>(args)...) {}

    template<std::size_t I, typename... Args>
    constexpr explicit variadic_union(std::in_place_index_t<I>, Args &&... args)
            : rest(std::in_place_index<I - 1>, std::forward<Args>(args)...) {}

    void destroy(std::size_t index) {
        if (index == 0) {
            head.m_get().~Head();
        } else {
            rest.destroy(index - 1);
        }
    }

    void copy(std::size_t index, const variadic_union &other) {
        if (index == 0) {
            head.m_get() = other.head.m_get();
        } else {
            rest.copy(index - 1, other.rest);
        }
    }

    void move(std::size_t index, variadic_union &&other) {
        if (index == 0) {
            head.m_get() = std::move(other.head.m_get());
        } else {
            rest.move(index - 1, std::move(other.rest));
        }
    }

    void move_construct(std::size_t index, variadic_union &&other) {
        if (index == 0) {
            // static_assert(std::is_same_v<Head,  int>);

            head = type_wrap<Head>(std::in_place_index<0>, std::move(other.head.m_get()));
            // std::cout << "!23";
        } else {
            rest.move_construct(index - 1, std::move(other.rest));
        }
    }

    void copy_construct(std::size_t index, const variadic_union &other) {
        if (index == 0) {
            head = type_wrap<Head>(std::in_place_index<0>, other.head.m_get());
        } else {
            rest.copy_construct(index - 1, other.rest);
        }
    }

    void swap(std::size_t index, variadic_union &other) {
        if (index == 0) {
            auto v1 = this->head.m_get();
            auto v2 = other.head.m_get();
            swap(v1, v2);
        } else {
            rest.swap(index - 1, other.rest);
        }
    }


};

template<typename Union>
constexpr decltype(auto) get_v(std::in_place_index_t<0>, Union &&u) {
    return std::forward<Union>(u).head.m_get();
}

template<std::size_t I, typename Union>
constexpr decltype(auto) get_v(std::in_place_index_t<I>, Union &&u) {
    return get_v(std::in_place_index<I - 1>, std::forward<Union>(u).rest);
}

template<std::size_t I, typename Variant>
constexpr decltype(auto) get_v(Variant &&v) {
    return get_v(std::in_place_index<I>, std::forward<Variant>(v).storage);
}

template<size_t Np, typename... Types>
struct Nth_type;

template<size_t Np, typename First, typename... Rest>
struct Nth_type<Np, First, Rest...> : Nth_type<Np - 1, Rest...> {
};

template<typename First, typename... Rest>
struct Nth_type<0, First, Rest...> {
    using type = First;
};

template<typename Var>
struct variant_size {
};

template<typename Var>
struct variant_size<const Var> : variant_size<Var> {
};

template<std::size_t index, typename Var>
struct variant_alternative {
};

template<std::size_t index, typename Var>
struct variant_alternative<index, const Var> {
    using type = typename std::add_const<typename variant_alternative<index, Var>::type>::type;
};

template<typename... Types>
class variant;

template<bool, typename ... Types>
struct variant_destruct_base;


template<std::size_t index, typename... Types>
struct variant_alternative<index, variant<Types...>> {
};

template<typename First, typename... Rest>
struct variant_alternative<0, variant<First, Rest...>> {
    using type = First;
};

template<std::size_t cur, typename First, typename... Rest>
struct variant_alternative<cur, variant<First, Rest...>> : variant_alternative<cur - 1, variant<Rest...>> {
};

template<typename... Types>
struct variant_size<variant<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {
};


template<class T> inline constexpr std::size_t variant_size_v = variant_size<T>::value;

template<size_t index, class T> using variant_alternative_t = typename variant_alternative<index, T>::type;

template<typename Ti>
struct Arr {
    Ti x[1];
};

template<size_t Ind, typename Tp, typename Ti,
        bool Ti_is_cv_bool = std::is_same_v<std::remove_cv_t<Ti>, bool>,
        typename = void>
struct overload {
    void identity();
};

template<size_t Ind, typename Tp, typename Ti>
struct overload<Ind, Tp, Ti, false,
        std::void_t<decltype(Arr<Ti>{{std::declval<Tp>()}})>> {
    static std::integral_constant<size_t, Ind> identity(Ti);
};

template<size_t Ind, typename Tp, typename Ti>
struct overload<Ind, Tp, Ti, true,
        std::enable_if_t<std::is_same_v<std::__remove_cvref_t<Tp>, bool>>> {
    static std::integral_constant<size_t, Ind> identity(Ti);
};

template<typename Tp, typename Variant,
        typename = std::make_index_sequence<variant_size_v<Variant>>>
struct overload_resolution;

template<typename Tp, typename... Ti, size_t... Ind>
struct overload_resolution<Tp, variant<Ti...>, std::index_sequence<Ind...>>
        : overload<Ind, Tp, Ti> ... {
    using overload<Ind, Tp, Ti>::identity...;
};

template<typename Tp, typename Variant>
using FUN_type
= decltype(overload_resolution<Tp, Variant>::identity(std::declval<Tp>()));

template<typename Tp, typename Variant, typename = void>
struct alt_index
        : std::integral_constant<size_t, variant_npos> {
};

template<typename Tp, typename Variant>
struct alt_index<Tp, Variant, std::void_t<FUN_type<Tp, Variant>>>
        : FUN_type<Tp, Variant> {
};


template<typename T, typename... Types>
struct index_of;

template<typename T, typename... Types>
struct index_of<T, T, Types...> : std::integral_constant<std::size_t, 0> {
};

template<typename T, typename U, typename... Types>
struct index_of<T, U, Types...> : std::integral_constant<std::size_t, index_of<T, Types...>::value + 1> {
};

template<typename T, typename... Types> inline constexpr std::size_t index_of_v = index_of<T, Types...>::value;

template<typename T, typename... Types>
struct count_of : std::integral_constant<std::size_t, 0> {
};

template<typename T, typename U, typename... Types>
struct count_of<T, U, Types...>
        : std::integral_constant<std::size_t, count_of<T, Types...>::value + std::is_same_v<T, U>> {
};

template<typename T, typename... Types> inline constexpr std::size_t count_of_v = count_of<T, Types...>::value;

template<typename LeftV, typename RightV>
void construct_single(LeftV &&leftV, RightV &&right_storage) {
    void *storage = std::addressof(leftV.storage);
    using Type = std::remove_reference_t<decltype(right_storage)>;
    new(storage) Type(std::forward<decltype(right_storage)>(right_storage));
}


template<bool, typename... Types>
struct variant_destruct_base {
public:
    using storage_t = variadic_union<Types...>;

    storage_t storage;
    size_t type_index;

public:
    constexpr variant_destruct_base() : type_index(variant_npos) {};

    template<class T, class... Args,
            typename = std::enable_if_t<count_of_v<T, Types...> == 1 && std::is_constructible_v<T, Args...>>>
    constexpr explicit variant_destruct_base(std::in_place_type_t<T>,
                                             Args &&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
            : variant_destruct_base(std::in_place_index<index_of_v<T, Types...>>, std::forward<Args>(args)...) {}

    template<std::size_t Np, typename = std::enable_if_t<Np < sizeof...(Types)>> using type_by_index =
    typename Nth_type<Np, Types...>::type;

    template<std::size_t I, class... Args, typename T = type_by_index<I>,
            typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
    constexpr explicit variant_destruct_base(std::in_place_index_t<I>,
                                             Args &&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
            : type_index(I), storage(std::in_place_index<I>, std::forward<Args>(args)...) {}

    constexpr bool valid() const { return type_index != variant_npos; }

    void reset() {
        if (!valid())
            return;
        //  helper_t::destroy(type_index, &storage);
        storage.destroy(type_index);
        type_index = variant_npos;
    }

    ~variant_destruct_base() { reset(); }

    [[nodiscard]] constexpr bool valueless_by_exception() const { return !this->valid(); }

    [[nodiscard]] constexpr std::size_t index() const noexcept { return this->type_index; }

};

template<typename... Types>
struct variant_destruct_base<true, Types...> {
public:
    using storage_t = variadic_union<Types...>;

    storage_t storage;
    size_t type_index;

public:
    constexpr variant_destruct_base() : type_index(variant_npos) {};

    template<std::size_t Np, typename = std::enable_if_t<Np < sizeof...(Types)>> using type_by_index =
    typename Nth_type<Np, Types...>::type;

    template<std::size_t I, class... Args, typename T = type_by_index<I>,
            typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
    constexpr explicit variant_destruct_base(std::in_place_index_t<I>,
                                             Args &&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
            : type_index(I), storage(std::in_place_index<I>, std::forward<Args>(args)...) {}

    template<class T, class... Args,
            typename = std::enable_if_t<count_of_v<T, Types...> == 1 && std::is_constructible_v<T, Args...>>>
    constexpr explicit variant_destruct_base(std::in_place_type_t<T>,
                                             Args &&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
            : variant_destruct_base(std::in_place_index<index_of_v<T, Types...>>, std::forward<Args>(args)...) {}

    [[nodiscard]] constexpr bool valid() const { return type_index != variant_npos; }

    void reset() { type_index = variant_npos; }

    [[nodiscard]] constexpr bool valueless_by_exception() const { return !this->valid(); }

    [[nodiscard]] constexpr std::size_t index() const noexcept { return this->type_index; }
};

template<typename... Types>
using variant_destruct_wrap = variant_destruct_base<(std::is_trivially_destructible_v<Types> && ...), Types...>;

template<size_t Np, typename... Types>
constexpr variant_alternative_t<Np, variant<Types...>> &get(variant_destruct_wrap<Types...> &);

template<size_t Np, typename... Types>
constexpr variant_alternative_t<Np, variant<Types...>> &&get(variant_destruct_wrap<Types...> &&);

template<size_t Np, typename... Types>
constexpr variant_alternative_t<Np, variant<Types...>> const &get(const variant_destruct_wrap<Types...> &);

template<size_t Np, typename... Types>
constexpr variant_alternative_t<Np, variant<Types...>> const &&get(const variant_destruct_wrap<Types...> &&);

template<typename ... Types>
struct variant_size<variant_destruct_wrap<Types...>> : variant_size<variant<Types...>> {
};


template<typename... Types, typename Var>
decltype(auto)
variant_cast(Var &&rhs) {
    if constexpr (std::is_lvalue_reference_v<Var>) {
        if constexpr (std::is_const_v<std::remove_reference_t<Var>>)
            return static_cast<const variant_destruct_wrap<Types...> &>(rhs);
        else
            return static_cast<variant_destruct_wrap<Types...> &>(rhs);
    } else
        return static_cast<variant_destruct_wrap<Types...> &&>(rhs);
}

template<typename ... Types, typename LeftV, typename RightV>
void construct_variant(LeftV &&left, RightV &&right) {
    left.type_index = std::move(right.type_index);
    visit([&left](auto &&val) { construct_single(std::forward<LeftV>(left), std::forward<decltype(val)>(val)); },
          variant_cast<Types...>(std::forward<RightV>(right)));
}

template<bool, typename... Types>
struct variant_move_assign_base : variant_destruct_wrap<Types...> {
public:
    using base = variant_destruct_wrap<Types...>;
    using base::base;

    variant_move_assign_base &
    operator=(variant_move_assign_base &&other) noexcept((std::is_nothrow_move_assignable_v<Types> && ...) &&
                                                         (std::is_nothrow_move_constructible_v<Types> && ...)) {
        if (this->valueless_by_exception() && other.valueless_by_exception()) {
            return *this;
        }
        if (other.valueless_by_exception()) {
            this->reset();
            return *this;
        }
/*
    visit_ind([this](auto &&ind, auto &&val) {
        if (this->type_index == ind) {
            get<ind>(*this) = std::move(val);
        } else {
            variant_cast<Types...>(*this).template emplace<ind>(std::move(val));
        }
    }, variant_cast<Types...>(other)); */
        if (this->index() == other.index()) {
            this->storage.move(this->index(), std::move(other.storage));
        } else {
            this->reset();
            this->storage.move_construct(other.index(), std::move(other.storage));
            this->type_index = std::move(other.type_index);
        }

        //construct_variant<Types...>(*this, std::move(other));
        return *this;
    }

    variant_move_assign_base(const variant_move_assign_base &) = default;

    variant_move_assign_base(variant_move_assign_base &&) = default;

    variant_move_assign_base &operator=(const variant_move_assign_base &) = default;
};

template<typename... Types>
struct variant_move_assign_base<true, Types...> : variant_destruct_wrap<Types...> {
    using base = variant_destruct_wrap<Types...>;
    using base::base;
};

template<typename... Types>
using variant_move_assign_wrap = variant_move_assign_base<(std::is_trivially_move_assignable_v<Types> && ...) &&
                                                          (std::is_trivially_move_constructible_v<Types> && ...),
        Types...>;

template<bool, typename... Types>
struct variant_copy_assign_base : variant_move_assign_wrap<Types...> {
    using base = variant_move_assign_wrap<Types...>;
    using base::base;

    variant_copy_assign_base &
    operator=(const variant_copy_assign_base &other) noexcept((std::is_nothrow_copy_assignable_v<Types> && ...)) {
        if (this->valueless_by_exception() && other.valueless_by_exception()) {
            return *this;
        }
        if (other.valueless_by_exception()) {
            this->reset();
            return *this;
        }
/*
    visit_ind([this](auto &&ind, auto &&val) {
        if (this->type_index == ind) {
            get<ind>(*this) = std::move(val);
        } else {
            variant_cast<Types...>(*this).template emplace<ind>(std::move(val));
        }
    }, variant_cast<Types...>(other)); */
        if (this->index() == other.index()) {
            this->storage.copy(this->index(), other.storage);
        } else {
            this->reset();
            this->type_index = other.type_index;
            this->storage.copy_construct(this->index(), other.storage);
        }

        //construct_variant<Types...>(*this, std::move(other));
        return *this;
    }

    variant_copy_assign_base(const variant_copy_assign_base &) = default; // todo maybe should not default this
    variant_copy_assign_base(variant_copy_assign_base &&) = default;

    variant_copy_assign_base &operator=(variant_copy_assign_base &&) = default;
};

template<typename... Types>
struct variant_copy_assign_base<true, Types...> : variant_move_assign_wrap<Types...> {
    using base = variant_move_assign_wrap<Types...>;
    using base::base;
};

template<typename... Types>
using variant_copy_assign_wrap = variant_copy_assign_base<(std::is_trivially_copy_assignable_v<Types> && ...) &&
                                                          (std::is_trivially_copy_constructible_v<Types> && ...),
        Types...>;

template<bool, typename... Types>
struct variant_move_base : variant_copy_assign_wrap<Types...> {
    using base = variant_copy_assign_wrap<Types...>;
    using base::base;

    variant_move_base(variant_move_base &&other) noexcept((std::is_nothrow_move_constructible_v<Types> && ...)) {
        //    helper_t::move(other.type_index, &other.storage, &this->storage);
        construct_variant<Types...>(*this, std::move(other));
    }

    template<typename Up>
    void destructive_move(unsigned short rhs_index, Up &&rhs) {
        this->reset();
        construct_single(*this, std::forward<Up>(rhs));
        this->type_index = rhs_index;
    }

    template<typename Up>
    void destructive_copy(unsigned short rhs_index, const Up &rhs) {
        this->reset();
        construct_single(*this, rhs);
        this->type_index = rhs_index;
    }

    variant_move_base(const variant_move_base &) = default;

    variant_move_base &operator=(const variant_move_base &) = default;

    variant_move_base &operator=(variant_move_base &&) = default;
};

template<typename... Types>
struct variant_move_base<true, Types...> : variant_copy_assign_wrap<Types...> {
    using base = variant_copy_assign_wrap<Types...>;
    using base::base;

    template<typename Up>
    void destructive_move(unsigned short rhs_index, Up &&rhs) {
        this->reset();
        construct_single(*this, std::forward<Up>(rhs));
        this->type_index = rhs_index;
    }

    template<typename Up>
    void destructive_copy(unsigned short rhs_index, const Up &rhs) {
        this->reset();
        construct_single(*this, rhs);
        this->type_index = rhs_index;
    }
};

template<typename... Types>
using variant_move_wrap = variant_move_base<(std::is_trivially_move_constructible_v<Types> && ...), Types...>;

template<typename... Types>
struct variant_size<variant_move_wrap<Types...>> : variant_size<variant<Types...>> {
};

template<bool, typename... Types>
struct variant_copy_base : variant_move_wrap<Types...> {
    using base = variant_move_wrap<Types...>;
    using base::base;

    variant_copy_base(const variant_copy_base &other) noexcept((std::is_nothrow_copy_constructible_v<Types> && ...)) {
        //     helper_t::copy(other.type_index, &other.storage, &this->storage);
        construct_variant<Types...>(*this, other);
    }

    variant_copy_base(variant_copy_base &&) = default;

    variant_copy_base &operator=(const variant_copy_base &) = default;

    variant_copy_base &operator=(variant_copy_base &&) = default;
};

template<typename... Types>
struct variant_copy_base<true, Types...> : variant_move_wrap<Types...> {
    using base = variant_move_wrap<Types...>;
    using base::base;
};

template<typename... Types>
using variant_copy_wrap = variant_copy_base<(std::is_trivially_copy_constructible_v<Types> && ...), Types...>;

template<typename... Types>
struct variant_size<variant_copy_wrap<Types...>> : variant_size<variant<Types...>> {
};

template<bool, typename... Types>
struct variant_def_construct_base : variant_copy_wrap<Types...> {
    using base = variant_copy_wrap<Types...>;
    using base::base;

    constexpr variant_def_construct_base() = delete;
};

template<typename... Types>
struct variant_def_construct_base<true, Types...> : variant_copy_wrap<Types...> {
    using base = variant_copy_wrap<Types...>;
    using base::base;

    constexpr variant_def_construct_base() noexcept(
    std::is_nothrow_default_constructible_v<typename Nth_type<0, Types...>::type>)
            : variant_def_construct_base(std::in_place_index<0>) {};
};

template<typename... Types>
using variant_def_construct_wrap =
variant_def_construct_base<std::is_default_constructible_v<typename Nth_type<0, Types...>::type>, Types...>;

template<std::size_t I, typename Variant, typename... Args>
void construct_by_index(Variant &v, Args &&... args) {
    v.type_index = I;
    auto &&storage = get<I>(v);
    new((void *) std::addressof(storage)) std::remove_reference_t<decltype(storage)>(std::forward<Args>(args)...);
    // v.storage.construct(I, std::forward<Args>(args)...);
}


template<typename... Types>
class variant
        : public variant_def_construct_wrap<Types...>,
          std::_Enable_copy_move<(std::is_copy_constructible_v<Types> && ...),
                  (std::is_copy_constructible_v<Types> && ...) && (std::is_copy_assignable_v<Types> && ...),
                  (std::is_move_constructible_v<Types> && ...),
                  (std::is_move_constructible_v<Types> && ...) && (std::is_move_assignable_v<Types> && ...),
                  variant<Types...>> {
public:
    using base = variant_def_construct_wrap<Types...>;
    using base::base;

    template<size_t Np, typename = std::enable_if_t<(Np < sizeof...(Types))>>
    using get_type = typename Nth_type<Np, Types...>::type;

    template<typename T, typename MatchT = get_type<alt_index<T, variant<Types...>>::value>,
            typename = std::enable_if_t<std::is_constructible_v<MatchT, T>>>

    constexpr variant(T &&item) noexcept(std::is_nothrow_constructible_v<MatchT, T>)
            : base(std::in_place_index<index_of_v<MatchT, Types...>>, std::forward<T>(item)) {}

    template<typename T, typename MatchT = get_type<alt_index<T, variant<Types...>>::value>,
            typename = std::enable_if_t<count_of_v<MatchT, Types...> == 1 && std::is_constructible_v<MatchT, T> &&
                                        std::is_assignable_v<MatchT, T>>>

    variant &
    operator=(T &&item) noexcept(std::is_nothrow_constructible_v<MatchT, T> &&
                                 std::is_nothrow_assignable_v<MatchT, T>) {
        constexpr auto index_v = index_of_v<MatchT, Types...>;
        if (this->index() == index_v) {
            get<index_v>(*this) = std::forward<T>(item);
        } else {
            if constexpr (std::is_nothrow_constructible_v<MatchT, T> || std::is_nothrow_move_constructible_v<MatchT>) {
                this->emplace<index_v>(std::forward<T>(item));
            } else {
                operator=(variant(std::forward<T>(item)));
            }
        }
        // operator=(variant(std::forward<T>(item)));
        return *this;
    }

    variant(const variant &rhs) = default;

    variant(variant &&) = default;

    variant &operator=(const variant &) = default;

    variant &operator=(variant &&) = default;

    ~variant() = default;

    template<std::size_t I, typename... Args,
            typename = std::enable_if_t<std::is_constructible_v<variant_alternative_t<I, variant>, Args...>>>
    variant_alternative_t<I, variant> & emplace(Args &&... args) {
        using type = variant_alternative_t<I, variant>;
        if constexpr (std::is_nothrow_constructible_v<type, Args...>) {
            this->reset();
            construct_by_index<I>(*this, std::forward<Args>(args)...);
        } else {
            this->reset();
            try {
                construct_by_index<I>(*this, std::forward<Args>(args)...);
            } catch (...) {
                this->type_index = variant_npos;
                throw;
            }
        }
        return get<I>(*this);
    }

    template<typename T, typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
    T &emplace(Args &&... args) {
        constexpr std::size_t index = index_of_v<T, Types...>;
        return this->emplace<index>(std::forward<Args>(args)...);
    }


    void swap(variant<Types...> &rhs) noexcept((std::is_nothrow_swappable<Types>::value && ...)
                                               && std::is_nothrow_move_constructible_v<variant<Types...>>) {

        if (this->valueless_by_exception() && rhs.valueless_by_exception()) {
            return;
        }
        if (rhs.valueless_by_exception()) {
            rhs = std::move(*this);
            rhs.type_index = this->type_index;
            this->reset();
            return;
        }
        if (this->valueless_by_exception()) {
            *this = std::move(rhs);
            rhs.reset();
            return;
        }
        visit_ind([this, &rhs](auto ind, auto &&val) mutable {
            if (this->index() == ind) {
                auto &this_s = get<ind>(*this);
                using std::swap;
                swap(this_s, val);
            } else {
                if (!this->valueless_by_exception()) {
                    auto tmp(std::move(val));
                    rhs = std::move(*this);
                    this->destructive_move(ind, std::move(tmp));
                } else {
                    this->destructive_move(ind, std::move(val));
                    rhs.reset();
                }
            }
        }, rhs);

    }

};

template<std::size_t I, typename... Types>
constexpr const variant_alternative_t<I, variant<Types...>> &get(const variant_destruct_wrap<Types...> &v) {
    if (v.index() != I) {
        throw bad_variant_access();
    }
    return get_v<I>(v);
}

template<std::size_t I, typename... Types>
constexpr variant_alternative_t<I, variant<Types...>> &get(variant_destruct_wrap<Types...> &v) {
    if (v.index() != I) {
        throw bad_variant_access();
    }
    return get_v<I>(v);
}

template<std::size_t I, typename... Types>
constexpr variant_alternative_t<I, variant<Types...>> &&get(variant_destruct_wrap<Types...> &&v) {
    if (v.index() != I) {
        throw bad_variant_access();
    }
    return get_v<I>(std::move(v));
}

template<std::size_t I, typename... Types>
constexpr const variant_alternative_t<I, variant<Types...>> &&get(const variant_destruct_wrap<Types...> &&v) {
    if (v.index() != I) {
        throw bad_variant_access();
    }
    return get_v<I>(std::move(v));
}

template<typename T, typename... Types>
constexpr const T &get(const variant_destruct_wrap<Types...> &v) {
    return get_v<index_of_v<T, Types...>>(v);
}

template<typename T, typename... Types>
constexpr T &get(variant_destruct_wrap<Types...> &v) {
    return get_v<index_of_v<T, Types...>>(v);
}

template<typename T, typename... Types>
constexpr T &&get(variant_destruct_wrap<Types...> &&v) {
    return get_v<index_of_v<T, Types...>>(std::move(v));
}

template<typename T, typename... Types>
constexpr const T &&get(const variant_destruct_wrap<Types...> &&v) {
    return get_v<index_of_v<T, Types...>>(std::move(v));
}

template<typename T, typename... Types>
constexpr bool holds_alternative(const variant_destruct_wrap<Types...> &v) {
    return v.index() == index_of_v<T, Types...>;
}

template<std::size_t I, typename... Types>
constexpr std::add_pointer_t<variant_alternative_t<I, variant<Types...>>> get_if(variant<Types...> *v) {
    if (v != nullptr && v->index() == I) {
        return std::addressof(get<I>(*v));
    }
    return nullptr;
}

template<std::size_t I, typename... Types>
constexpr std::add_pointer_t<const variant_alternative_t<I, variant<Types...>>> get_if(const variant<Types...> *v) {
    if (v != nullptr && v->index() == I) {
        return std::addressof(get<I>(*v));
    }
    return nullptr;
}

template<typename T, typename... Types>
constexpr std::add_pointer_t<T> get_if(variant<Types...> *v) {
    return get_if<index_of_v<T, Types...>>(v);
}

template<typename T, typename... Types>
constexpr std::add_pointer_t<const T> get_if(const variant<Types...> *v) {
    return get_if<index_of_v<T, Types...>>(v);
}

template<typename T>
constexpr T &&at_impl(T &&elem) { return std::forward<T>(elem); }

template<typename T, typename... Is>
constexpr auto &&at_impl(T &&elems, std::size_t i, Is... is) {
    return at_impl(std::forward<T>(elems)[i], is...);
}

template<typename T, typename... Is>
constexpr auto &&at(T &&elems, Is... is) {
    return at_impl(std::forward<T>(elems), is...);
}


template<typename Func, typename... Variants, std::size_t... Is>
constexpr decltype(auto) make_vtable_impl(std::index_sequence<Is...> seq) {
    struct generator {
        static constexpr decltype(auto) func_r(Func f, Variants... vs) {
            return static_cast<Func>(f)(get<Is>(static_cast<Variants>(vs))...);
        }
    };
    return &generator::func_r;
}

template<typename F,
        typename... Vs,
        std::size_t... Is,
        std::size_t... Js,
        typename... Ls>
constexpr auto make_vtable_impl(
        std::index_sequence<Is...>, std::index_sequence<Js...>, Ls... ls) {
    return std::experimental::make_array(
            make_vtable_impl<F, Vs...>(std::index_sequence<Is..., Js>{}, ls...)...);
}

template<typename F, typename ... Variants>
constexpr auto make_vtable() {
    return make_vtable_impl<F, Variants...>(
            std::index_sequence<>{},
            std::make_index_sequence<variant_size_v<std::decay_t<Variants>>>{}...);
}

template<typename Visitor, typename... Variants>
constexpr decltype(auto) visit(Visitor &&visitor, Variants &&... variants) {
    if ((variants.valueless_by_exception() || ...)) {
        throw bad_variant_access();
    }
    constexpr auto vtable = make_vtable<Visitor &&, Variants &&...>();
    return at(vtable, variants.index()...)(std::forward<Visitor>(visitor), std::forward<Variants>(variants)...);
}


template<typename Func, typename... Variants, std::size_t... Is>
constexpr decltype(auto) make_vtable_ind_impl(std::index_sequence<Is...> seq) {
    struct generator_ind {
        static constexpr decltype(auto) func_r_ind(Func f, Variants... vs) {
            return static_cast<Func>(f)(std::integral_constant<std::size_t, Is>{}...,
                                        get<Is>(static_cast<Variants>(vs))...);
        }
    };
    return &generator_ind::func_r_ind;
}

template<typename F,
        typename... Vs,
        std::size_t... Is,
        std::size_t... Js,
        typename... Ls>
constexpr auto make_vtable_ind_impl(
        std::index_sequence<Is...>, std::index_sequence<Js...>, Ls... ls) {
    return std::experimental::make_array(
            make_vtable_ind_impl<F, Vs...>(std::index_sequence<Is..., Js>{}, ls...)...);
}

template<typename F, typename ... Variants>
constexpr auto make_vtable_ind() {
    return make_vtable_ind_impl<F, Variants...>(
            std::index_sequence<>{},
            std::make_index_sequence<variant_size_v<std::decay_t<Variants>>>{}...);
}

template<typename Visitor, typename... Variants>
constexpr decltype(auto) visit_ind(Visitor &&visitor, Variants &&... variants) { // visit with index passing to visitor
    if ((variants.valueless_by_exception() || ...)) {
        throw bad_variant_access();
    }
    constexpr auto vtable = make_vtable_ind<Visitor &&, Variants &&...>();
    return at(vtable, variants.index()...)(std::forward<Visitor>(visitor), std::forward<Variants>(variants)...);
}

