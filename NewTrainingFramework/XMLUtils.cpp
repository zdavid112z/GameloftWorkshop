#include "stdafx.h"
#include "XMLUtils.h"
#include <cstdlib>

uint32 XMLUtils::ParseIdFromAttribute(rapidxml::xml_node<>* node, uint32 id)
{
	auto idAttribute = node->first_attribute("id");
	if (idAttribute != 0)
		id = atoi(std::string(idAttribute->value(), idAttribute->value() + idAttribute->value_size()).c_str());
	else
	{
		auto nameAttribute = node->first_attribute("name");
		if (nameAttribute != 0)
			id = Utils::Hash((uint8*)nameAttribute->value(), nameAttribute->value_size());
	}
	return id;
}

Vector4 XMLUtils::ParseVector4(rapidxml::xml_node<>* node, const Vector4& defaultValue)
{
	Vector4 result;
	if (!node)
		return defaultValue;
	char str[] = "rxgybzaw";
	for (int i = 0; str[i] != 0; i++)
	{
		auto n = node->first_node(&str[i], 1);
		if (n)
			result[i / 2] = (float)atof(Utils::Trim(std::string(n->value(), n->value() + n->value_size())).c_str());
	}
	return result;
}

Vector3 XMLUtils::ParseVector3(rapidxml::xml_node<>* node, const Vector3& defaultValue)
{
	Vector3 result;
	if (!node)
		return defaultValue;
	char str[] = "rxgybz";
	for(int i = 0; str[i] != 0; i++)
	{
		auto n = node->first_node(&str[i], 1);
		if (n)
			result[i / 2] = (float)atof(Utils::Trim(std::string(n->value(), n->value() + n->value_size())).c_str());
	}
	return result;
}

Vector2 XMLUtils::ParseVector2(rapidxml::xml_node<>* node, const Vector2& defaultValue)
{
	Vector2 result;
	if (!node)
		return defaultValue;
	char str[] = "rxgy";
	for (int i = 0; str[i] != 0; i++)
	{
		auto n = node->first_node(&str[i], 1);
		if (n)
			result[i / 2] = (float)atof(Utils::Trim(std::string(n->value(), n->value() + n->value_size())).c_str());
	}
	return result;
}

float XMLUtils::ParseFloat(rapidxml::xml_node<>* node, float defaultValue)
{
	if (!node)
		return defaultValue;
	return (float)atof(Utils::Trim(std::string(node->value(), node->value() + node->value_size())).c_str());
}

int XMLUtils::ParseInt(rapidxml::xml_node<>* node, int defaultValue)
{
	if (!node)
		return defaultValue;
	return atoi(Utils::Trim(std::string(node->value(), node->value() + node->value_size())).c_str());
}

uint XMLUtils::ParseUint(rapidxml::xml_node<>* node, uint defaultValue)
{
	if (!node)
		return defaultValue;
	return atoi(Utils::Trim(std::string(node->value(), node->value() + node->value_size())).c_str());
}

bool XMLUtils::ParseBool(rapidxml::xml_node<>* node, bool defaultValue)
{
	if (!node)
		return defaultValue;
	std::string value = Utils::Trim(std::string(node->value(), node->value() + node->value_size()));
	for (char& c : value)
		c = tolower(c);
	if (value == "true" || value == "1")
		return true;
	if (value == "false" || value == "0")
		return false;
	return defaultValue;
}

std::string XMLUtils::ParseString(rapidxml::xml_node<>* node, const std::string& defaultValue)
{
	if (!node)
		return defaultValue;
	return Utils::Trim(std::string(node->value(), node->value() + node->value_size()));
}

uint32 XMLUtils::ParseIdFromValue(rapidxml::xml_node<>* node, uint32 defaultValue)
{
	if (!node)
		return defaultValue;
	std::string value = Utils::Trim(std::string(node->value(), node->value() + node->value_size()));
	int isnumber = 1;
	for (int i = 0; value[i] != 0; i++)
		if (!isdigit(value[i]))
		{
			isnumber = 0;
			break;
		}
	if (isnumber)
		return atoi(value.c_str());
	return Utils::Hash(value);
}