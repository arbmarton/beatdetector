#include "Utilities.h"

std::filesystem::path getShaderFolderPath()
{
    std::string root = std::filesystem::current_path().parent_path().string();
    root.append("/shaders/");
    return std::filesystem::path(root);
}

std::filesystem::path getShaderPath(const std::string& shaderName)
{
    return std::filesystem::path(getShaderFolderPath().string() + shaderName);
}

std::filesystem::path getSoundsFolderPath()
{
	std::string root = std::filesystem::current_path().parent_path().string();
	root.append("/sounds/");
	return std::filesystem::path(root);
}

std::filesystem::path getSoundPath(const std::string& soundName)
{
	return std::filesystem::path(getSoundsFolderPath().string() + soundName);
}