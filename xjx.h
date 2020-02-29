#ifndef CPU_H
#define CPU_H

#include <vector>
#include <map>

namespace XJX {
enum MicroOperation {
	nop			= 0,
	db_ram		= 1, // all
	ram_db		= 2, // all
	db_ins		= 3, // all
	ins_ab		= 4, // lo
	ins_mc		= 5, // hi
	UNUSED		= 6,
	mc_0		= 7,
	pc_ab		= 8, // lo
	pc_inc		= 9,
	if_0_pc_inc	= 10,
	ins_pc		= 11, // lo
	acc_0		= 12,
	plus		= 13, // all
	minus		= 14, // all
	acc_db		= 15, // all
	acc_inc		= 16,
	acc_dec		= 17,
	db_acc		= 18, // all
	stop		= 19,

	// xjx custom microcode
	ins_db		= 20, // lo
	ins_mur1	= 21, // lo -> mc_addr
	ins_mur2	= 22, // lo -> mop, hi -> asm_reference
	mur_mc		= 23, // writes mur to mc
	iomap       = 24,
};

// Buses
extern uint address_bus;
extern uint data_bus;

// RAM
extern std::vector<uint> ram;
extern uint hi_max;
extern uint lo_max;
extern uint modulus;

// CPU
extern uint ins; // general-purpose register
extern uint program_counter;
extern std::vector<uint> microcode;
extern uint mc_addr;
extern uint mc_addr_max;
extern std::map<uint, uint> asm_reference; // maps assembler to microcode address
extern uint mur[3]; // microcode update register, stores mc_addr, mop, asm_reference

// ALU
extern uint acc;

// Name lookup
extern std::map<uint, std::pair<std::string, std::string>> mcToName;
extern std::map<std::string, uint> nameToMC;

// IO
struct IOMapping {
	uint min_addr;
	uint max_addr;
	int offset;
	int in;
	int out;
};
extern std::map<uint, std::pair<const char*, char**>> io_entries;
extern std::vector<IOMapping> io_mappings;

/**
 * @brief init Updates all static data
 */
void init();

/**
 * @brief execMOP Executes the next MicroOperation
 * @return Returns false if it was stop
 */
bool execMOP();

/**
 * @brief lookupAsm Converts assembler to MicroCode address
 * @return The address
 */
uint lookupAsm(uint val);

/**
 * @brief hi Returns the high part of a value
 * @param val The value in question
 */
uint hi(uint val);

/**
 * @brief lo Returns the high part of a value
 * @param val The value in question
 */
uint lo(uint val);

/**
 * @brief accCheck Checks the accumulator for over/underflow
 */
void accCheck();

/**
 * @brief updateAsmRef Inserts or updates an ASM -> MicroCode address reference from the MUR
 */
void murToMC();

/**
 * @brief ioMap Loads a module and maps its IO memory region to us.
 */
void ioMap();

ssize_t safeRead(int fd, std::string& data);
ssize_t safeWrite(int fd, const std::string& data);
}

#endif // CPU_H
