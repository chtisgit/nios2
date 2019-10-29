#include "nios2.h"

namespace nios2 {

#define NEXT                                                                                       \
	cmd = load<uint32_t>(pc), op = cmd & 0x3F;                                                 \
	goto *jtable[op]

void CPU::run(uint32_t entry)
{
	static void *jtable[] = {
	    &&_call,  &&_jmpi,     &&_reserved, &&_ldbu,     &&_addi,     &&_stb,      &&_br,
	    &&_ldb,   &&_cmpgei,   &&_reserved, &&_reserved, &&_ldhu,     &&_andi,     &&_sth,
	    &&_bge,   &&_ldh,      &&_cmplti,   &&_initda,   &&_ori,      &&_stw,      &&_blt,
	    &&_ldw,   &&_cmpnei,   &&_reserved, &&_reserved, &&_flushda,  &&_xori,     &&_reserved,
	    &&_bne,   &&_reserved, &&_cmpeqi,   &&_reserved, &&_reserved, &&_ldbuio,   &&_muli,
	    &&_stbio, &&_beq,      &&_ldbio,    &&_cmpgeui,  &&_reserved, &&_reserved, &&_ldhuio,
	    &&_andhi, &&_sthio,    &&_bgeu,     &&_ldhio,    &&_cmpltui,  &&_reserved, &&_custom,
	    &&_initd, &&_orhi,     &&_stwio,    &&_bltu,     &&_ldwio,    &&_rdprs,    &&_reserved,
	    &&_rtype, &&_flushd,   &&_xorhi,    &&_reserved, &&_reserved, &&_reserved};

	static void *rtable[] = {
	    &&_reserved, &&_eret,     &&_roli,     &&_rol,      &&_flushp,   &&_ret,
	    &&_nor,      &&_mulxuu,   &&_cmpge,    &&_bret,     &&_reserved, &&_ror,
	    &&_flushi,   &&_jmp,      &&_and,      &&_reserved, &&_cmplt,    &&_reserved,
	    &&_slli,     &&_sll,      &&_wrprs,    &&_reserved, &&_or,       &&_mulxsu,
	    &&_cmpne,    &&_reserved, &&_srli,     &&_srl,      &&_nextpc,   &&_callr,
	    &&_xor,      &&_mulxss,   &&_cmpeq,    &&_reserved, &&_reserved, &&_reserved,
	    &&_divu,     &&_div,      &&_rdctl,    &&_mul,      &&_cmpgeu,   &&_initi,
	    &&_reserved, &&_reserved, &&_reserved, &&_reserved, &&_trap,     &&_wrctl,
	    &&_reserved, &&_cmpltu,   &&_add,      &&_reserved, &&_reserved, &&_break,
	    &&_reserved, &&_sync,     &&_reserved, &&_reserved, &&_sub,      &&_srai,
	    &&_sra,      &&_reserved, &&_reserved, &&_reserved, &&_reserved};

    pc = entry;
    
	for (;;) {

		auto cmd = Memory[pc];
		auto op = cmd & 0x3F;
		RInstr rinstr;

		NEXT;
	_reserved:
		throw ReservedInstructionError{};

	_call : {
		JInstr instr;
		instr.decode(cmd);

		ra = pc + 4;
		pc = (pc & 0xF0000000) | instr.imm26;
	}
		NEXT;

	_jmpi : {
		JInstr instr;
		instr.decode(cmd);

		pc = (pc & 0xF0000000) | instr.imm26;
	}
		NEXT;

	_ldbu : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) = load<uint8_t>(reg(instr.a) + signExtend(instr.imm16, 16)) & 0xFF;
	}
		NEXT;

