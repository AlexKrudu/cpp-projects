#include <exception>
#include <string>
#include <type_traits>
#include <vector>


#include "variant.h"
#include "gtest/gtest.h"
#include "test-classes.h"


TEST(traits, destructor) {
    using variant1 = variant<int, double, dummy_t>;
    using variant2 = variant<int, std::string>;
    using variant3 = variant<char, long, variant1>;
    using variant4 = variant<char, variant2, int>;
    ASSERT_TRUE(std::is_trivially_destructible_v<variant1>);
    ASSERT_FALSE(std::is_trivially_destructible_v<variant2>);
    ASSERT_TRUE(std::is_trivially_destructible_v<variant3>);
    ASSERT_FALSE(std::is_trivially_destructible_v<variant4>);
}

TEST(traits, default_constructor) {
    using variant1 = variant<std::string, int, std::vector<int>>;
    using variant2 = variant<no_default_t, int>;
    using variant3 = variant<throwing_default_t, int, double>;
    ASSERT_TRUE(std::is_default_constructible_v<variant1>);
    ASSERT_FALSE(std::is_default_constructible_v<variant2>);
    ASSERT_TRUE(std::is_nothrow_default_constructible_v<variant1>);
    ASSERT_FALSE(std::is_nothrow_default_constructible_v<variant3>);
}

TEST(traits, copy_constructor) {
    using variant1 = variant<int, no_copy_t, std::vector<std::string>>;
    using variant2 = variant<std::string, std::vector<std::string>, int>;
    using variant3 = variant<int, double, dummy_t>;
    using variant4 = variant<double, int, non_trivial_copy_t>;
    ASSERT_FALSE(std::is_copy_constructible_v<variant1>);
    ASSERT_TRUE(std::is_copy_constructible_v<variant2>);
    ASSERT_FALSE(std::is_trivially_copy_constructible_v<variant2>);
    ASSERT_TRUE(std::is_trivially_copy_constructible_v<variant3>);
    ASSERT_FALSE(std::is_trivially_copy_constructible_v<variant4>);
}

TEST(traits, move_constructor) {
    using variant1 = variant<int, std::string, no_move_t, double>;
    using variant2 = variant<double, std::string, int>;
    using variant3 = variant<int, dummy_t, char>;
    using variant4 = variant<int, double, throwing_move_operator_t>;
    ASSERT_FALSE(std::is_move_constructible_v<variant1>);
    ASSERT_TRUE(std::is_move_constructible_v<variant2>);
    ASSERT_TRUE(std::is_move_constructible_v<variant3>);
    ASSERT_TRUE(std::is_nothrow_move_constructible_v<variant2>);
    ASSERT_FALSE(std::is_trivially_move_constructible_v<variant2>);
    ASSERT_TRUE(std::is_trivially_move_constructible_v<variant3>);
    ASSERT_TRUE(std::is_move_constructible_v<variant4>);
    ASSERT_FALSE(std::is_nothrow_move_constructible_v<variant4>);
}

TEST(traits, converting_constructor) {
    using variant1 = variant<std::string, std::vector<double>>;
    bool construct1 = std::is_constructible_v<variant1, std::size_t>;
    bool construct2 = std::is_constructible_v<variant1, const char *>;
    bool construct3 = std::is_nothrow_constructible_v<variant1, std::string &&>;
    bool construct4 = std::is_nothrow_constructible_v<variant1, const char *>;
    ASSERT_FALSE(construct1);
    ASSERT_TRUE(construct2);
    ASSERT_TRUE(construct3);
    ASSERT_FALSE(construct4);
}

TEST(traits, in_place_type) {
    using variant1 = variant<int, float, std::string, dummy_t, std::vector<int>, no_default_t>;
    bool construct1 = std::is_constructible_v<variant1, std::in_place_type_t<throwing_move_operator_t>>;
    bool construct2 = std::is_constructible_v<variant1, std::in_place_type_t<dummy_t>>;
    bool construct3 = std::is_constructible_v<variant1, std::in_place_type_t<no_default_t>>;
    bool construct4 = std::is_constructible_v<variant1, std::in_place_type_t<std::vector<int>>, size_t, int>;
    bool construct5 = std::is_constructible_v<variant1, std::in_place_type_t<std::vector<int>>, size_t>;
    bool construct6 = std::is_constructible_v<variant1, std::in_place_type_t<std::string>>;
    ASSERT_FALSE(construct1);
    ASSERT_TRUE(construct2);
    ASSERT_FALSE(construct3);
    ASSERT_TRUE(construct4);
    ASSERT_TRUE(construct5);
    ASSERT_TRUE(construct6);
}

