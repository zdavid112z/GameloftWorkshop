#include "stdafx.h"
#include "Globals.h"
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include "rapidxml_utils.hpp"
#include "rapidxml_iterators.hpp"
#include <fstream>
#include <iostream>

int Globals::screenWidth;
int Globals::screenHeight;
std::string Globals::windowTitle;
Vector4 Globals::clearColor;

std::unordered_map<std::string, GlobalData> Globals::globals;

void Globals::AddGlobal(const std::string& name, GlobalData data)
{
	globals[name] = data;
}

void Globals::LoadGlobals()
{
	globals["screenWidth"] = GlobalData::CreateInt(&screenWidth);
	globals["screenHeight"] = GlobalData::CreateInt(&screenHeight);
	globals["windowTitle"] = GlobalData::CreateString(&windowTitle);
	globals["clearColor"] = GlobalData::CreateVec4(&clearColor);

	rapidxml::file<> fin("config.xml");
	rapidxml::xml_document<> doc;
	doc.parse<0>(fin.data());

	rapidxml::xml_node<>* root = doc.first_node();
	for (auto node = root->first_node("object"); node; node = node->next_sibling("object"))
	{
		auto attribute = node->first_attribute("name");
		char* name = attribute->value();
		auto it = globals.find(std::string(name));
		if (it == globals.end())
			continue;
		GlobalData data = it->second;
		char* value;
		auto valueAttribute = node->first_attribute("value");
		if (valueAttribute != 0)
			value = valueAttribute->value();
		else value = node->value();
		float* v = 0;
		int numVComp = 0;
		switch (data.type)
		{
		case VariableType::INT:
			*(int*)data.address = atoi(value);
			break;
		case VariableType::FLOAT:
			*(float*)data.address = (float)atof(value);
			break;
		case VariableType::STRING:
			*(std::string*)data.address = std::string(value);
			break;
		case VariableType::VEC2:
			numVComp = 2;
		case VariableType::VEC3:
			if(!numVComp)
				numVComp = 3;
		case VariableType::VEC4:
			if (!numVComp)
				numVComp = 4;
			v = (float*)data.address;
			{

				for (int i = 0; i < numVComp; i++)
				{
					char* beg = value;
					while (!isdigit(*value) && *value != '-' && *value != '.' && *value != 'e' && *value != 'E')
						value++;
					beg = value;
					while (isdigit(*value) || *value == '-' || *value == '.' || *value == 'e' || *value == 'E')
						value++;
					*value = 0;
					v[i] = (float)atof(beg);
				}
			}
			break;
		}
	}
}