	_addi : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) = reg(instr.a) + signExtend(instr.imm16, 16);
	}
		NEXT;

	_stb : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		store(instr.a + signExtend(instr.imm16, 16), uint8_t(reg(instr.b) & 0xFF));
	}
		NEXT;

	_br : {
		IInstr instr;
		instr.decode(cmd);

		pc = pc + 4 + signExtend(instr.imm16, 16);
	}
		NEXT;

	_ldb : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) =
		    signExtend(load<uint8_t>(reg(instr.a) + signExtend(instr.imm16, 16)), 8);
	}
		NEXT;

	_cmpgei : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		uint32_t res = 0;
		if (toSigned(reg(instr.a)) >= toSigned(signExtend(instr.imm16, 16))) {
			res = 1;
		}

		reg(instr.b) = res;
	}
		NEXT;

	_ldhu : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) =
		    load<uint16_t>(reg(instr.a) + signExtend(instr.imm16, 16)) & 0xFFFF;
	}
		NEXT;

	_andi : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) = reg(instr.a) & uint32_t(instr.imm16);
	}
		NEXT;

	_sth : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		store(instr.a + signExtend(instr.imm16, 16), uint16_t(reg(instr.b) & 0xFFFF));
	}
		NEXT;

	_bge : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		uint32_t res = 0;
		if (toSigned(reg(instr.a)) >= toSigned(reg(instr.b))) {
			pc += signExtend(instr.imm16, 16);
		}
	}
		NEXT;

	_ldh : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) =
		    signExtend(load<uint16_t>(reg(instr.a) + signExtend(instr.imm16, 16)), 16);
	}
		NEXT;

	_cmplti : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		uint32_t res = 0;
		if (toSigned(reg(instr.a)) < toSigned(signExtend(instr.imm16, 16))) {
			res = 1;
		}

		reg(instr.b) = res;
	}
		NEXT;

	_initda:
		pc += 4;
		NEXT;

	_ori : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) = reg(instr.a) | uint32_t(instr.imm16);
	}
		NEXT;

	_stw : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		store(instr.a + signExtend(instr.imm16, 16), reg(instr.b));
	}
		NEXT;

	_blt : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		if (toSigned(reg(instr.a)) < toSigned(reg(instr.b))) {
			pc += signExtend(instr.imm16, 16);
		}
	}
		NEXT;

	_ldw : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) = load<uint32_t>(reg(instr.a) + signExtend(instr.imm16, 16));
	}
		NEXT;

	_cmpnei : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		uint32_t res = 0;
		if (reg(instr.a) != signExtend(instr.imm16, 16)) {
			res = 1;
		}

		reg(instr.b) = res;
	}
		NEXT;

	_flushda:
		pc += 4;
		NEXT;

	_xori : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) = reg(instr.a) ^ uint32_t(instr.imm16);
	}
		NEXT;

	_bne : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		if (reg(instr.a) != reg(instr.b)) {
			pc += signExtend(instr.imm16, 16);
		}
	}
		NEXT;

	_cmpeqi : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		uint32_t res = 0;
		if (reg(instr.a) == signExtend(instr.imm16, 16)) {
			res = 1;
		}

		reg(instr.b) = res;
	}
		NEXT;

	_ldbuio : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) = load<uint8_t>(reg(instr.a) + signExtend(instr.imm16, 16)) & 0xFF;
	}
		NEXT;

	_muli : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) = reg(instr.a) * signExtend(instr.imm16, 16);
	}
		NEXT;

	_stbio : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		store(instr.a + signExtend(instr.imm16, 16), uint8_t(reg(instr.b) & 0xFF));
	}
		NEXT;

	_beq : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		if (reg(instr.a) == reg(instr.b)) {
			pc += signExtend(instr.imm16, 16);
		}
	}
		NEXT;

	_ldbio : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) =
		    signExtend(load<uint8_t>(reg(instr.a) + signExtend(instr.imm16, 16)), 8);
	}
		NEXT;

	_cmpgeui : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		uint32_t res = 0;
		if (reg(instr.a) >= signExtend(instr.imm16, 16)) {
			res = 1;
		}

		reg(instr.b) = res;
	}
		NEXT;

	_ldhuio : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) =
		    load<uint16_t>(reg(instr.a) + signExtend(instr.imm16, 16)) & 0xFFFF;
	}
		NEXT;

	_andhi : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) = reg(instr.a) & uint32_t(instr.imm16 << 16);
	}
		NEXT;

	_sthio : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		store(instr.a + signExtend(instr.imm16, 16), uint16_t(reg(instr.b) & 0xFFFF));
	}
		NEXT;

	_bgeu : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		uint32_t res = 0;
		if (reg(instr.a) >= reg(instr.b)) {
			pc += signExtend(instr.imm16, 16);
		}
	}
		NEXT;

	_ldhio : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) =
		    signExtend(load<uint16_t>(reg(instr.a) + signExtend(instr.imm16, 16)), 16);
	}
		NEXT;

	_cmpltui : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		uint32_t res = 0;
		if (reg(instr.a) < signExtend(instr.imm16, 16)) {
			res = 1;
		}

		reg(instr.b) = res;
	}
		NEXT;

	_custom:
		pc += 4;
		NEXT;

	_initd:
		pc += 4;
		NEXT;

	_orhi : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) = reg(instr.a) | uint32_t(instr.imm16 << 16);
	}
		NEXT;

	_stwio : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		store(instr.a + signExtend(instr.imm16, 16), reg(instr.b));
	}
		NEXT;

	_bltu : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		if (reg(instr.a) < reg(instr.b)) {
			pc += signExtend(instr.imm16, 16);
		}
	}
		NEXT;

	_ldwio : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) = load<uint32_t>(reg(instr.a) + signExtend(instr.imm16, 16));
	}
		NEXT;

	_rdprs:
		throw IllegalInstructionError{};
		NEXT;

	_rtype:
		rinstr.decode(cmd);
		goto *rtable[rinstr.opx];

	_flushd:
		pc += 4;
		NEXT;

	_xorhi : {
		IInstr instr;
		instr.decode(cmd);
		pc += 4;

		reg(instr.b) = reg(instr.a) ^ uint32_t(instr.imm16 << 16);
	}
		NEXT;

		// R-Type OPX

	_eret:
		pc = ea;
		status = estatus;
		NEXT;

	_roli:
		throw NotImplementedError{};
		NEXT;

	_rol:
		throw NotImplementedError{};
		NEXT;

	_flushp:
		pc += 4;
		NEXT;

	_ret:
		pc = ra;
		NEXT;

	_nor:
		reg(rinstr.c) = ~(reg(rinstr.a) | reg(rinstr.b));
		NEXT;

	_mulxuu:
		throw NotImplementedError{};
		NEXT;

	_cmpge : {
		pc += 4;

		int res = 0;
		if (toSigned(reg(rinstr.a)) >= toSigned(reg(rinstr.b))) {
			res = 1;
		}

		reg(rinstr.c) = res;
	}
		NEXT;

	_bret : {
		status = bstatus;
		pc = ba;
	}
		NEXT;

	_ror:
		throw NotImplementedError{};
		NEXT;

	_flushi:
		throw NotImplementedError{};
		NEXT;

	_jmp:
		pc = reg(rinstr.a);
		NEXT;

	_and:
		reg(rinstr.c) = reg(rinstr.a) & reg(rinstr.b);
		pc += 4;
		NEXT;

	_cmplt : {
		pc += 4;

		int res = 0;
		if (toSigned(reg(rinstr.a)) < toSigned(reg(rinstr.b))) {
			res = 1;
		}

		reg(rinstr.c) = res;
	}
		NEXT;

	_slli:
		reg(rinstr.c) = reg(rinstr.a) << rinstr.b;
		pc += 4;
		NEXT;

	_sll:
		reg(rinstr.c) = reg(rinstr.a) << (reg(rinstr.b) & 0x1F);
		pc += 4;
		NEXT;

	_wrprs:
		throw NotImplementedError{};
		NEXT;

	_or:
		reg(rinstr.c) = reg(rinstr.a) | reg(rinstr.b);
		pc += 4;
		NEXT;

	_mulxsu:
		throw NotImplementedError{};
		NEXT;

	_cmpne : {
		pc += 4;

		int res = 0;
		if (reg(rinstr.a) != reg(rinstr.b)) {
			res = 1;
		}

		reg(rinstr.c) = res;
	}
		NEXT;

	_srli:
		reg(rinstr.c) = reg(rinstr.a) >> rinstr.b;
		pc += 4;
		NEXT;

	_srl:
		reg(rinstr.c) = reg(rinstr.a) >> (reg(rinstr.b) & 0x1F);
		pc += 4;
		NEXT;

	_nextpc:
		reg(rinstr.c) = pc += 4;
		NEXT;

	_callr:
		std::swap(pc, ra);
		ra += 4;
		NEXT;

	_xor:
		reg(rinstr.c) = reg(rinstr.a) ^ reg(rinstr.b);
		pc += 4;
		NEXT;

	_mulxss:
		throw NotImplementedError{};
		NEXT;

	_cmpeq : {
		pc += 4;

		int res = 0;
		if (reg(rinstr.a) == reg(rinstr.b)) {
			res = 1;
		}

		reg(rinstr.c) = res;
	}
		NEXT;

	_divu:
		reg(rinstr.c) = reg(rinstr.a) / reg(rinstr.b);
		pc += 4;
		NEXT;

	_div:
		reg(rinstr.c) = toSigned(reg(rinstr.a)) / toSigned(reg(rinstr.b));
		pc += 4;

		NEXT;

	_rdctl:
		throw NotImplementedError{};

		NEXT;

	_mul:
		reg(rinstr.c) = reg(rinstr.a) * reg(rinstr.b);
		pc += 4;

		NEXT;

	_cmpgeu : {
		pc += 4;

		int res = 0;
		if (reg(rinstr.a) >= reg(rinstr.b)) {
			res = 1;
		}

		reg(rinstr.c) = res;
	}
		NEXT;

	_initi:
		pc += 4;
		NEXT;

	_trap:
		estatus = status;
		ea = pc + 4;
		throw NotImplementedError{};

		NEXT;

	_wrctl:
		throw NotImplementedError{};
		NEXT;

	_cmpltu : {
		pc += 4;

		int res = 0;
		if (reg(rinstr.a) < reg(rinstr.b)) {
			res = 1;
		}

		reg(rinstr.c) = res;
	}
		NEXT;

	_add:
		reg(rinstr.c) = reg(rinstr.a) + reg(rinstr.b);
		pc += 4;
		NEXT;

	_break:
		throw NotImplementedError{};
		NEXT;

	_sync:
		throw NotImplementedError{};
		NEXT;

	_sub:
		reg(rinstr.c) = reg(rinstr.a) - reg(rinstr.b);
		pc += 4;
		NEXT;

	_srai:
		reg(rinstr.c) = signExtend(reg(rinstr.a) >> rinstr.b, 32 - rinstr.b);
		pc += 4;
		NEXT;

	_sra:{
        auto sh = (reg(rinstr.b) & 0x1F);
		reg(rinstr.c) = signExtend(reg(rinstr.a) >> sh, 32 - sh);
		pc += 4;
    }
		NEXT;
	}
}

} // namespace nios2