TEST(traits, in_place_index) {
    using variant1 = variant<int, float, std::string, dummy_t, std::vector<int>, no_default_t>;
    bool construct1 = std::is_constructible_v<variant1, std::in_place_index_t<1337>>;
    bool construct2 = std::is_constructible_v<variant1, std::in_place_index_t<3>>;
    bool construct3 = std::is_constructible_v<variant1, std::in_place_index_t<5>>;
    bool construct4 = std::is_constructible_v<variant1, std::in_place_index_t<4>, size_t, int>;
    bool construct5 = std::is_constructible_v<variant1, std::in_place_index_t<4>, size_t>;
    bool construct6 = std::is_constructible_v<variant1, std::in_place_index_t<3>>;
    ASSERT_FALSE(construct1);
    ASSERT_TRUE(construct2);
    ASSERT_FALSE(construct3);
    ASSERT_TRUE(construct4);
    ASSERT_TRUE(construct5);
    ASSERT_TRUE(construct6);
}


TEST(traits, copy_assignment) {
  using variant1 = variant<std::string, double, no_copy_t>;
  using variant2 = variant<std::vector<short>, int, no_copy_assignment_t>;
  using variant3 = variant<dummy_t, int, non_trivial_copy_assignment_t>;
  using variant4 = variant<double, non_trivial_copy_t, bool>;
  using variant5 = variant<int, short, char, dummy_t, bool>;
  ASSERT_FALSE(std::is_copy_assignable_v<variant1>);
  ASSERT_FALSE(std::is_copy_assignable_v<variant2>);
  ASSERT_TRUE(std::is_copy_assignable_v<variant3>);
  ASSERT_TRUE(std::is_copy_assignable_v<variant4>);
  ASSERT_TRUE(std::is_copy_assignable_v<variant5>);
  ASSERT_FALSE(std::is_trivially_copy_assignable_v<variant3>);
  ASSERT_FALSE(std::is_trivially_copy_assignable_v<variant4>);
  ASSERT_TRUE(std::is_trivially_copy_assignable_v<variant5>);
}

TEST(traits, move_assignment) {
  using variant1 = variant<std::string, double, no_move_t>;
  using variant2 = variant<int, std::vector<std::string>, no_move_assignment_t, bool>;
  using variant3 = variant<dummy_t, int, std::vector<double>>;
  using variant4 = variant<double, std::string, bool>;
  using variant5 = variant<int, short, char, dummy_t, bool>;
  using variant6 = variant<int, std::string, throwing_move_operator_t, double>;
  using variant7 = variant<int, throwing_move_assignment_t, double>;
  ASSERT_FALSE(std::is_move_assignable_v<variant1>);
  ASSERT_FALSE(std::is_move_assignable_v<variant2>);
  ASSERT_TRUE(std::is_move_assignable_v<variant3>);
  ASSERT_TRUE(std::is_move_assignable_v<variant4>);
  ASSERT_TRUE(std::is_move_assignable_v<variant5>);
  ASSERT_FALSE(std::is_trivially_move_assignable_v<variant3>);
  ASSERT_FALSE(std::is_trivially_move_assignable_v<variant4>);
  ASSERT_TRUE(std::is_trivially_move_assignable_v<variant5>);
  ASSERT_TRUE(std::is_move_assignable_v<variant6>);
  ASSERT_TRUE(std::is_move_assignable_v<variant7>);
  ASSERT_FALSE(std::is_nothrow_move_assignable_v<variant6>);
  ASSERT_FALSE(std::is_nothrow_move_assignable_v<variant7>);
  ASSERT_TRUE(std::is_nothrow_move_assignable_v<variant3>);
  ASSERT_TRUE(std::is_nothrow_move_assignable_v<variant4>);
  ASSERT_TRUE(std::is_nothrow_move_assignable_v<variant5>);
}

