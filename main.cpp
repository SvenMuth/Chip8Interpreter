#include <iostream>
#include <print>
#include <fstream>

#include "main.h"

#include <random>


Chip8::Chip8()
{
    constexpr std::array<uint8_t, 80> font{
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80, // F
    };

    // Write font to memory from 0x50 to 0x9F
    for (int i = 0x50; const auto& f : font)
    {
        m_memory.at(i) = f;
        i++;
    }
}

auto Chip8::read_rom(const std::filesystem::path& file_path) -> void
{
    std::ifstream file(file_path, std::ios::binary | std::ios::in);

    if (!file.good())
    {
        throw std::runtime_error("Failed to open ROM!");
    }

    char c{};

    // Standard starting position in memory for ROM data
    int memory_pos{0x200};
    while (file.get(c))
    {
        if (memory_pos >= m_memory.size())
        {
            throw std::runtime_error("ROM size is to big for memory!");
        }
        m_memory.at(memory_pos) = static_cast<uint8_t>(c);
        memory_pos++;
    }
}

auto Chip8::main_loop() -> void
{
    while (true)
    {
        const auto opcode = fetch();

        const auto nibbles = get_nibbles(opcode);
        const auto instruction = decode(opcode, nibbles);
        execute(instruction, nibbles);
    }
}

auto Chip8::fetch() const -> std::uint16_t
{
    const auto opcode_first_byte = m_memory.at(m_program_counter);
    const auto opcode_second_byte = m_memory.at(m_program_counter + 1);

    std::uint16_t opcode = 0x0;
    opcode = opcode | opcode_first_byte << 8;
    opcode = opcode | opcode_second_byte << 0;

    return opcode;
}

auto Chip8::decode(const std::uint16_t opcode, const Nibbles nibbles) -> Instruction
{
    auto& [first_nibble, second_nibble,
        third_nibble, fourth_nibble] = nibbles;

    switch (first_nibble)
    {
    case 0x0:
        return get_instruction_0XXX(third_nibble, fourth_nibble);
        break;
    case 0x1:
        return Instruction::I_1NNN;
        break;
    case 0x2:
        return Instruction::I_2NNN;
        break;
    case 0x3:
        return Instruction::I_3XNN;
        break;
    case 0x4:
        return Instruction::I_4XNN;
        break;
    case 0x5:
        return Instruction::I_5XY0;
        break;
    case 0x6:
        return Instruction::I_6XNN;
        break;
    case 0x7:
        return Instruction::I_7XNN;
        break;
    case 0x8:
        return get_instruction_8XXX(fourth_nibble);
        break;
    case 0x9:
        return Instruction::I_9XY0;
        break;
    case 0xA:
        return Instruction::I_ANNN;
        break;
    case 0xB:
        return Instruction::I_BNNN;
        break;
    case 0xC:
        return Instruction::I_CXNN;
        break;
    case 0xD:
        return Instruction::I_DXYN;
        break;
    case 0xE:
        return get_instruction_EXXX(third_nibble, fourth_nibble);
        break;
    case 0xF:
        return get_instruction_FXXX(third_nibble, fourth_nibble);
        break;
    default:
        throw std::invalid_argument("Invalid opcode!");
        break;
    }

    return Instruction::UNINITIALIZED;
}

auto Chip8::get_instruction_0XXX(const std::uint8_t third_nibble, const std::uint8_t fourth_nibble) -> Instruction
{
    if (third_nibble == 0xE and fourth_nibble == 0x0)
    {
        return Instruction::I_00E0;
    }
    if (third_nibble == 0xE and fourth_nibble == 0xE)
    {
        return Instruction::I_00EE;
    }

    return Instruction::UNINITIALIZED;
}

auto Chip8::get_instruction_8XXX(const std::uint8_t fourth_nibble) -> Instruction
{
    if (fourth_nibble == 0x0)
    {
        return Instruction::I_8XY0;
    }
    if (fourth_nibble == 0x1)
    {
        return Instruction::I_8XY1;
    }
    if (fourth_nibble == 0x2)
    {
        return Instruction::I_8XY2;
    }
    if (fourth_nibble == 0x3)
    {
        return Instruction::I_8XY3;
    }
    if (fourth_nibble == 0x4)
    {
        return Instruction::I_8XY4;
    }
    if (fourth_nibble == 0x5)
    {
        return Instruction::I_8XY5;
    }
    if (fourth_nibble == 0x6)
    {
        return Instruction::I_8XY6;
    }
    if (fourth_nibble == 0x7)
    {
        return Instruction::I_8XY7;
    }
    if (fourth_nibble == 0xE)
    {
        return Instruction::I_8XYE;
    }

    return Instruction::UNINITIALIZED;
}

