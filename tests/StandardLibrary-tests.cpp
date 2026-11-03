#include "doctest.h"
#include <string>

import junopl.stl;

using JunoPL::JunoSTL;
using Qualifiers = JunoSTL::Qualifiers;
using Operations = JunoSTL::Operations;

TEST_CASE("STL - qualified lookup") {
    auto delay =
        JunoSTL::find(Qualifiers::GAME, "delay", Operations::INSTRUCTION);
    REQUIRE(delay.has_value());
    CHECK(delay->qualifier == Qualifiers::GAME);
    CHECK(delay->operation == Operations::INSTRUCTION);
    CHECK(delay->accepts(1));
    CHECK_FALSE(delay->accepts(0));
    CHECK_FALSE(delay->accepts(2));

    // Registered as an instruction only
    CHECK_FALSE(JunoSTL::find(Qualifiers::GAME, "delay", Operations::EXPRESSION)
                    .has_value());
    // Wrong qualifier
    CHECK_FALSE(
        JunoSTL::find(Qualifiers::MATH, "delay", Operations::INSTRUCTION)
            .has_value());
    CHECK_FALSE(JunoSTL::find(Qualifiers::GAME, "noSuchFunction",
                              Operations::INSTRUCTION)
                    .has_value());
}

TEST_CASE("STL - qualifier strings and aliases") {
    CHECK(JunoSTL::qualifierFromString("String") == Qualifiers::STRINGS);
    CHECK(JunoSTL::qualifierFromString("Strings") == Qualifiers::STRINGS);
    CHECK(JunoSTL::qualifierFromString("") == Qualifiers::COMMON);
    CHECK(JunoSTL::qualifierFromString("Unknown") == Qualifiers::COMMON);
    CHECK(JunoSTL::qualifierToString(Qualifiers::STRINGS) == "Strings");
    CHECK(JunoSTL::qualifierToString(Qualifiers::COMMON) == "Common");

    auto join = JunoSTL::find("String", "join", Operations::EXPRESSION);
    REQUIRE(join.has_value());
    CHECK(join->accepts(2));
    CHECK(join->accepts(5));
    CHECK_FALSE(join->accepts(1));
}

TEST_CASE("STL - instruction and expression overloads") {
    // Input.throttle(x) sets the throttle, Input.throttle() reads it
    auto setThrottle =
        JunoSTL::find(Qualifiers::INPUT, "throttle", Operations::INSTRUCTION);
    REQUIRE(setThrottle.has_value());
    CHECK(setThrottle->accepts(1));
    CHECK_FALSE(setThrottle->accepts(0));

    auto getThrottle =
        JunoSTL::find(Qualifiers::INPUT, "throttle", Operations::EXPRESSION);
    REQUIRE(getThrottle.has_value());
    CHECK(getThrottle->accepts(0));
    CHECK_FALSE(getThrottle->accepts(1));

    // Part.name(partID, newName) renames, Part.name(id) reads
    auto renamePart =
        JunoSTL::find(Qualifiers::PART, "name", Operations::INSTRUCTION);
    REQUIRE(renamePart.has_value());
    CHECK(renamePart->accepts(2));

    auto readPartName =
        JunoSTL::find(Qualifiers::PART, "name", Operations::EXPRESSION);
    REQUIRE(readPartName.has_value());
    CHECK(readPartName->accepts(1));
}

