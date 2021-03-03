#pragma once

struct nullopt_t {
};

struct in_place_t {
};
inline constexpr nullopt_t nullopt;
inline constexpr in_place_t in_place;

template<typename T, bool v = std::is_trivially_destructible_v<T>>
struct optional_d_base {
    union {
        T value;
        char dummy;
    };
    bool is_valid = false;

    constexpr optional_d_base() noexcept : dummy() {}

    constexpr optional_d_base(nullopt_t) noexcept : dummy() {}

    ~optional_d_base() {
        reset();
    }

    template<typename... Args>
    explicit constexpr
    optional_d_base(in_place_t, Args &&... args) : value(std::forward<Args>(args)...), is_valid(true) {
    }

    void reset() {
        if (is_valid) {
            value.~T();
            is_valid = false;
            dummy = {};
        }
    }

};

template<typename T>
struct optional_d_base<T, true> {
    union {
        T value;
        char dummy;
    };

    bool is_valid = false;

    constexpr optional_d_base() noexcept : dummy() {}

    constexpr explicit optional_d_base(nullopt_t) noexcept : dummy() {}

    template<typename... Args>
    explicit constexpr
    optional_d_base(in_place_t, Args &&... args) : value(std::forward<Args>(args)...), is_valid(true) {}

    void reset() {
        is_valid = false;
    }
};

template <typename T, bool v = std::is_trivially_copyable_v<T>>
struct optional_c_base {

};

template <typename T>
struct optional_c_base<T, true> {

};

template<typename T>
struct optional_c_base<T, false> {

};


template<typename T, bool v = std::is_trivially_copyable_v<T>>
struct optional_c_base : public optional_d_base<T> {
    using base = optional_d_base<T>;
    using base::base;

    constexpr optional_c_base(optional_c_base const &other) {
        if (other.is_valid) {
            new(&this->value) T(other.value);
            //this->value = other.value;
            this->is_valid = true;
        }
    };

    constexpr optional_c_base(optional_c_base &&other) noexcept(std::is_nothrow_move_constructible_v<T>) {
        if (other.is_valid) {
            new(&this->value) T(std::move(other.value));
            this->is_valid = true;
        }
        other.reset();
    };

    optional_c_base &operator=(optional_c_base const &other) {
        if (this != &other) {
            if (this->is_valid && other.is_valid) {
                this->value = other.value;
            } else if (this->is_valid && !other.is_valid) {
                this->reset();
            } else if (!this->is_valid && other.is_valid) {
                new(&this->value) T(other.value);
                this->is_valid = true;
            }
        }
        return *this;
    };

    optional_c_base &operator=(optional_c_base &&other) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_move_assignable_v<T>) {
        if (this != &other) {
            if (this->is_valid && other.is_valid) {
                this->value = std::move(other.value);
            } else if (this->is_valid && !other.is_valid) {
                this->reset();
            } else if (!this->is_valid && other.is_valid) {
                new(&this->value) T(std::move(other.value));
                this->is_valid = true;
            }
        }
        return *this;
    };
};


template<typename T>
struct optional_c_base<T, true> : public optional_d_base<T> {
    using base = optional_d_base<T>;
    using base::base;

    constexpr optional_c_base(optional_c_base const &) = default;

    constexpr optional_c_base(optional_c_base &&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

    optional_c_base &operator=(optional_c_base const &) = default;

    optional_c_base &operator=(optional_c_base &&) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_move_assignable_v<T>) = default;
};


template<typename T>
class optional : public optional_c_base<T> {
public:
    using base = optional_c_base<T>;
    using base::base;

    constexpr optional(T value) : base(in_place, std::move(value)) {}


    optional &operator=(nullopt_t) noexcept {
        this->reset();
        return *this;
    }

    constexpr explicit operator bool() const noexcept { return this->is_valid; }

    constexpr T &operator*() noexcept { return this->value; }

    constexpr T const &operator*() const noexcept { return this->value; }

    constexpr T *operator->() noexcept { return &(this->value); }

    constexpr T const *operator->() const noexcept { return &(this->value); }

    template<typename... Args>
    void emplace(Args &&... args) {
        this->reset();
        this->value = T(std::forward<Args>(args)...);
        this->is_valid = true;
    }

};


template<typename T>
constexpr bool operator==(optional<T> const &a, optional<T> const &b) {
    if (bool(a) != bool(b)) {
        return false;
    }
    return *a == *b;
}

template<typename T>
constexpr bool operator!=(optional<T> const &a, optional<T> const &b) {
    return !(a == b);
}

template<typename T>
constexpr bool operator<(optional<T> const &a, optional<T> const &b) {
    if (!bool(b)) {
        return false;
    }
    if (!bool(a)) {
        return true;
    }
    return *a < *b;
}

template<typename T>
constexpr bool operator<=(optional<T> const &a, optional<T> const &b) {
    return (a == b || a < b);
}

template<typename T>
constexpr bool operator>(optional<T> const &a, optional<T> const &b) {
    return !(a <= b);
}

template<typename T>
constexpr bool operator>=(optional<T> const &a, optional<T> const &b) {
    return (a == b || a > b);
}
