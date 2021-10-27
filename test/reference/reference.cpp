/*
 * Copyright 2021-present Julián Bermúdez Ortega
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

  friend bool operator<(MyReferenceImpl const& lhs, MyReferenceImpl const& rhs)
  {
    return (lhs.foo_ < rhs.foo_) ||
           ((lhs.foo_ == rhs.foo_) && (lhs.bar_ < rhs.bar_));
  }

  friend bool operator==(MyReferenceImpl const& lhs, MyReferenceImpl const& rhs)
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
using MyConstReference = julibert::Reference<const MyReferenceImpl>;

SCENARIO("Reference")
{
  GIVEN("a Reference")
  {
    MyReference foo(11, 89);

    THEN("it can be assigned from a prvalue")
    {
      MyReference foo = MyReference();
      REQUIRE(0 == foo->foo());
      REQUIRE(0 == foo->bar());
    }

    THEN("it can be assigned from a lvalue")
    {
      MyReference bar(1, 1);
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

    THEN("we can generate a constant Reference copying this one")
    {
      MyConstReference bar(foo);
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

  GIVEN("a constant Reference")
  {
    THEN("its constant implementation methods are available")
    {
      MyConstReference foo(11, 89);
      // foo->set_foo(11); // It would give an error.
      REQUIRE(11 == foo->foo());
    }

    THEN("it is not possible to create a Reference")
    {
      REQUIRE_FALSE(std::is_convertible_v<MyConstReference, MyReference>);
    }
  }

  GIVEN("a Reference, its implementation methods are available")
  {
    MyReference foo(0, 0);
    foo->foo(11);
    REQUIRE(11 == foo->foo());
  }
}

SCENARIO("Reference operator<")
{
  GIVEN("two References foo and bar")
  {
    WHEN("foo is less than bar")
    {
      MyReference foo(0, 1);
      MyReference bar(1, 0);
      THEN("operator< shall return true")
      {
        REQUIRE(foo < bar);
        REQUIRE_FALSE(bar < foo);
      }
    }

    WHEN("foo is equal than bar")
    {
      MyReference foo(1, 2);
      MyReference bar(1, 2);
      THEN("operator< shall return false")
      {
        REQUIRE_FALSE(foo < bar);
        REQUIRE_FALSE(bar < foo);
      }
    }
  }
}

SCENARIO("Reference operator==")
{
  GIVEN("two References")
  {
    WHEN("one is a copy of the other")
    {
      MyReference foo(11, 89);
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
        MyReference foo(11, 89);
        MyReference bar(11, 89);
        REQUIRE(foo == bar);
        REQUIRE_FALSE(foo != bar);
      }

      THEN("if they are different the operator== shall return false")
      {
        MyReference foo(11, 89);
        MyReference bar(11, 79);
        REQUIRE_FALSE(foo == bar);
        REQUIRE(foo != bar);
      }
    }
  }
}

struct ReferenceBaseImpl
{
  ReferenceBaseImpl() = default;
  int foo;
};

using ReferenceBase = julibert::Reference<ReferenceBaseImpl>;

struct ReferenceDerivedImpl : ReferenceBaseImpl
{
  ReferenceDerivedImpl() = default;
  int bar;
};

using ReferenceDerived = julibert::Reference<ReferenceDerivedImpl>;

SCENARIO("Reference conversion")
{
  GIVEN("a Reference derived")
  {
    ReferenceDerived derived{};
    THEN("it can be converted to Reference base")
    {
      ReferenceBase base = static_cast<ReferenceBase>(derived);
    }
  }
}

using StringRef = julibert::Reference<std::string>;

SCENARIO("Reference initializer_list constructor")
{
  StringRef my_string_ref{ 'a', 'b', 'c', 'd' };
}