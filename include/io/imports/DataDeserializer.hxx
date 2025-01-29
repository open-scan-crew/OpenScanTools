#ifndef DATADESERIALIZER_HXX_
#define DATADESERIALIZER_HXX_

#include "DataDeserializer.h"

#include "io/SerializerKeys.h"

#include "utils/Utils.h"

template<class ListType>
bool DataDeserializer::DeserializeList(const nlohmann::json& json, ListType& data)
{
	if (json.find(Key_Name) != json.end())
		data.setName(Utils::from_utf8(json.at(Key_Name).get<std::string>()));
	else
		return false;

	if (json.find(Key_Id) != json.end())
		data.setId(xg::Guid(json.at(Key_Id).get<std::string>()));
	else
		return false;

	if (json.find(Key_Origin) != json.end())
		data.setOrigin(json.at(Key_Origin).get<bool>());

	if (json.find(Key_Elements) != json.end() && json.at(Key_Elements).is_array())
		for (const nlohmann::json& itItem : json.at(Key_Elements))
			if (itItem.find(Key_Name) != itItem.end())
			{
				if(itItem.at(Key_Name).type() == nlohmann::detail::value_t::string)
					data.insertStrValue(Utils::from_utf8(itItem.at(Key_Name).get<std::string>()));
				else
					data.insertStrValue(std::to_wstring(itItem.at(Key_Name).get<double>()));

			}
	return true;
}

#endif // !DATADESERIALIZER_HXX_