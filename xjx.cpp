#include "xjx.h"
#include <cmath>
#include <stdexcept>
using namespace std;

// Buses
uint XJX::address_bus;
uint XJX::data_bus;

// RAM
vector<uint> XJX::ram;
uint XJX::hi_max = 19;
uint XJX::lo_max = 999;
uint XJX::modulus;

// CPU
uint XJX::ins; // general-purpose register
uint XJX::program_counter;
vector<uint> XJX::microcode;
uint XJX::mc_addr;
uint XJX::mc_addr_max = 199;
map<uint, uint> XJX::asm_reference; // maps assembler to microcode address
uint XJX::mur[3]; // microcode update register, stores address, mop, asm_reference

// ALU
uint XJX::acc;

// Name lookup
map<uint, pair<string, string>> XJX::mcToName;
map<string, uint> XJX::nameToMC;

void XJX::init() {
	if(ram.size() != lo_max + 1) ram.resize(lo_max + 1);
	if(microcode.size() != mc_addr_max + 1) microcode.resize(mc_addr_max + 1);

	modulus = pow(10, floor(log10(lo_max)) + 1);

	mcToName.insert({nop, {"nop", "nop"}});
	mcToName.insert({db_ram, {"db -> ram", "db_ram"}});
	mcToName.insert({ram_db, {"ram -> db", "ram_db"}});
	mcToName.insert({db_ins, {"db -> ins", "db_ins"}});
	mcToName.insert({ins_ab, {"ins -> ab", "ins_ab"}});
	mcToName.insert({ins_mc, {"ins -> mc", "ins_mc"}});
	// u6 is UNUSED
	mcToName.insert({mc_0, {"mc = 0", "mc_0"}});
	mcToName.insert({pc_ab, {"pc -> ab", "pc_ab"}});
	mcToName.insert({pc_inc, {"pc++", "pc_inc"}});
	mcToName.insert({if_0_pc_inc, {"=0:pc++", "if_0_pc_inc"}});
	mcToName.insert({ins_pc, {"ins -> pc", "ins_pc"}});
	mcToName.insert({acc_0, {"acc = 0", "acc_0"}});
	mcToName.insert({plus, {"acc += db", "plus"}});
	mcToName.insert({minus, {"acc -= db", "minus"}});
	mcToName.insert({acc_db, {"acc -> db", "acc_db"}});
	mcToName.insert({acc_inc, {"acc++", "acc_inc"}});
	mcToName.insert({acc_dec, {"acc--", "acc_dec"}});
	mcToName.insert({db_acc, {"db -> acc", "db_acc"}});
	mcToName.insert({stop, {"stop", "stop"}});

	// xjx custom microcode
	mcToName.insert({ins_db, {"ins -> db", "ins_db"}});
	mcToName.insert({ins_mur1, {"ins -> mur (mc_addr)", "ins_mur1"}});
	mcToName.insert({ins_mur2, {"ins -> mur (mop, asm_ref)", "ins_mur2"}});
	mcToName.insert({mur_mc, {"mur -> mc", "mur_mc"}});

	for(auto it = mcToName.begin(); it != mcToName.end(); ++it) {
		nameToMC.insert({it->second.second, it->first});
	}
}

bool XJX::execMOP() {
	bool res = true;

	switch(microcode[mc_addr]) {
	case nop:			break;
	case db_ram:		ram[address_bus] = data_bus; break;
	case ram_db:		data_bus = ram[address_bus]; break;
	case db_ins:		ins = data_bus; break;
	case ins_ab:		address_bus = lo(ins); break;
	case ins_mc:		mc_addr = lookupAsm(hi(ins)) - 1; break;
	// u6 is UNUSED
	case mc_0:			mc_addr = -1; break;
	case pc_ab:			address_bus = program_counter; break;
	case pc_inc:		++program_counter; break;
	case if_0_pc_inc:	if(acc == 0) ++program_counter; break;
	case ins_pc:		program_counter = ins; break;
	case acc_0:			acc = 0; break;
	case plus:			acc += data_bus; accCheck(); break;
	case minus:			acc -= data_bus; accCheck(); break;
	case acc_db:		data_bus = acc; break;
	case acc_inc:		++acc; accCheck(); break;
	case acc_dec:		--acc; accCheck(); break;
	case db_acc:		acc = data_bus; accCheck(); break;
	case stop:			res = false; break;

	// xjx custom microcode
	case ins_db:		data_bus = lo(ins); break;
	case ins_mur1:		mur[0] = lo(ins); break;
	case ins_mur2:		mur[1] = lo(ins); mur[2] = hi(ins); break;
	case mur_mc:		murToMC(); break;

	default:			throw runtime_error("Unknown MicroOperation " + to_string(microcode[mc_addr]) +	" at address " + to_string(mc_addr));
	}

	++mc_addr;
	return res;
}

uint XJX::lookupAsm(uint val) {
	if(asm_reference.count(val) == 0) {
		return 0;
	} else {
		return asm_reference[val];
	}
}

uint XJX::hi(uint val) {
	return val / modulus;
}

uint XJX::lo(uint val) {
	return val % modulus;
}

void XJX::accCheck() {
	uint max = hi_max * modulus + lo_max;
	if(acc > max) {
		acc = max;
	}
}

void XJX::murToMC() {
	if(mur[1] != lo_max) microcode[mur[0]] = mur[1];

	if(mur[2] != 0) {
		if(asm_reference.count(mur[2]) == 0) {
			asm_reference.insert({mur[2], mur[0]});
		} else {
			asm_reference.at(mur[2]) = mur[0];
		}
	}

	mur[0] = 0;
	mur[1] = 0;
	mur[2] = 0;
}
