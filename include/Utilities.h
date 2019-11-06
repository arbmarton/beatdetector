#pragma once

#include <filesystem>

std::filesystem::path getShaderFolderPath();
std::filesystem::path getShaderPath(const std::string& shaderName);

std::filesystem::path getSoundsFolderPath();
std::filesystem::path getSoundPath(const std::string& soundName);