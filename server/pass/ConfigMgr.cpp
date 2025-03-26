#include "ConfigMgr.h"

ConfigMgr::ConfigMgr()
{
	boost::filesystem::path current_path = boost::filesystem::current_path();
	boost::filesystem::path config_path = current_path / "config.ini";

	std::cout << "Config path: " << config_path << std::endl;


	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(config_path.string(), pt);


	for (const auto& dection_pair : pt) {
		const std::string& section_name = dection_pair.first;
		const boost::property_tree::ptree& section_tree = dection_pair.second;

		std::map < std::string, std::string> section_config;
		for (const auto& key_value_pair : section_tree) {
			const std::string& key = key_value_pair.first;
			const std::string& value = key_value_pair.second.get_value<std::string>();
			section_config[key] = value;
		}
		SectionInfo sectionInfo;
		sectionInfo._section_datas = section_config;
		_config_datas[section_name] = sectionInfo;
	}



	//print _config_datas
	for (const auto& section_entry : _config_datas) {
		const std::string& section_name = section_entry.first;
		const SectionInfo& section_info = section_entry.second;

		std::cout << "Section: " << section_name << std::endl;
		for (const auto& key_value : section_info._section_datas) {
			std::cout << "key: " <<key_value.first << "  value: " << key_value.second << std::endl;
		}

	}


}
