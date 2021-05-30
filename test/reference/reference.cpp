/*
 * Copyright 2019-present Julián Bermúdez Ortega.
 *
 * This file is part of julibert::cpputils.
 *
 * julibert::cpputils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * julibert::cpputils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with julibert::cpputils.  If not, see <https://www.gnu.org/licenses/>.
 */
#define CATCH_CONFIG_MAIN

#include <julibert/cpputils/reference/reference.hpp>

#include <catch2/catch.hpp>

struct MyReferenceImpl
{
public:
  MyReferenceImpl() = default;

  MyReferenceImpl(uint8_t foo, uint8_t bar)
    : foo_{ foo }
    , bar_{ bar }
  {}

  friend bool operator==(const MyReferenceImpl& lhs, const MyReferenceImpl& rhs)
  {
    return (lhs.foo_ == rhs.foo_) && (lhs.bar_ == rhs.bar_);
  }

  void foo(uint8_t foo) { foo_ = foo; }
  uint8_t foo() const { return foo_; }
  uint8_t bar() const { return bar_; }

private:
  uint8_t foo_ = 0;
  uint8_t bar_ = 0;
};

using MyReference = julibert::Reference<MyReferenceImpl>;

SCENARIO("Reference")
{
  GIVEN("a Reference")
  {
    MyReference foo(std::in_place, 11, 89);

    THEN("it can be assigned from a prvalue")
    {
      MyReference foo = MyReference(std::in_place);
      REQUIRE(0 == foo->foo());
      REQUIRE(0 == foo->bar());
    }

    THEN("it can be assigned from a lvalue")
    {
      MyReference bar(std::in_place, 1, 1);
      REQUIRE(1 == bar->foo());
      REQUIRE(1 == bar->bar());

      foo = bar;
      REQUIRE(1 == foo->foo());
      REQUIRE(1 == foo->bar());
    }

    THEN("it can by cloned")
    {
      MyReference bar(MyReference::clone(foo));
      REQUIRE(11 == bar->foo());
      REQUIRE(89 == bar->bar());

      foo->foo(0);
      REQUIRE(11 == bar->foo());
      REQUIRE(0 == foo->foo());
    }

    THEN("we can generate another from this one")
    {
      MyReference bar(foo);
      REQUIRE(11 == bar->foo());
      REQUIRE(89 == bar->bar());

      foo->foo(0);
      REQUIRE(0 == bar->foo());
    }

    THEN("we can generate a constant Rerence copying this one")
    {
      const MyReference bar(foo);
      REQUIRE(11 == bar->foo());
      REQUIRE(89 == bar->bar());

      foo->foo(12);
      REQUIRE(12 == bar->foo());
    }

    THEN("the get methods shall return references to the underlying type")
    {
      const MyReference bar(foo);
      MyReferenceImpl& foo_ref = foo.get();
      // MyReferenceImpl& bar_ref = bar.get(); // It would give an error.
      MyReferenceImpl const& bar_ref = bar.get();
    }
  }

  GIVEN(
    "a constant Reference, its constant implementation methods are available")
  {
    const MyReference foo(std::in_place, 11, 89);
    // foo->set_foo(11); // It would give an error.
    REQUIRE(11 == foo->foo());
  }

  GIVEN("a Reference, its implementation methods are available")
  {
    MyReference foo(std::in_place, 0, 0);
    foo->foo(11);
    REQUIRE(11 == foo->foo());
  }
}

SCENARIO("Reference operator==")
{
  GIVEN("two References")
  {
    WHEN("one is a copy of the other")
    {
      MyReference foo(std::in_place, 11, 89);
      MyReference bar(foo);

      THEN("operator== shall return true")
      {
        REQUIRE(foo == bar);
        REQUIRE_FALSE(foo != bar);
      }
    }

    WHEN("they are contructed independenly")
    {
      THEN("if they are equal the operator== shall return true")
      {
        MyReference foo(std::in_place, 11, 89);
        MyReference bar(std::in_place, 11, 89);
        REQUIRE(foo == bar);
        REQUIRE_FALSE(foo != bar);
      }

      THEN("if they are different the operator== shall return false")
      {
        MyReference foo(std::in_place, 11, 89);
        MyReference bar(std::in_place, 11, 79);
        REQUIRE_FALSE(foo == bar);
        REQUIRE(foo != bar);
      }
    }
  }
}