TEST(traits, converting_assignment) {
  using variant1 = variant<std::string, std::vector<char>, bool>;
  bool assignment1 = std::is_assignable_v<variant1 &, std::string &&>;
  bool assignment2 = std::is_assignable_v<variant1 &, const char *>;
  bool assignment3 = std::is_assignable_v<variant1 &, size_t>;
  bool assignment4 = std::is_nothrow_assignable_v<variant1 &, std::string &&>;
  bool assignment5 = std::is_nothrow_assignable_v<variant1 &, const std::string &>;
  bool assignment6 = std::is_assignable_v<variant1 &, double *>;
  ASSERT_TRUE(assignment1);
  ASSERT_TRUE(assignment2);
  ASSERT_FALSE(assignment3);
  ASSERT_TRUE(assignment4);
  ASSERT_FALSE(assignment5);
  ASSERT_FALSE(assignment6);
}

TEST(traits, variant_size) {
    using variant1 = variant<int, std::string, variant<int, std::vector<int>, size_t>, bool>;
    ASSERT_EQ(variant_size_v<variant1>, 4);
    ASSERT_EQ(variant_size_v<variant1>, variant_size_v<const variant1>);
    ASSERT_EQ(variant_size_v<variant1>, variant_size<variant1>::value);
    ASSERT_EQ(variant_size_v<variant1>, variant_size<variant1>{});
    ASSERT_EQ(variant_size_v<variant1>, variant_size<variant1>{}());
}

TEST(traits, variant_alternative) {
    using variant1 = variant<int, std::string, variant<int, std::vector<int>, size_t>, bool>;
    using T1 = variant_alternative_t<1, variant1>;
    using T2 = typename variant_alternative<1, variant1>::type;
    using T3 = variant_alternative_t<1, const variant1>;
    bool res1 = std::is_same_v<T1, std::string>;
    bool res2 = std::is_same_v<T1, T2>;
    bool res3 = std::is_same_v<const T1, T3>;
    ASSERT_TRUE(res1);
    ASSERT_TRUE(res2);
    ASSERT_TRUE(res3);
}

static_assert(variant<int>().index() == 0, "Constexpr empty ctor failed");
static_assert(holds_alternative<int>(variant<int, double>()), "Constexpr empty ctor holds_alternative test failed");
static_assert(holds_alternative<int>(variant<int>()), "Constexpr empty ctor holds_alternative test failed");
static_assert(variant<int, double>().index() == 0, "Constexpr empty ctor failed");

TEST(correctness, empty_ctor) {
    variant<int, double> v;
    ASSERT_TRUE(v.index() == 0);
    ASSERT_TRUE(holds_alternative<int>(v));
}

constexpr bool simple_copy_ctor_test() {
    variant<int, double> x{42.0};
    variant<int, double> other{x};
    if (x.index() != other.index())
        return false;
    if (get<1>(x) != get<1>(other))
        return false;
    if (!holds_alternative<double>(x) || !holds_alternative<double>(other))
        return false;
    return true;
}

static_assert(simple_copy_ctor_test(), "Basic constexpr copy-constructor failed");

TEST(correctness, copy_ctor1) { ASSERT_TRUE(simple_copy_ctor_test()); }

TEST(correcntess, copy_constructor_without_default) {
  variant<no_default_t, non_trivial_copy_t> orig(std::in_place_index<1>, 123);
  variant<no_default_t, non_trivial_copy_t> copy(orig);
  ASSERT_EQ(orig.index(), copy.index());
  ASSERT_EQ(get<1>(orig).x + 1, get<non_trivial_copy_t>(copy).x);
}

constexpr bool direct_init_copy_ctor() {
  variant<no_copy_assignment_t> x;
  variant<no_copy_assignment_t> other{x};
  if (!holds_alternative<no_copy_assignment_t>(x) || !holds_alternative<no_copy_assignment_t>(other))
    return false;
  return true;
}

TEST(correctness, copy_ctor2) { ASSERT_TRUE(direct_init_copy_ctor()); }

constexpr bool simple_move_ctor_test() {
  {
    variant<no_copy_assignment_t> x;
    variant<no_copy_assignment_t> other{std::move(x)};
    if (!holds_alternative<no_copy_assignment_t>(x) || !holds_alternative<no_copy_assignment_t>(other))
      return false;
  }
  {
    variant<int, double> x{42};
    variant<int, double> y = std::move(x);
    if (x.index() != y.index()
     || x.index() != 0
     || get<0>(x) != get<0>(y))
      return false;
  }
  return true;
}

