//
// Created by marcin on 07/02/19.
//

#include "catch2/catch.hpp"
#include <cpga/common.hpp>

using namespace std::string_literals;
using cpga::str;

TEST_CASE("str correctly concatenates its arguments", "[str]") {
  SECTION("when all arguments are std::strings") {
    auto result{str("a "s, "few "s, "std::strings"s)};
    auto expected{"a few std::strings"};

    REQUIRE(result == expected);
  }

  SECTION("when arguments are std::strings and c-strings") {
    auto result{str("a "s, "few ", "std::strings")};
    auto expected{"a few std::strings"};

    REQUIRE(result == expected);
  }

  SECTION("when arguments are a mixture of string and numeric types") {
    auto result{str("a "s, "few ", "numbers: ", 1, ", ", 1.2, ", ", 2ul)};
    auto expected{"a few numbers: 1, 1.2, 2"};

    REQUIRE(result == expected);
  }

  SECTION("when argument includes a boolean") {
    auto result{str("a bool: ", true, ", another: ", false)};
    auto expected{"a bool: 1, another: 0"};

    REQUIRE(result == expected);
  }
}

TEST_CASE("is_same evaluates correctly", "[is_same]") {
  SECTION("for a std::vector") {
    std::vector<std::string> expected;

    REQUIRE(cpga::is_same<decltype(expected), std::vector<std::string>>());
  }

  SECTION("for an integer") {
    int expected;

    REQUIRE(cpga::is_same<decltype(expected), int>());
  }
}

TEST_CASE("is_size_constructible evaluates correctly", "[is_size_constructible]") {
  SECTION("for a std::vector") {
    REQUIRE(cpga::is_size_constructible<std::vector<int>>());
  }

  SECTION("for a std::list") {
    REQUIRE(cpga::is_size_constructible<std::list<int>>());
  }

  SECTION("for a std::forward_list") {
    REQUIRE(cpga::is_size_constructible<std::forward_list<int>>());
  }

  SECTION("for a std::deque") {
    REQUIRE(cpga::is_size_constructible<std::deque<int>>());
  }
}

TEST_CASE("join concatenates sequences correctly correctly", "[join]") {
  SECTION("for a std::vector<std::string>") {
    std::vector<std::string> items{
        "first",
        "second",
        "third"
    };

    auto expected = "first,second,third";

    REQUIRE(cpga::join(items, ",") == expected);
  }

  SECTION("for a std::vector<std::string>") {
    std::vector<const char *> items{
        "first",
        "second",
        "third"
    };

    auto expected = "first,second,third";

    REQUIRE(cpga::join(items, ",") == expected);
  }

  SECTION("for a std::vector<int>") {
    std::vector<int> items{
        1, 2, 3, 4, 5
    };

    auto expected = "1,2,3,4,5";

    REQUIRE(cpga::join(items, ",") == expected);
  }

  SECTION("with default delimiter") {
    std::vector<int> items{
        1, 2, 3, 4, 5
    };

    auto expected = "1,2,3,4,5";

    REQUIRE(cpga::join(items) == expected);
  }
}