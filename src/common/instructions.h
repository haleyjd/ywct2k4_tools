/*
  Copyright (C) 2023 James Haley

  Most code here is derived from microjit by Thomas Denney
  https://github.com/thomasdenney/microjit

  For code in this module only:
  MIT License
  
  Copyright (c) 2018 Thomas Denney
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#pragma once

namespace WCTCode
{
    // Takes a twos-complemenet integer stored in the lower |bits| bits of |value|
    // and returns the 32-bit representation
    static inline constexpr int32_t unTwosComplement(uint32_t value, uint8_t bits)
    {
        return (value & (1u << (bits - 1))) ? int32_t(value - (1 << bits)) : int32_t(value);
    }

    // Retrieves the value of instruction[offset..(offset + length))
    static inline constexpr uint16_t uintRegion(unsigned instruction, uint8_t offset, uint8_t length)
    {
        return ((((1u << length) - 1) << offset) & instruction) >> offset;
    }
    
    // Represents |x| as a twos complement number in the lower |bits| bits of the result
    template<typename T, typename R = typename std::make_unsigned<T>::type>
    static inline constexpr R twosComplement(T x, uint8_t bits)
    {
        constexpr R MAXBIT = sizeof(T) * 8 - 1;
        return (R(x) & (1u << MAXBIT)) ? R(1u << (bits - 1)) | R(x + (1u << (bits - 1))) : R(x);
    }

    enum class LowRegister : uint8_t
    {
        r0,
        r1,
        r2,
        r3,
        r4,
        r5,
        r6,
        r7
    };

    enum class Register : uint8_t
    {
        r0,
        r1,
        r2,
        r3,
        r4,
        r5,
        r6,
        r7,
        r8,
        r9,
        r10,
        r11,
        r12,
        r13,
        r14,
        r15,

        // alternate names
        ip = r12,
        sp = r13,
        lr = r14,
        pc = r15
    };

    enum class ALUOp : uint8_t
    {
        adc  = 0b0101u,
        andB = 0b0000u,
        asr2 = 0b0100u,
        bic  = 0b1110u,
        cmn  = 0b1011u,
        cmp2 = 0b1010u,
        eor  = 0b0001u,
        lsl2 = 0b0010u,
        lsr2 = 0b0011u,
        mul  = 0b1101u,
        mvn  = 0b1111u,
        neg  = 0b1001u,
        orr  = 0b1100u,
        ror  = 0b0111u,
        sbc  = 0b0110u,
        tst  = 0b1000u
    };

    enum class Condition : uint8_t 
    {
        eq = 0b0000u, // Equal, Z set
        ne = 0b0001u, // Not equal, Z clear
        cs = 0b0010u,
        hs = 0b0010u, // Carry set/unsigned higher or same, C set
        cc = 0b0011u,
        lo = 0b0011u, // Carry clear/unsigned lower, C clear
        mi = 0b0100u, // Minus/negative, N set
        pl = 0b0101u, // Positive or zero, N clear
        vs = 0b0110u, // Overflow, V set
        vc = 0b0111u, // No overflow, V clear
        hi = 0b1000u, // Unsigned higher, C set and Z clear
        ls = 0b1001u, // Unsigned lower or same, C clear or Z set
        ge = 0b1010u, // Signed greater than or equal Nset and V set, or N clear and V clear
        lt = 0b1011u, // Signed less than, N set and V clear, or N clear and V set
        gt = 0b1100u, // Signed greater than, Z clear, and either N set and V set, or N clear and V clear
        le = 0b1101u  // Signed less than or equal, Z set, or N set and V clear, or N clear and V set
    };

    enum class RegisterList : uint8_t 
    {
        empty = 0b00000000u,
        r0    = 0b00000001u,
        r1    = 0b00000010u,
        r2    = 0b00000100u,
        r3    = 0b00001000u,
        r4    = 0b00010000u,
        r5    = 0b00100000u,
        r6    = 0b01000000u,
        r7    = 0b10000000u
    };
    static inline constexpr RegisterList operator | (RegisterList rl1, RegisterList rl2)
    {
        return RegisterList(uint8_t(rl1) | uint8_t(rl2));
    }

    using Instruction = uint16_t;
    struct InstructionPair
    {
        Instruction i[2];
    };

    // Ensures that |value| only contains the |maxBits| least significant bits,
    // and then shifts the value by |offset|
    static inline constexpr Instruction shift(uint16_t value, uint16_t offset, uint8_t maxBits)
    {
        return (value & ((1u << maxBits) - 1)) << offset;
    }

    template<typename T>
    static inline constexpr Instruction shiftReg(T value, uint16_t offset, uint8_t maxBits)
    {
        return shift(uint16_t(value), offset, maxBits);
    }

    static inline constexpr uint8_t highBit(Register r)
    {
        return (uint8_t(r) & 0b1000u) >> 3;
    }

    // All arithmetic/logical operations are generally of the form
    // |rd| = |rd| OP |rm|
    // OP sometimes ignores rd
    // Both |rd| and |rm| must be low registers
    static inline constexpr Instruction arithmeticOperation(LowRegister rd, LowRegister rm, ALUOp opcode)
    {
        return shift(0b010000u, 10, 6) | shift(uint16_t(opcode), 6, 4) | shiftReg(rm, 3, 3) | shiftReg(rd, 0, 3);
    }

    // ADC A7-4
    // Adds two values and the carry flag
    // |rd| and |rm| must be low registers
    static inline constexpr Instruction addWithCarry(LowRegister rd, LowRegister rm)
    {
        return arithmeticOperation(rd, rm, ALUOp::adc);
    }

    // ADD (1) A7-5
    // |rd| = |rn| + |imm|
    // 0 <= |imm| < 8, |rd| and |rn| must be low registers
    static inline constexpr Instruction addSmallImm(LowRegister rd, LowRegister rn, uint8_t imm)
    {
        return shift(0b0001110u, 9, 7) | shift(imm, 6, 3) | shiftReg(rn, 3, 3) | shiftReg(rd, 0, 3);
    }

    // ADD (2) A7-6
    // Add an eight bit unsigned immediate value
    // |rd| must be a low register
    static inline constexpr Instruction addLargeImm(LowRegister rd, uint8_t imm)
    {
        return shift(0b00110u, 11, 5) | shiftReg(rd, 8, 3) | shift(imm, 0, 8);
    }

    // ADD (3) A7-7
    // |rd| = |rn| + |rm|
    // All registers must be low registers
    static inline constexpr Instruction addReg(LowRegister rd, LowRegister rn, LowRegister rm)
    {
        return shift(0b0001100u, 9, 7) | shiftReg(rm, 6, 3) | shiftReg(rn, 3, 3) | shiftReg(rd, 0, 3);
    }

    // ADD (4) A7-8
    // |rd| = |rd| + |rm|
    // At least one of the registes should be a high register!
    static inline constexpr Instruction addGeneral(Register rd, Register rm)
    {
        return shift(0b01000100u, 8, 9)
            |  shift(highBit(rd), 7, 1)
            |  shift(highBit(rm), 6, 1)
            |  shift(uint8_t(rm) & 0b111u, 3, 3)
            |  shift(uint8_t(rd) & 0b111u, 0, 3);
    }

    // ADD (5) A7-10
    // |rd| = PC + |imm| * 4
    // |rd| must be a a low register
    static inline constexpr Instruction addPCRelativeAddress(LowRegister rd, uint8_t imm)
    {
        return shift(0b10100u, 11, 5) | shiftReg(rd, 8, 3) | shift(imm, 0, 8);
    }
    
    // ADD (6) A7-11
    // |rd| = SP + |imm| * 4
    // |rd| must be a a low register
    static inline constexpr Instruction addSPRelativeAddress(LowRegister rd, uint8_t imm)
    {
        return shift(0b10101u, 11, 5) | shiftReg(rd, 8, 3) | shift(imm, 0, 8);
    }
    
    // ADD (7) A7-12
    // SP = SP + |imm7| * 4
    static inline constexpr Instruction addSP(uint8_t imm7)
    {
        return shift(0b101100000u, 7, 9) | shift(imm7, 0, 7);
    }
    
    // AND A7-14
    // |rd| = |rd| & |rm|
    // |rd| and |rm| must be low registers
    static inline constexpr Instruction andBitwise(LowRegister rd, LowRegister rm)
    {
        return arithmeticOperation(rd, rm, ALUOp::andB);
    }
    
    // ASR (1) A7-15
    // |rd| = |rm| >> imm
    // |rm| and |rd| must be low registers
    static inline constexpr Instruction arithmeticShiftRightImm(LowRegister rd, LowRegister rm, uint8_t imm)
    {
        return shift(0b00010u, 11, 5) | shift(imm, 6, 5) | shiftReg(rm, 3, 3) | shiftReg(rd, 0, 3);
    }
    
    // ASR (2) A7-17
    // |rd| = |rd| >> |rs|
    // |rd| and |rs| must be low registers
    static inline constexpr Instruction arithmeticShiftRightRegister(LowRegister rd, LowRegister rs)
    {
        return arithmeticOperation(rd, rs, ALUOp::asr2);
    }
    
    // B (1) A7-19
    // Conditional branch by signed |imm| if |c| holds
    // @see unconditionalBranch for branch jumping information
    static inline constexpr Instruction conditionalBranch(Condition c, int8_t imm)
    {
        return shift(0b1101u, 12, 4) | shift(uint8_t(c), 8, 4) | shift(twosComplement(imm, 8), 0, 8);
    }
    
    // Alternative version of B (1)
    static inline constexpr Instruction conditionalBranchNatural(Condition c, int8_t imm)
    {
        return conditionalBranch(c, imm - 2);
    }
    
    // B (2) A7-21
    // Unconditional branch by |imm| (signed)
    //
    // The value of |imm| is an offset from the address of the branch instruction +
    // 4, all divided by two. This is effectively the same as computing the offset
    // from not the instruction that immediately follows the branch, but the one
    // after that. So if you want to branch to the next instruction, do -1, two
    // instructions, do 0, the branch instruction itself, do -2
    static inline constexpr Instruction unconditionalBranch(int16_t imm)
    {
        return shift(0b11100u, 11, 5) | shift(twosComplement(imm, 11), 0, 11);
    }
    
    // Alternative implementation of B (2) where |imm| is the offset, by instruction
    // count, from the current instruction
    static inline constexpr Instruction unconditionalBranchNatural(int16_t imm)
    {
        return unconditionalBranch(imm - 2);
    }
    
    // BIC A7-23
    // |rd| = |rd| & !|rm|
    // |rd| and |rs| must be low registers
    static inline constexpr Instruction bitClear(LowRegister rd, LowRegister rm)
    {
        return arithmeticOperation(rd, rm, ALUOp::bic);
    }
    
    // BL (1) A7-26
    // Used for calling another Thumb subroutine
    // Offset is from instruction address + 4, as per convention. See other branch
    // instructions.
    // Returns a pair of instructions for branching and linking by |offset|
    // instructions
    static inline constexpr InstructionPair branchAndLink(int32_t offset)
    {
        const uint32_t twentyTwoBits = twosComplement(offset, 22);
        return InstructionPair {
            uint16_t(shift(0b111u, 13, 3) | shift(0b10u, 11, 2) | shift(uintRegion(twentyTwoBits, 11, 11), 0, 11)),
            uint16_t(shift(0b111u, 13, 3) | shift(0b11u, 11, 2) | shift(uintRegion(twentyTwoBits,  0, 11), 0, 11))
        };
    }
    
    static inline constexpr InstructionPair branchAndLinkNatural(int32_t offset)
    {
        return branchAndLink(offset - 2);
    }
    
    // BLX (1) A7-26
    // Used for calling another ARM subroutine
    // See BL (1)
    static inline constexpr InstructionPair branchLinkAndExchange(int32_t offset)
    {
        const uint32_t twentyTwoBits = twosComplement(offset, 22);
        return InstructionPair {
            uint16_t(shift(0b111u, 13, 3) | shift(0b10u, 11, 2) | shift(uintRegion(twentyTwoBits, 11, 11), 0, 11)),
            uint16_t(shift(0b111u, 13, 3) | shift(0b01u, 11, 2) | shift(uintRegion(twentyTwoBits,  0, 11), 0, 11))
        };
    }
    
    // BLX (2) A7-30
    // Branches to the address stored in |rm|
    static inline constexpr Instruction branchLinkExchangeToRegister(Register rm)
    {
        return shift(0b010001111u, 7, 9) | shiftReg(rm, 3, 4) | shift(0, 0, 3);
    }
    
    // BX A7-32
    // Branches between ARM code and Thumb code
    static inline constexpr Instruction branchAndExchange(Register rm)
    {
        return shift(0b010001110u, 7, 9) | shiftReg(rm, 3, 4);
    }
    
    // CMP (1) A7-35
    // Comparison between a low register |rn| and an immediate 8-bit value |imm|
    static inline constexpr Instruction compareImmediate(LowRegister rn, uint8_t imm)
    {
        return shift(0b00101u, 11, 5) | shiftReg(rn, 8, 3) | shift(imm, 0, 8);
    }
    
    // CMP (2) A7-36
    // Comparison between two low registers
    static inline constexpr Instruction compareLowRegisters(LowRegister rn, LowRegister rm)
    {
        return arithmeticOperation(rn, rm, ALUOp::cmp2);
    }
    
    // CMP (3) A7-37
    // Comparison between two registers
    static inline constexpr Instruction compareRegistersGeneral(Register rn, Register rm)
    {
        return shift(0b01000101u, 8, 8)
            |  shift(highBit(rn), 7, 1)
            |  shift(highBit(rm), 6, 1)
            |  shift(uint8_t(rm) & 0b111u, 3, 3)
            |  shift(uint8_t(rn) & 0b111u, 0, 3);
    }
    
    // EOR A7-43
    // |rd| = |rd| EOR |rm|
    // |rd| and |rm| must be low registers
    static inline constexpr Instruction eor(LowRegister rd, LowRegister rm)
    {
        return arithmeticOperation(rd, rm, ALUOp::eor);
    }
    
    // LDMIA A7-44
    // Loads values from subsequent addresses into the registers in |regs| starting
    // from the address in |rn| and then increments |rn| by the number of addresses
    // loaded (* 4) if |rn| is not in |regs|. Can be used in conjunction with STMIA
    // for efficient block copy.
    // |rn| must be a low register
    static inline constexpr Instruction loadMultipleIncrementAfter(LowRegister rn, RegisterList regs)
    {
        return shift(0b11001u, 11, 5) | shiftReg(rn, 8, 3) | shift(uint16_t(regs), 0, 8);
    }

    static inline constexpr Instruction loadOrStoreWithOffset(uint8_t op, LowRegister rd, LowRegister rn, uint8_t offset)
    {
        return shift(op, 11, 5) | shift(offset, 6, 5) | shiftReg(rn, 3, 3) | shiftReg(rd, 0, 3);
    }

    static inline constexpr Instruction loadOrStoreWithRegisterOffset(uint8_t op, LowRegister rd, LowRegister rn, LowRegister rm)
    {
        return shift(op, 9, 7) | shiftReg(rm, 6, 3) | shiftReg(rn, 3, 3) | shiftReg(rd, 0, 3);
    }
    
    // LDR (1) A7-47
    // Loads 32-bit words from memory
    static inline constexpr Instruction loadWordWithOffset(LowRegister rd, LowRegister rn, uint8_t offset)
    {
        return loadOrStoreWithOffset(0b01101u, rd, rn, offset);
    }
    
    // LDR (2) A7-49
    static inline constexpr Instruction loadWordWithRegisterOffset(LowRegister rd, LowRegister rn, LowRegister rm)
    {
        return loadOrStoreWithRegisterOffset(0b0101100u, rd, rn, rm);
    }
    
    // LDR (3) A7-51
    // Loads PC-relative data
    static inline constexpr Instruction loadWordWithPCOffset(Register rd, uint8_t offset)
    {
        return shift(0b01001u, 11, 5) | shiftReg(rd, 8, 3) | shift(offset, 0, 8);
    }
    
    // LDR (4) A7-53
    // SP relative data
    static inline constexpr Instruction loadWordWithStackPointerOffset(Register rd, uint8_t offset)
    {
        return shift(0b10011u, 11, 5) | shiftReg(rd, 8, 3) | shift(offset, 0, 8);
    }
    
    // LDRB (1) A7-55
    // Loads a byte and zero-extends it; useful for accessing record files
    static inline constexpr Instruction loadByteWithOffset(LowRegister rd, LowRegister rn, uint8_t offset)
    {
        return loadOrStoreWithOffset(0b01111u, rd, rn, offset);
    }
    
    // LDRB (2) A7-56
    static inline constexpr Instruction loadByteWithRegisterOffset(LowRegister rd, LowRegister rn, LowRegister rm)
    {
        return loadOrStoreWithRegisterOffset(0b0101110u, rd, rn, rm);
    }
    
    // LDRH (1) A7-57
    // Loads a half word and zero-extends it; useful for accessing record files
    static inline constexpr Instruction loadHalfWordWithOffset(LowRegister rd, LowRegister rn, uint8_t offset)
    {
        return loadOrStoreWithOffset(0b10001u, rd, rn, offset);
    }
    
    // LDRH (2) A7-59
    static inline constexpr Instruction loadHalfWordWithRegisterOffset(LowRegister rd, LowRegister rn, LowRegister rm)
    {
        return loadOrStoreWithRegisterOffset(0b0101101u, rd, rn, rm);
    }
    
    // LDRSB A7-61
    static inline constexpr Instruction loadSignedByteWithRegisterOffset(LowRegister rd, LowRegister rn, LowRegister rm)
    {
        return loadOrStoreWithRegisterOffset(0b0101011u, rd, rn, rm);
    }
    
    // LDRSH A7-62
    static inline constexpr Instruction loadSignedHalfWordWithRegisterOffset(LowRegister rd, LowRegister rn, LowRegister rm)
    {
        return loadOrStoreWithRegisterOffset(0b0101111u, rd, rn, rm);
    }
    
    // LSL (1) A7-64
    // |rm| and |rd| must be low registers
    // 0 <= |imm| < 32
    static inline constexpr Instruction logicalShiftLeftImmediate(LowRegister rd, LowRegister rm, uint8_t imm)
    {
        return shift(0b00000, 11, 5) | shift(imm, 6, 5) | shiftReg(rm, 3, 3) | shiftReg(rd, 0, 3);
    }
    
    // LSL (2) A7-66
    // |rd| = |rd| << |rs|
    // |rd| and |rs| must be low registers
    static inline constexpr Instruction leftShiftLogicalRegister(LowRegister rd, LowRegister rs)
    {
        return arithmeticOperation(rd, rs, ALUOp::lsl2);
    }
    
    // LSR (2) A7-70
    // |rd| = |rd| >> |rs|
    // |rd| and |rs| must be low registers
    static inline constexpr Instruction rightShiftLogicalRegister(LowRegister rd, LowRegister rs)
    {
        return arithmeticOperation(rd, rs, ALUOp::lsr2);
    }
    
    // MOV (1) A7-72
    // Moves an immediate unsigned 8-bit value to |rd|
    static inline constexpr Instruction moveImmediate(LowRegister rd, uint8_t x)
    {
        return shift(0b00100u, 11, 5) | shiftReg(rd, 8, 3) | shift(uint16_t(x), 0, 8);
    }
    
    // MOV (2) A7-73
    // |rd| = |rn|
    static inline constexpr Instruction moveLowToLow(LowRegister rd, LowRegister rn)
    {
        return shift(0b0001110u, 9, 7) | shift(0b000, 6, 3) | shiftReg(rn, 3, 3) | shiftReg(rd, 0, 3);
    }
    
    // MOV (3) A7-75
    // Moves from any register to any other register
    // Doesn't change the flags, unlike MOV (2)
    static inline constexpr Instruction moveGeneral(Register rd, Register rm)
    {
        return shift(0b01000110u, 8, 8) 
            |  shift(highBit(rd), 7, 1) 
            |  shift(highBit(rm), 6, 1)
            |  shift(uint16_t(rm) & 0b111u, 3, 3)
            |  shift(uint16_t(rd) & 0b111u, 0, 3);
    }
    
    // MUL A7-77
    // |rd| = |rd| * |rm|
    // |rd| and |rm| must be low registers
    static inline constexpr Instruction mul(LowRegister rd, LowRegister rm)
    {
        return arithmeticOperation(rd, rm, ALUOp::mul);
    }
    
    // MVN A7-79
    // |rd| = !|rm|
    // |rd| and |rm| must be low registers
    static inline constexpr Instruction moveNot(LowRegister rd, LowRegister rm)
    {
        return arithmeticOperation(rd, rm, ALUOp::mvn);
    }
    
    // NEG A7-80
    // |rd| = 0 - |rm|
    // |rd| and |rm| must be low registers
    static inline constexpr Instruction neg(LowRegister rd, LowRegister rm)
    {
        return arithmeticOperation(rd, rm, ALUOp::neg);
    }
    
    // ORR A7-81
    // |rd| = |rd| | |rm|
    // |rd| and |rm| must be low registers
    static inline constexpr Instruction orBitwise(LowRegister rd, LowRegister rm)
    {
        return arithmeticOperation(rd, rm, ALUOp::orr);
    }
    
    // POP A7-82
    // Pops multiple registers in r0 to r7 or PC from the stack - i.e. if PC is
    // popped then a branch occurs
    // Written as POP in the THUMB and LDMIA in ARM
    static inline constexpr Instruction popMultiple(bool pc, RegisterList regs)
    {
        return shift(0b1011110u, 9, 7) 
            |  shift(pc ? 1 : 0, 8, 1)      // Important that this is exactly one bit
            |  shift(uint16_t(regs), 0, 8);
    }
    
    // PUSH A7-85
    // Pushes multiple registers in r0 to r7 or PC to the stack
    // Note that this is written in ARM as STMDB, and Thumb as PUSH
    static inline constexpr Instruction pushMultiple(bool lr, RegisterList regs)
    {
        return shift(0b1011010u, 9, 7)
            |  shift(lr ? 1 : 0, 8, 1)
            |  shift(uint16_t(regs), 0, 8);
    }

    // ROR A7-92
    // |rd| = |rd| rotated right by |rm|
    // |rd| and |rm| must be low registers
    static inline constexpr Instruction ror(LowRegister rd, LowRegister rs)
    {
        return arithmeticOperation(rd, rs, ALUOp::ror);
    }
    
    // SBC A7-94
    // |rd| = |rd| - |rm| - NOT(C FLAG)
    // |rd| and |rm| must be low registers
    static inline constexpr Instruction subtractWithCarry(LowRegister rd, LowRegister rm)
    {
        return arithmeticOperation(rd, rm, ALUOp::sbc);
    }
    
    // STR (1) A7-99
    // Stores 32-bit data from a general-purpose register to memory
    static inline constexpr Instruction storeWordWithOffset(LowRegister rd, LowRegister rn, uint8_t imm)
    {
        return loadOrStoreWithOffset(0b01100u, rd, rn, imm);
    }
    
    // STR (2) A7-101
    static inline constexpr Instruction storeWordWithRegisterOffset(LowRegister rd, LowRegister rn, LowRegister rm)
    {
        return loadOrStoreWithRegisterOffset(0b0101000u, rd, rn, rm);
    }
    
    // STR (3) A7-103
    static inline constexpr Instruction storeWordWithStackPointerOffset(LowRegister rd, uint8_t imm)
    {
        return shift(0b10010u, 11, 5) | shiftReg(rd, 8, 3) | shift(imm, 0, 8);
    }
    
    // STRB (1) A7-105
    static inline constexpr Instruction storeByteWithOffset(LowRegister rd, LowRegister rn, uint8_t imm)
    {
        return loadOrStoreWithOffset(0b01110u, rd, rn, imm);
    }
    
    // STRB (2) A7-107
    static inline constexpr Instruction storeByteWithRegisterOffset(LowRegister rd, LowRegister rn, LowRegister rm)
    {
        return loadOrStoreWithRegisterOffset(0b0101010u, rd, rn, rm);
    }
    
    // STRH (1) A7-109
    static inline constexpr Instruction storeHalfWordWithOffset(LowRegister rd, LowRegister rn, uint8_t imm)
    {
        return loadOrStoreWithOffset(0b10000u, rd, rn, imm);
    }
    
    // STRH (2) A7-111
    static inline constexpr Instruction storeHalfWordWithRegisterOffset(LowRegister rd, LowRegister rn, LowRegister rm)
    {
        return loadOrStoreWithRegisterOffset(0b0101001u, rd, rn, rm);
    }
    
    // STMIA A7-96
    // Stores a set of registers |regs| from the address starting in |rn|
    // |rn| is a low register and it will be incremented to four times the number of
    // registers in |regs|
    static inline constexpr Instruction storeMultipleIncrementAfter(Register rn, RegisterList regs)
    {
        return shift(0b11000u, 11, 5) | shiftReg(rn, 8, 3) | shift(uint16_t(regs), 0, 8);
    }
    
    // SUB (1) A7-113
    // |rd| = |rn| - |imm|
    // 0 <= |imm| < 8, |rd| and |rn| must be low registers
    static inline constexpr Instruction subSmallImm(LowRegister rd, LowRegister rn, uint8_t imm)
    {
        return shift(0b0001111u, 9, 7) | shift(imm, 6, 3) | shiftReg(rn, 3, 3) | shiftReg(rd, 0, 3);
    }
    
    // SUB (2) A7-114
    // Subtract an eight bit unsigned immediate value
    // |rd| must be a low register
    static inline constexpr Instruction subLargeImm(LowRegister rd, uint8_t imm)
    {
        return shift(0b00111u, 11, 5) | shiftReg(rd, 8, 3) | shift(imm, 0, 8);
    }
    
    // SUB (3) A7-115
    // |rd| = |rn| - |rm|
    // All registers must be low registers
    static inline constexpr Instruction subReg(LowRegister rd, LowRegister rn, LowRegister rm)
    {
        return shift(0b0001101u, 9, 7) | shiftReg(rm, 6, 3) | shiftReg(rn, 3, 3) | shiftReg(rd, 0, 3);
    }
    
    // SUB (4) A7-116
    // Decrements the SP by four times a 7-bit immediate
    static inline constexpr Instruction subSP(uint8_t imm)
    {
        return shift(0b101100001u, 7, 9) | shift(imm, 0, 7);
    }
    
    // SXTB A7-120
    // |rd| = SignExtend(rm[7:0])
    // Extracts the least significant eight bits of |rm| and sign-extends them to 32 bits
    // |rd| and |rm| must be low registers
    static inline constexpr Instruction signExtendByte(LowRegister rd, LowRegister rm)
    {
        return shift(0b1011001001u, 6, 10) | shiftReg(rm, 3, 3) | shiftReg(rd, 0, 3);
    }
    
    // SXTH A7-121
    // |rd| = SignExtend(rm[15:0])
    // Extracts the least significant sixteen bits of |rm| and sign-extends them to 32 bits
    // |rd| and |rm| must be low registers
    static inline constexpr Instruction signExtendHalfWord(LowRegister rd, LowRegister rm)
    {
        return shift(0b1011001000u, 6, 10) | shiftReg(rm, 3, 3) | shiftReg(rd, 0, 3);
    }
    
    // Pseudo-instructions
    
    // Equivalent to mov r8, r8
    static inline constexpr Instruction nop()
    {
        return moveGeneral(Register::r8, Register::r8);
    }
    
    // Returns from a procedure
    // Equivalent to blx lr
    static inline constexpr Instruction ret()
    {
        return branchAndExchange(Register::lr);
    }

    // Verify contents of a dword look like an unconditional branch op
    static inline constexpr bool isBranchRel24(uint32_t data)
    {
        return (data & 0xFF000000u) == 0xEA000000u;
    }

    // sign-extender for signed 24-bit immediates
    struct ext24_t
    {
        signed int d : 24;
    };

    // Get offset from a 24-bit relative branch
    static inline int32_t getBranchRel24Dest(uint32_t data)
    {
        ext24_t extender;
        extender.d = (data & 0x00FFFFFFu);
        return int32_t(extender.d);
    }
} // end namespace WCTCode

// EOF
