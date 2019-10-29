#include <cstdint>
#include <vector>

namespace nios2 {

enum InstrType { R, I, J };

struct RInstr {
	static constexpr InstrType type = InstrType::R;
	int_least32_t opx;
	uint8_t op, a, b, c;

	void decode(uint32_t opcode)
	{
		op = opcode & 0x3F;
		opx = (opcode >> 6) & 0x7FF;
		c = (opcode >> 17) & 0x1F;
		b = (opcode >> 22) & 0x1F;
		a = (opcode >> 27) & 0x1F;
	}
};

struct IllegalInstructionError {};

struct ReservedInstructionError {};

struct NotImplementedError {
	int op, opx;
};

struct IInstr {
	static constexpr InstrType type = InstrType::I;
	uint_least16_t imm16;
	uint8_t op, a, b;

	void decode(uint32_t opcode)
	{
		op = opcode & 0x3F;
		imm16 = (opcode >> 6) & 0xFFFF;
		b = (opcode >> 22) & 0x1F;
		a = (opcode >> 27) & 0x1F;
	}
};

struct JInstr {
	static constexpr InstrType type = InstrType::J;
	uint_least32_t imm26;
	uint8_t op;

	void decode(uint32_t opcode)
	{
		op = opcode & 0x3F;
		imm26 = opcode >> 6;
	}
};

class CPU {
	uint32_t pc;
	uint32_t ra, ea, ba;
	uint32_t status, estatus, bstatus;
	uint32_t regfile[32] = {0}, zero;

	uint32_t& reg(int i) {
		if(i == 0){
			zero = 0;
			return zero;
		}

		return regfile[i];
	}

public:
	std::vector<uint8_t> Memory;

	void run(uint32_t entry = 0);

	constexpr uint32_t signExtend(uint32_t value, int bits) {
		uint32_t mask = (1 << bits) - 1;
		uint32_t se = (~0) & mask;
		if((value >> bits) & 1){
			value |= se;
		}

		return value;
	}

	constexpr int32_t toSigned(uint32_t value) {
		return int32_t(value);
	}

	template<typename T>
	T load(uint32_t addr)
	{
		// TODO memory-mapped IO
		uint32_t val = 0;
		for(int i = 0; i != sizeof(T); i++)
			val = (val << 8) | Memory[addr+i];
		return val;
	}

	template<typename T>
	void store(uint32_t addr, T val)
	{
		// TODO memory-mapped IO
		for(int i = 0; i != sizeof(T); i++){
			Memory[addr+i] = val & 0xFF;
			val >>= 8;
		}
	}

	void store(uint32_t addr, uint16_t val)
	{
		// TODO memory-mapped IO
		Memory[addr/4] = val;
	}

	void store(uint32_t addr, uint8_t val)
	{
		// TODO memory-mapped IO
		Memory[addr/4] = val;
	}

	void setMemSize(uint32_t sizeInBytes)
	{
		Memory.resize(sizeInBytes / sizeof(Memory[0]));
	}
};

} // namespace nios2