static_assert(simple_move_ctor_test(), "Simple constexpr move test failed");

TEST(correctness, move_ctor) {
    simple_move_ctor_test();

    variant<coin_wrapper> x;
    variant<coin_wrapper> y = std::move(x);
    ASSERT_TRUE(!get<0>(x).has_coins());
    ASSERT_TRUE(get<0>(y).has_coins() == 1);
}

constexpr bool simple_value_move_ctor() {
    {
        only_movable x;
        variant<only_movable> y(std::move(x));
        if (x.has_coin() || !get<0>(y).has_coin())
            return false;
    }
    {
        coin_wrapper x;
        variant<coin_wrapper> y(std::move(x));
        if (x.has_coins() || !get<0>(y).has_coins())
            return false;
    }
    return true;
}

static_assert(simple_value_move_ctor(), "Simple value-forwarding ctor failed");

TEST(correctness, value_move_ctor) {
  simple_value_move_ctor();
  variant<int, coin_wrapper> x(yac_coin{});
  ASSERT_TRUE(x.index() == 0);
}

TEST(correctness, alternative_selection) {
  {
    variant<char, std::optional<char16_t>> v = u'\u2043';
    ASSERT_EQ(v.index(), 1);
  }
  {
    double d = 3.14;
    variant<int, std::reference_wrapper<double>> y = d;
    ASSERT_EQ(y.index(), 1);
  }
  // For brave and truth
  {
    // See NB in #4 https://en.cppreference.com/w/cpp/utility/variant/variant
    variant<bool, std::string> v("asdasd");
    ASSERT_EQ(v.index(), 1); // Overload resolution is not your friend anymore
  }
  {
    variant<long, double, float> v = 0;
    ASSERT_EQ(v.index(), 0);
  }
  {
    variant<std::vector<int>, bool, std::string> a(true);
    ASSERT_EQ(a.index(), 1);
  }
}

TEST(correctness, valueless_by_exception) {
  using V = variant<std::vector<int>, throwing_move_operator_t>;
  auto v1 = std::vector{1, 2, 3};
  V v = v1;
  ASSERT_ANY_THROW({
    V tmp(std::in_place_index<1>);
    v = std::move(tmp);
  });
  ASSERT_TRUE(v.valueless_by_exception());
  auto v2 = std::vector{4, 5, 6};
  V w = v2;
  ASSERT_FALSE(w.valueless_by_exception());
  ASSERT_EQ(get<std::vector<int>>(w), v2);
  w.swap(v);
  ASSERT_TRUE(w.valueless_by_exception());
  ASSERT_FALSE(v.valueless_by_exception());
  ASSERT_EQ(get<0>(v), v2);
  w.swap(v);
  ASSERT_TRUE(v.valueless_by_exception());
  ASSERT_FALSE(w.valueless_by_exception());
  ASSERT_EQ(get<0>(w), v2);
}

TEST(correctness, assign) {
  struct bruh_conversion {
    bruh_conversion(int) { throw std::exception(); }
  };
  std::string s = "here comes some std::string";
  variant<std::string, bruh_conversion> v = s;
  ASSERT_ANY_THROW(v = 42);
  ASSERT_EQ(get<0>(v), s);
}

TEST(correctness, visit) {
  using V = variant<int, long, double>;
  V v1 = 42;
  V v2 = 1337L;
  V v3 = 0.5;
  bool was_called = false;
  visit([&](int i, long l, double d){
    ASSERT_EQ(i, 42);
    ASSERT_EQ(l, 1337L);
    ASSERT_EQ(d, 0.5);
    was_called = true;
  }, v1, v2, v3);
  ASSERT_TRUE(was_called);
}

TEST(correctness, emplace) {
  using V = variant<std::vector<int>, std::string>;
  std::string s = "A fairly long string that will cause an allocation";
  std::vector<int> t = {1, 2, 3};
  V v = s;
  ASSERT_EQ(v.index(), 1);
  v.emplace<0>(t);
  ASSERT_EQ(v.index(), 0);
  ASSERT_EQ(get<0>(v), t);
  v.emplace<std::string>(s);
  ASSERT_EQ(v.index(), 1);
  ASSERT_EQ(get<1>(v), s);
  v.emplace<0>(t);
  ASSERT_EQ(v.index(), 0);
  ASSERT_EQ(get<0>(v), t);
}

