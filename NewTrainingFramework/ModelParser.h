#pragma once

#include "Vertex.h"
#include <string>

ModelData ParseModel(const std::string& path);
ModelData ParseObj(const std::string& path);