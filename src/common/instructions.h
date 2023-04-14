/*
  Copyright (C) 2023 James Haley
  
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
*/

#pragma once

namespace WCTCode
{
    // 3-bit encodable registers
    enum class LowRegister
    {
        R0,
        R1,
        R2,
        R3,
        R4,
        R5,
        R6,
        R7
    };

    // Instructions supported for patching (incl. data directives)
    enum class Instruction
    {
        ADDS,         // xx 1C/1D
        SUBS,         // xx 1E/1F
        MOVS_R0_IMM8, // xx 20
        MOVS_R1_IMM8, // xx 21
        MOVS_R2_IMM8, // xx 22
        CMP_R0_IMM8,  // xx 28
        LDR_R1_REL8,  // xx 49
        BNE_REL8,     // xx D1
        BGT_REL8,     // xx DC
        BLE_REL8,     // xx DD
        B_REL8,       // xx E0
        DCB,          // xx
        DCW,          // xx xx
        DCD,          // xx xx xx xx
        NUMINSTRUCTIONS
    };

    // Opcodes
    enum class Opcode : uint16_t
    {
        ADDS    = 0x1C00u,
        SUBS    = 0x1E00u,
        MOVS_R0 = 0x2000u,
        MOVS_R1 = 0x2100u,
        MOVS_R2 = 0x2200u,
        CMP_R0  = 0x2800u,
        LDR_R1  = 0x4900u,
        BNE_8   = 0xD100u,
        BGT_8   = 0xDC00u,
        BLE_8   = 0xDD00u,
        B_8     = 0xE000u
    };

    // Verify contents of a short word look like a specific Imm8 instruction
    static inline constexpr bool isImm8Op(uint16_t data, Opcode op)
    {
        return ((data & 0xFF00u) == uint16_t(op));
    }

    // Encode an Imm8 instruction with immediate data
    static inline constexpr uint16_t encodeImm8Op(Opcode op, uint8_t imm8)
    {
        return (uint16_t(op) | imm8);
    }

    // Verify contents of a short word look like an adds ALU op
    static inline constexpr bool isAddsOp(uint16_t data)
    {
        return ((data & 0xFF00u) >= 0x1C00u && (data & 0xFF00u) <= 0x1DFFu);
    }

    // Encode 3-bit parameters into an ALU adds op
    static inline constexpr uint16_t encodeAdds(LowRegister rd, LowRegister rs, uint8_t rn)
    {
        return (uint16_t(Opcode::ADDS) | (uint8_t(rd) & 7) | ((uint8_t(rs) << 3) & 7) | ((uint16_t(rn) << 6) & 7));
    }

    // Verify contents of a short word look like a subs ALU op
    static inline constexpr bool isSubsOp(uint16_t data)
    {
        return ((data & 0xFF00u) >= 0x1E00u && (data & 0xFF00u) <= 0x1FFFu);
    }

    // Encode 3-bit parameters into an ALU subs op
    static inline constexpr uint16_t encodeSubs(LowRegister rd, LowRegister rs, uint8_t rn)
    {
        return (uint16_t(Opcode::SUBS) | (uint8_t(rd) & 7) | ((uint8_t(rs) << 3) & 7) | ((uint16_t(rn) << 6) & 7));
    }

    // Instruction sizes
    static constexpr uint8_t instructionSizes[uint32_t(Instruction::NUMINSTRUCTIONS)] =
    {
        2, // ADDS
        2, // SUBS
        2, // MOVS_R0_IMM8
        2, // MOVS_R1_IMM8
        2, // MOVS_R2_IMM8
        2, // CMP_R0_IMM8
        2, // LDR_R1_REL8
        2, // BNE_REL8
        2, // BGT_REL8
        2, // BLE_REL8
        2, // B_REL8
        1, // DCB
        2, // DCW
        4  // DCD
    };

    // One-byte patch
    struct patch1_t
    {
        uint32_t    offset;      // offset in file
        Instruction instruction; // instruction type at offset
        uint8_t     original;    // expected value
        uint8_t     patched;     // value to patch in
    };

    // Two-byte patch
    struct patch2_t
    {
        uint32_t    offset;      // offset in file
        Instruction instruction; // instruction type at offset
        uint16_t    original;    // expected value
        uint16_t    patched;     // value to patch in
    };

    // Four-byte patch
    struct patch4_t
    {
        uint32_t    offset;      // offset in file
        Instruction instruction; // instruction type at offset
        uint32_t    original;    // expected value
        uint32_t    patched;     // value to patch in
    };

} // end namespace WCTCode

// EOF
