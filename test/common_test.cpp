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