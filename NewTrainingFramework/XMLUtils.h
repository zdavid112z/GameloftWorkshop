#pragma once

#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "stdafx.h"
#include "utils.h"
#include "../Utilities/utilities.h"

class XMLUtils
{
public:
	static uint32 ParseIdFromAttribute(rapidxml::xml_node<>* node, uint32 defaultValue = 0);
	static Vector4 ParseVector4(rapidxml::xml_node<>* node, const Vector4& defaultValue = Vector4());
	static Vector3 ParseVector3(rapidxml::xml_node<>* node, const Vector3& defaultValue = Vector3());
	static Vector2 ParseVector2(rapidxml::xml_node<>* node, const Vector2& defaultValue = Vector2());
	static float ParseFloat(rapidxml::xml_node<>* node, float defaultValue = 0);
	static int ParseInt(rapidxml::xml_node<>* node, int defaultValue = 0);
	static uint ParseUint(rapidxml::xml_node<>* node, uint defaultValue = 0);
	static bool ParseBool(rapidxml::xml_node<>* node, bool defaultValue = 0);
	static std::string ParseString(rapidxml::xml_node<>* node, const std::string& defaultValue);
	static uint32 ParseIdFromValue(rapidxml::xml_node<>* node, uint32 defaultValue = 0);
};