constexpr bool in_place_ctor() {
    variant<bool, double> x1(std::in_place_type<double>, 42);
    variant<bool, double> x2(std::in_place_index<1>, 42);
    return (x1.index() == 1 && get<1>(x1) == 42.0)
           && (x2.index() == 1 && get<1>(x2) == 42.0);
}

static_assert(in_place_ctor(), "Simple in-place ctor failed");

TEST(correctness, inplace_ctors) {
    in_place_ctor();

    variant<bool, std::string> why_not(std::in_place_type<bool>, "asdasd");
    ASSERT_TRUE(why_not.index() == 0 && get<0>(why_not));

    variant<bool, std::string> x2(std::in_place_index<0>, "asdasd");
    ASSERT_TRUE(x2.index() == 0 && get<0>(x2));

    variant<std::string, std::vector<int>, char> var{std::in_place_index<1>, std::vector<int>{1, 2, 3, 4, 5}};
    auto other = std::vector<int>{1, 2, 3, 4, 5};
    ASSERT_EQ(get<1>(var), other);
    auto other2 = std::vector<int>(4, 42);
    variant<std::string, std::vector<int>, char> var2{std::in_place_index<1>, 4, 42};
    ASSERT_EQ(get<1>(var2), other2);
}

TEST(correctness, variant_exceptions1) {
    variant<throwing_move_operator_t> x;
    try {
        x.emplace<throwing_move_operator_t>(throwing_move_operator_t{});
    } catch (std::exception const &item) {
        ASSERT_TRUE(x.valueless_by_exception());
        ASSERT_EQ(x.index(), variant_npos);
        ASSERT_THROW(get<0>(x), bad_variant_access);
        return;
    }
    FAIL();
}

constexpr bool get_if_test_basic() {
    variant<float, double, long double> v = 4.5;
    if (!get_if<double>(&v))
        return false;
    return true;
}

static_assert(get_if_test_basic(), "Bad get_if behavior");

TEST(correctness, multiple_same_types) {
  variant<int, const int, int const, volatile int const> v;
  v.emplace<int>(4);
  ASSERT_TRUE(holds_alternative<int>(v));
  ASSERT_TRUE(v.index() == 0);
  ASSERT_TRUE(get_if<int>(&v));
  ASSERT_TRUE(get_if<0>(&v));
  ASSERT_TRUE(get<int>(v) == 4);
  ASSERT_TRUE(get<0>(v) == 4);

  v.emplace<1>(4);
  ASSERT_TRUE(v.index() == 1);
  ASSERT_TRUE(get_if<1>(&v));
  ASSERT_TRUE(get<1>(v) == 4);

  v.emplace<2>(4);
  ASSERT_TRUE(v.index() == 2);
  ASSERT_TRUE(get_if<2>(&v));
  ASSERT_TRUE(get<2>(v) == 4);

  ASSERT_THROW(get<1>(v), bad_variant_access);
}

TEST(visits, visit_valueless) {
    variant<throwing_move_operator_t> x;
    try {
        x.emplace<throwing_move_operator_t>(throwing_move_operator_t{});
    } catch (std::exception const &item) {
        ASSERT_TRUE(x.valueless_by_exception());
        auto visitor = [](auto &&x) {};
        ASSERT_THROW(visit(visitor, x), bad_variant_access);
        return;
    }

    assert(false && "Exception expected");
}

TEST(visits, visit_on_multiple) {
    variant<int, const int, int const, double> v;
    v.emplace<2>(42);
    auto visitor = [](auto x) -> int { return x; };
    auto result = visit(visitor, v);
    ASSERT_EQ(result, 42);

    auto visitor2 = [](int x) -> int { return x; };
    result = visit(visitor2, v);
    ASSERT_EQ(result, 42);

    auto visitor3 = [](double const x) -> int { return x; };
    result = visit(visitor3, v);
    ASSERT_EQ(result, 42);
}

TEST(visits, visit_overload) {
    variant<char const *> v = "abce";
    auto visitor = overloaded{
            [](std::string) -> bool { return false; },
            [](bool) -> bool { return true; }
    };
    ASSERT_TRUE(visit(visitor, v));
}

