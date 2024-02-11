#pragma once
#include<string>
#include <iostream>


std::unordered_map<std::string, std::string> parseQuery(const std::string_view queryString);
std::unordered_map<std::string, std::string> parseParams(const std::string_view url, const std::string_view path);
