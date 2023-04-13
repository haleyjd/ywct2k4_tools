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
    // Instructions supported for patching (incl. data directives)
    enum class Instructions
    {
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
        return ((data & 0xff00u) == uint16_t(op));
    }

    // Encode an Imm8 instruction with immediate data
    static inline constexpr uint16_t encodeImm8Op(Opcode op, uint8_t imm8)
    {
        return (uint16_t(op) | imm8);
    }

    // Instruction sizes
    static constexpr uint8_t instructionSizes[uint32_t(Instructions::NUMINSTRUCTIONS)] =
    {
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

} // end namespace WCTCode

// EOF
