// Chemfiles, a modern library for chemistry file reading and writing
// Copyright (C) Guillaume Fraux and contributors -- BSD license

#include <fstream>
#include <sstream>
#include <cstring>

#include "catch.hpp"
#include "helpers.hpp"
#include "chemfiles.h"

static std::string generate_chemfiles_error() {
    // Generate a log event
    CHECK(chfl_trajectory_open("noformat", 'r') == NULL);
    return "Can not find a format associated with the '' extension.";
}

TEST_CASE("Errors", "[CAPI]") {
    CHECK(chfl_last_error() == std::string(""));

    generate_chemfiles_error();

    CHECK(chfl_last_error() != std::string(""));
    CHECK_STATUS(chfl_clear_errors());
    CHECK(chfl_last_error() == std::string(""));
}

TEST_CASE("Configuration", "[CAPI]") {
    CHECK_STATUS(chfl_add_configuration("another_config"));
    CHECK(chfl_add_configuration("not-there") == CHFL_CONFIGURATION_ERROR);
}

TEST_CASE("Version", "[CAPI]") {
    std::ifstream file(VERSION_FILE_PATH);
    REQUIRE(file.is_open());
    std::stringstream content;
    content << file.rdbuf();
    file.close();

    // Remove trailling \n
    std::string version = content.str().substr(0, content.str().length() - 1);
    CHECK(chfl_version() == version);
}

// Global variables for access from callback and main
static char* buffer = NULL;

static void callback(const char* message) {
    size_t size = strlen(message) + 1;
    REQUIRE(buffer == NULL);
    buffer = static_cast<char*>(malloc(sizeof(char) * size));
    strcpy(buffer, message);
}

TEST_CASE("Warnings", "[CAPI]") {
    CHECK_STATUS(chfl_set_warning_callback(callback));

    std::string message = generate_chemfiles_error();

    CHECK(buffer == message);
    free(buffer);
}