TEST_CASE("STL - arity ranges") {
    // Camera.targetOffset(vector) or Camera.targetOffset(x, y, z)
    auto targetOffset = JunoSTL::find(Qualifiers::CAMERA, "targetOffset",
                                      Operations::INSTRUCTION);
    REQUIRE(targetOffset.has_value());
    CHECK(targetOffset->accepts(1));
    CHECK_FALSE(targetOffset->accepts(2));
    CHECK(targetOffset->accepts(3));

    // Craft.name() and Craft.name(id)
    auto craftName =
        JunoSTL::find(Qualifiers::CRAFT, "name", Operations::EXPRESSION);
    REQUIRE(craftName.has_value());
    CHECK(craftName->accepts(0));
    CHECK(craftName->accepts(1));
    CHECK_FALSE(craftName->accepts(2));

    // Lists.insert(ident, value) or Lists.insert(ident, value, value, value)
    auto insert =
        JunoSTL::find(Qualifiers::LISTS, "insert", Operations::INSTRUCTION);
    REQUIRE(insert.has_value());
    CHECK(insert->accepts(2));
    CHECK_FALSE(insert->accepts(3));
    CHECK(insert->accepts(4));

    // Variadic: String.format(string, values...)
    auto format =
        JunoSTL::find(Qualifiers::STRINGS, "format", Operations::EXPRESSION);
    REQUIRE(format.has_value());
    CHECK(format->accepts(1));
    CHECK(format->accepts(9));
}

TEST_CASE("STL - zero-argument getters") {
    auto vecX =
        JunoSTL::find(Qualifiers::VECTOR, "vecX", Operations::EXPRESSION);
    REQUIRE(vecX.has_value());
    CHECK(vecX->accepts(0));
    CHECK_FALSE(vecX->accepts(1));

    auto altitude =
        JunoSTL::find(Qualifiers::ALTITUDE, "getAGL", Operations::EXPRESSION);
    REQUIRE(altitude.has_value());
    CHECK(altitude->accepts(0));

    auto lockPrograde = JunoSTL::find(Qualifiers::CRAFT, "lockPrograde",
                                      Operations::INSTRUCTION);
    REQUIRE(lockPrograde.has_value());
    CHECK(lockPrograde->accepts(0));
}

TEST_CASE("STL - arity descriptions") {
    auto beep =
        JunoSTL::find(Qualifiers::GAME, "beep", Operations::INSTRUCTION);
    REQUIRE(beep.has_value());
    CHECK(beep->describeArities() == "3 arguments");

    auto delay =
        JunoSTL::find(Qualifiers::GAME, "delay", Operations::INSTRUCTION);
    REQUIRE(delay.has_value());
    CHECK(delay->describeArities() == "1 argument");

    auto join =
        JunoSTL::find(Qualifiers::STRINGS, "join", Operations::EXPRESSION);
    REQUIRE(join.has_value());
    CHECK(join->describeArities() == "at least 2 arguments");

    auto targetOffset = JunoSTL::find(Qualifiers::CAMERA, "targetOffset",
                                      Operations::INSTRUCTION);
    REQUIRE(targetOffset.has_value());
    CHECK(targetOffset->describeArities() == "1 argument or 3 arguments");

    auto insert =
        JunoSTL::find(Qualifiers::LISTS, "insert", Operations::INSTRUCTION);
    REQUIRE(insert.has_value());
    CHECK(insert->describeArities() == "2 arguments or 4 arguments");
}

TEST_CASE("STL - doc entries") {
    // Game.frequencyOf is an expression
    CHECK(JunoSTL::find(Qualifiers::GAME, "frequencyOf", Operations::EXPRESSION)
              .has_value());
    CHECK_FALSE(
        JunoSTL::find(Qualifiers::GAME, "frequencyOf", Operations::INSTRUCTION)
            .has_value());

    // Game.funk lives under Game now
    CHECK(JunoSTL::find(Qualifiers::GAME, "funk", Operations::EXPRESSION)
              .has_value());

    // Craft.currentStage replaced Craft.stage
    CHECK(
        JunoSTL::find(Qualifiers::CRAFT, "currentStage", Operations::EXPRESSION)
            .has_value());
    CHECK_FALSE(
        JunoSTL::find(Qualifiers::CRAFT, "stage", Operations::EXPRESSION)
            .has_value());

    // List expressions are qualified
    CHECK(JunoSTL::find(Qualifiers::LISTS, "length", Operations::EXPRESSION)
              .has_value());
    CHECK(JunoSTL::find(Qualifiers::LISTS, "idxOf", Operations::EXPRESSION)
              .has_value());

    // Events - variables
    CHECK(
        JunoSTL::find(Qualifiers::INPUT, "setVariable", Operations::INSTRUCTION)
            .has_value());
}
