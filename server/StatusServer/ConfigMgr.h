#pragma once

#include"const.h"

#include<map>
struct SectionInfo {

	SectionInfo() {}
	~SectionInfo() {
		_section_datas.clear();
	};

	SectionInfo(const SectionInfo& src) {
		this->_section_datas = src._section_datas;
	};

	SectionInfo& operator = (const SectionInfo& src) {

		if (&src == this) {
			return *this;
		}

		this->_section_datas = src._section_datas;
		return *this;
	};




	std::map<std::string, std::string> _section_datas;

	std::string operator[](const std::string& key) {
		if (_section_datas.find(key) == _section_datas.end()) {
			return "";
		}

		return _section_datas[key];
	}

};


class ConfigMgr
{

public:

	~ConfigMgr() {
		_config_datas.clear();
	}

	ConfigMgr(const ConfigMgr& src) {
		this->_config_datas = src._config_datas;
	}

	ConfigMgr& operator = (const ConfigMgr& src) {
		if (&src == this) {
			return *this;
		}

		this->_config_datas = src._config_datas;
	}

	static ConfigMgr& Inst() {
		static ConfigMgr cfg_mgr;
		return cfg_mgr;
	}

	std::map<std::string, SectionInfo> _config_datas;
	
	SectionInfo operator[](const std::string& key) {
		if (_config_datas.find(key) == _config_datas.end()) {
			return SectionInfo();
		}

			return  _config_datas[key];
	}

private:
	ConfigMgr();


};