auto Chip8::get_instruction_EXXX(const std::uint8_t third_nibble, const std::uint8_t fourth_nibble) -> Instruction
{
    if (third_nibble == 0x9 and fourth_nibble == 0xE)
    {
        return Instruction::I_EX9E;
    }
    if (third_nibble == 0xA and fourth_nibble == 0x1)
    {
        return Instruction::I_EXA1;
    }

    return Instruction::UNINITIALIZED;
}

auto Chip8::get_instruction_FXXX(const std::uint8_t third_nibble, const std::uint8_t fourth_nibble) -> Instruction
{
    if (third_nibble == 0x0 and fourth_nibble == 0x7)
    {
        return Instruction::I_8XY0;
    }
    if (third_nibble == 0x1 and fourth_nibble == 0x5)
    {
        return Instruction::I_8XY1;
    }
    if (third_nibble == 0x1 and fourth_nibble == 0x8)
    {
        return Instruction::I_8XY2;
    }
    if (third_nibble == 0x1 and fourth_nibble == 0xE)
    {
        return Instruction::I_8XY3;
    }
    if (third_nibble == 0x0 and fourth_nibble == 0xA)
    {
        return Instruction::I_8XY0;
    }
    if (third_nibble == 0x2 and fourth_nibble == 0x9)
    {
        return Instruction::I_8XY1;
    }
    if (third_nibble == 0x3 and fourth_nibble == 0x3)
    {
        return Instruction::I_8XY2;
    }
    if (third_nibble == 0x5 and fourth_nibble == 0x5)
    {
        return Instruction::I_8XY3;
    }
    if (third_nibble == 0x6 and fourth_nibble == 0x5)
    {
        return Instruction::I_8XY3;
    }

    return Instruction::UNINITIALIZED;
}