constexpr bool test_visit() {
  using V = variant<int, short, long>;
  V a1(1);
  V b1(2);
  V c1(3);
  bool res1 = (visit(sqr_sum_visitor{}, a1, b1, c1) == 14);

  V a2(std::in_place_index<0>, 2);
  V b2(std::in_place_index<1>, 2);
  V c2(std::in_place_index<2>, 2);
  bool res2 = (visit(sqr_sum_visitor{}, a2, b2, c2) == 12);
  return res1 && res2;
}

static_assert(test_visit(), "Visit is not constexpr");


TEST(swap, valueless) {
  throwing_move_operator_t::swap_called = 0;
  using V = variant<int, throwing_move_operator_t>;
  V a = 14;
  V b = 88;
  ASSERT_ANY_THROW({
    V tmp(std::in_place_index<1>);
    a = std::move(tmp);
  });
  ASSERT_ANY_THROW({
    V tmp(std::in_place_index<1>);
    b = std::move(tmp);
  });
  ASSERT_TRUE(a.valueless_by_exception());
  ASSERT_TRUE(b.valueless_by_exception());
  a.swap(b);
  ASSERT_EQ(throwing_move_operator_t::swap_called, 0);
}

TEST(swap, same_alternative) {
  throwing_move_operator_t::swap_called = 0;
  using V = variant<int, throwing_move_operator_t>;
  V a(std::in_place_index<1>);
  V b(std::in_place_index<1>);
  a.swap(b);
  ASSERT_EQ(throwing_move_operator_t::swap_called, 1);
}

TEST(swap, different_alternatives) {
  using V = variant<int, std::string, dummy_t>;
  V a(42);
  V b("kek");
  V c(std::in_place_index<2>);
  a.swap(b);
  b.swap(c);
  ASSERT_TRUE(holds_alternative<std::string>(a));
  ASSERT_TRUE(holds_alternative<dummy_t>(b));
  ASSERT_TRUE(holds_alternative<int>(c));
  ASSERT_EQ(get<std::string>(a), "kek");
  ASSERT_EQ(get<int>(c), 42);
}

TEST(assignment, same_alternative) {
  using V = variant<non_trivial_int_wrapper_t, non_trivial_copy_assignment_t>;
  V a(std::in_place_type<non_trivial_copy_assignment_t>, 42);
  V b(std::in_place_type<non_trivial_copy_assignment_t>, 14882);
  a = b;
  ASSERT_EQ(get<1>(a).x, 14882 + 5);
}

TEST(assignment, back_and_forth) {
  using V = variant<non_trivial_int_wrapper_t, non_trivial_copy_assignment_t>;
  V a = non_trivial_int_wrapper_t(42);
  V b = non_trivial_copy_assignment_t(14882);
  ASSERT_EQ(get<0>(a).x, 42);
  a = 42;
  ASSERT_EQ(get<0>(a).x, 43);
  a = non_trivial_copy_assignment_t(42);
  ASSERT_EQ(get<1>(a).x, 42);
  b = a;
  ASSERT_EQ(get<1>(b).x, 47);
  a = b;
  ASSERT_EQ(get<1>(a).x, 52);
}

TEST(assignment, move_only) {
  only_movable::move_assignment_called = 0;
  using V = variant<only_movable>;
  V a(std::in_place_type<only_movable>);
  V b(std::in_place_type<only_movable>);
  a = std::move(b);
  ASSERT_TRUE(get<0>(a).has_coin());
  ASSERT_FALSE(get<0>(b).has_coin());
  ASSERT_EQ(only_movable::move_assignment_called, 1);
}

TEST(assignment, different_alternatives) {
  using V = variant<std::vector<int>, std::vector<double>>;
  V a = std::vector{13.37, 2020.02};
  V b = std::vector{1337, 14882};
  a = b;
  ASSERT_TRUE(holds_alternative<std::vector<int>>(a));
}

TEST(constructor, move_only) {
  using V = variant<only_movable>;
  V a(std::in_place_type<only_movable>);
  V b(std::move(a));
  ASSERT_TRUE(get<0>(b).has_coin());
  ASSERT_FALSE(get<0>(a).has_coin());
}
