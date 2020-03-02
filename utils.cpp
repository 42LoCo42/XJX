#include "utils.h"
#include <iostream>
#include <fstream>
#include "xjx.h"
using namespace std;

void Utils::split(const string& s, const string& delim, vector<string>& parts) {
	size_t pos = 0;
	size_t start = 0;
	std::string token;
	parts.clear();
	while ((pos = s.find(delim, start)) != std::string::npos) {
		token = s.substr(start, pos - start);
		parts.push_back(token);
		start  = pos + delim.length();
	}

	// push last part
	parts.push_back(s.substr(start, s.length() - start));
}

auto Utils::loadXJX(const string& filename) -> bool {
	ifstream file(filename);
	string contents;

	if(file.is_open()) {
		contents.clear();
		string line;
		while(getline(file, line)) {
			contents += line + '\n';
		}
		file.close();
	} else return false;

	vector<string> lines;
	split(contents, "\n", lines);
	if(lines.empty()) return false;

	enum {
		null,
		xjx_header,
		johnny_header,
		ram,
		mc,
		io,
		io_desc,
	} state = null;

	size_t index = 0;
	for(auto& line : lines) {
		if(line.empty() || line[0] == '#') continue;
		if(line == "!XJX") state = xjx_header;
		else if(line == "!Johnny") state = johnny_header;
		else if(line == "!RAM") {
			state = ram;
			index = 0;
		} else if(line == "!MC") {
			state = mc;
			index = 0;
		} else if(line == "!IO") {
			state = io;
		} else if(line == "!IO_DESC") {
			state = io_desc;
		} else if(state == xjx_header) {
			vector<string> header_parts;
			split(line, " ", header_parts);
			XJX::hi_max = stoi(header_parts[0]);
			XJX::lo_max = stoi(header_parts[1]);
			XJX::mc_addr_max = stoi(header_parts[2]);
			XJX::init();
		} else if(state == johnny_header) {
			XJX::hi_max = 19;
			XJX::lo_max = 999;
			XJX::mc_addr_max = 199;
			XJX::init();
		} else if(state == ram) {
			XJX::ram[index] = stoi(line);
			++index;
		} else if(state == mc) {
			if(line[0] >= '0' && line[0] <= '9') {
				XJX::microcode[index] = stoi(line);
				++index;
			} else {
				auto mc = XJX::nameToMC.find(line);
				if(mc != XJX::nameToMC.end()) {
					XJX::microcode[index] = mc->second;
					++index;
				}
			}
		} else if(state == io) {
			vector<string> addrs;
			split(line, " ", addrs);
			if(addrs.size() == 2) {
				XJX::io_min_addr = stoi(addrs[0]);
				XJX::io_max_addr = stoi(addrs[1]);
				state = null;
			}
		} else if(state == io_desc) {
			vector<string> parts;
			split(line, " ", parts);
			if(parts.empty()) continue;
			XJX::io_entries.push_back(parts);
		}
	}

	return true;
}