auto Chip8::execute(const Instruction instruction, const Nibbles nibbles) -> void
{
    auto& [first_nibble, second_nibble,
        third_nibble, fourth_nibble] = nibbles;

    switch (instruction)
    {
    case Instruction::I_00E0:
        clear_screen();
        break;
    case Instruction::I_00EE:
        m_program_counter = m_stack.top();
        m_stack.pop();
        break;

    case Instruction::I_1NNN:
        m_program_counter = get_number_NNN(second_nibble, third_nibble, fourth_nibble);
        break;

    case Instruction::I_2NNN:
        m_stack.push(m_program_counter);
        m_program_counter = get_number_NNN(second_nibble, third_nibble, fourth_nibble);
        break;

    case Instruction::I_3XNN:
        if (m_registers.at(second_nibble) == get_number_NN(third_nibble, fourth_nibble))
        {
            m_program_counter += 1;
        }
        break;

    case Instruction::I_4XNN:
        if (m_registers.at(second_nibble) != get_number_NN(third_nibble, fourth_nibble))
        {
            m_program_counter += 1;
        }
        break;

    case Instruction::I_5XY0:
        if (m_registers.at(second_nibble) == m_registers.at(third_nibble))
        {
            m_program_counter += 1;
        }
        break;

    case Instruction::I_9XY0:
        if (m_registers.at(second_nibble) != m_registers.at(third_nibble))
        {
            m_program_counter += 1;
        }
        break;

    case Instruction::I_6XNN:
        m_registers.at(second_nibble) = get_number_NN(third_nibble, fourth_nibble);
        break;

    case Instruction::I_7XNN:
        // carry flag?
        m_registers.at(second_nibble) += get_number_NN(third_nibble, fourth_nibble);
        break;

    case Instruction::I_8XY0:
        m_registers.at(second_nibble) = m_registers.at(third_nibble);
        break;

    case Instruction::I_8XY1:
        m_registers.at(second_nibble) |= m_registers.at(third_nibble);
        break;

    case Instruction::I_8XY2:
        m_registers.at(second_nibble) &= m_registers.at(third_nibble);
        break;

    case Instruction::I_8XY3:
        m_registers.at(second_nibble) ^= m_registers.at(third_nibble);
        break;

    case Instruction::I_8XY4:
        m_registers.at(second_nibble) += m_registers.at(third_nibble);
        break;

    case Instruction::I_8XY5:
        m_registers.at(second_nibble) = m_registers.at(second_nibble) - m_registers.at(third_nibble); // VX - VY
        break;
    case Instruction::I_8XY7:
        m_registers.at(second_nibble) = m_registers.at(third_nibble) - m_registers.at(third_nibble); // VY - VX
        break;
    case Instruction::I_8XY6: // TODO: Implement further options later
        m_registers.at(second_nibble) = m_registers.at(third_nibble) >> 1;
        break;

    case Instruction::I_8XYE: // TODO: Implement further options later
        m_registers.at(second_nibble) = m_registers.at(third_nibble) << 1;
        break;

    case Instruction::I_ANNN:
        m_index_register = get_number_NNN(second_nibble, third_nibble, fourth_nibble);
        break;

    case Instruction::I_BNNN: // TODO: Implement further options later
        m_program_counter = get_number_NNN(second_nibble, third_nibble, fourth_nibble) + m_registers.at(0);
        break;

    case Instruction::I_CXNN:
        {
            std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_int_distribution<std::mt19937::result_type> dist(0,255);

            m_registers.at(second_nibble) = dist(rng) & get_number_NN(third_nibble, fourth_nibble);
        }
        break;

    case Instruction::I_DXYN:
        {
            auto X = m_registers.at(second_nibble) % 64;
            auto Y = m_registers.at(third_nibble) % 32;

            auto& VF = m_registers.at(15);
            VF = 0; // set VF 0 -> pixel turned off?

            const auto pixel_bits = m_memory.at(m_index_register + fourth_nibble);

            std::uint8_t check_bits = 0x80;
            while (true)
            {
                if ( check_bits & pixel_bits)
                {
                    if (m_display.at(X * Y) == 1)
                    {
                        m_display.at(X * Y) = 0;
                        VF = 1;
                    }
                    else
                    {
                        m_display.at(X * Y) = 1;
                    }

                }
                if (check_bits == 1)
                {
                    break;
                }

                check_bits = check_bits >> 1;

                X += 1;
                Y += 1;

                if (X % 64 == 0 or Y % 32 == 0)
                {
                    break;
                }
            }
        }
        break;

    case Instruction::I_EX9E:
        break;
    case Instruction::I_EXA1:
        break;
    case Instruction::I_FX07:
        break;
    case Instruction::I_FX15:
        break;
    case Instruction::I_FX18:
        break;
    case Instruction::I_FX1E:
        break;
    case Instruction::I_FX0A:
        break;
    case Instruction::I_FX29:
        break;
    case Instruction::I_FX33:
        break;
    case Instruction::I_FX55:
        break;
    case Instruction::I_FX65:
        break;
    case Instruction::UNINITIALIZED:
        [[fallthrough]]
    default:
        throw std::invalid_argument("Instruction is not valid!");
        break;
    }
}

auto Chip8::get_nibbles(std::uint16_t instruction) -> Nibbles
{
    const Nibbles nibbles{
        .first_nibble = static_cast<std::uint8_t>((instruction & 0xF000) >> 12),
        .second_nibble = static_cast<std::uint8_t>((instruction & 0x0F00) >> 8),
        .third_nibble = static_cast<std::uint8_t>((instruction & 0x00F0) >> 4),
        .fourth_nibble = static_cast<std::uint8_t>((instruction & 0x000F) >> 0),
    };

    return nibbles;
}

auto Chip8::get_number_NN(const std::uint8_t third_nibble, const std::uint8_t fourth_nibble) -> std::uint16_t
{
    return third_nibble << 4 | fourth_nibble << 0;
}

auto Chip8::get_number_NNN(const std::uint8_t second_nibble, const std::uint8_t third_nibble,
    const std::uint8_t fourth_nibble) -> std::uint16_t
{
    return second_nibble << 8 | third_nibble << 4 | fourth_nibble << 0;
}

auto Chip8::clear_screen() -> void
{
    for (auto& pixel : m_display)
    {
        pixel = 0;
    }
}

auto main(int argc, char** argv) -> int
{
    //std::string c = "\u2B1C";
    //std::print("{}",c); //works in real terminal
    try
    {
        Chip8 chip8;

        std::filesystem::path file_path{};
        if (argc == 2)
        {
            file_path = argv[1];
        }
        else
        {
            throw std::runtime_error("Missing ROM name as argument!");
        }

        chip8.read_rom(file_path);
    }
    catch (const std::runtime_error& re)
    {
        std::println(std::cerr, "{}", re.what());
    }
    catch (const std::invalid_argument& ia)
    {
        std::println(std::cerr, "{}", ia.what());
    }

    return 0;
}
