#include <iostream>
#include <print>
#include <fstream>
#include <random>
#include <ncurses.h>

#include "main.h"

Chip8::Chip8()
{
    m_program_counter = START_ADDRESS;

    //Write font to memory from 0x50 to 0x9F
    for (int i{FONTSET_START_ADDRESS}; const auto& f : FONT)
    {
        m_memory.at(i) = f;
        i++;
    }
}

auto Chip8::read_rom(const std::filesystem::path& file_path) -> void
{
    std::ifstream rom(file_path, std::ios::binary | std::ios::in);
    if (!rom.good())
    {
        throw std::runtime_error("Failed to open ROM!");
    }

    char c{};

    //Standard starting position in memory for ROM data
    int memory_pos{START_ADDRESS};
    while (rom.get(c))
    {
        if (memory_pos >= m_memory.size())
        {
            throw std::runtime_error("ROM size is to big for memory!");
        }
        m_memory.at(memory_pos) = static_cast<uint8_t>(c);
        memory_pos++;
    }

    rom.close();
}

auto Chip8::main_loop() -> void
{
    while (true)
    {
        const auto opcode = fetch();

        const auto nibbles = get_nibbles(opcode);
        const auto instruction = decode(nibbles);

        execute(instruction, nibbles);
    }
}

auto Chip8::fetch() -> std::uint16_t
{
    const auto opcode_first_byte = m_memory.at(m_program_counter);
    const auto opcode_second_byte = m_memory.at(m_program_counter + 1);

    m_program_counter += 2;

    std::uint16_t opcode{0x0};
    opcode |= opcode_first_byte << 8;
    opcode |= opcode_second_byte << 0;

    return opcode;
}

auto Chip8::decode(const Nibbles nibbles) -> Instruction
{
    auto [first_nibble, second_nibble,
        third_nibble, fourth_nibble] = nibbles;

    switch (first_nibble)
    {
    case 0x0:
        return get_instruction_0XXX(nibbles);
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
        return get_instruction_8XXX(nibbles);
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
        return get_instruction_EXXX(nibbles);
        break;
    case 0xF:
        return get_instruction_FXXX(nibbles);
        break;
    default:
        throw std::invalid_argument("Invalid opcode!");
        break;
    }

    return Instruction::UNINITIALIZED;
}

auto Chip8::get_instruction_0XXX(const Nibbles nibbles) -> Instruction
{
    auto [first_nibble, second_nibble,
        third_nibble, fourth_nibble] = nibbles;

    if (third_nibble == 0xE and fourth_nibble == 0x0) return Instruction::I_00E0;
    if (third_nibble == 0xE and fourth_nibble == 0xE) return Instruction::I_00EE;

    return Instruction::UNINITIALIZED;
}

auto Chip8::get_instruction_8XXX(const Nibbles nibbles) -> Instruction
{
    const auto fourth_nibble = nibbles.fourth_nibble;

    if (fourth_nibble == 0x0) return Instruction::I_8XY0;
    if (fourth_nibble == 0x1) return Instruction::I_8XY1;
    if (fourth_nibble == 0x2) return Instruction::I_8XY2;
    if (fourth_nibble == 0x3) return Instruction::I_8XY3;
    if (fourth_nibble == 0x4) return Instruction::I_8XY4;
    if (fourth_nibble == 0x5) return Instruction::I_8XY5;
    if (fourth_nibble == 0x6) return Instruction::I_8XY6;
    if (fourth_nibble == 0x7) return Instruction::I_8XY7;
    if (fourth_nibble == 0xE) return Instruction::I_8XYE;

    return Instruction::UNINITIALIZED;
}

auto Chip8::get_instruction_EXXX(const Nibbles nibbles) -> Instruction
{
    auto [first_nibble, second_nibble,
        third_nibble, fourth_nibble] = nibbles;

    if (third_nibble == 0x9 and fourth_nibble == 0xE) return Instruction::I_EX9E;
    if (third_nibble == 0xA and fourth_nibble == 0x1) return Instruction::I_EXA1;

    return Instruction::UNINITIALIZED;
}

auto Chip8::get_instruction_FXXX(const Nibbles nibbles) -> Instruction
{
    auto [first_nibble, second_nibble,
        third_nibble, fourth_nibble] = nibbles;

    if (third_nibble == 0x0 and fourth_nibble == 0x7) return Instruction::I_FX07;
    if (third_nibble == 0x1 and fourth_nibble == 0x5) return Instruction::I_FX15;
    if (third_nibble == 0x1 and fourth_nibble == 0x8) return Instruction::I_FX18;
    if (third_nibble == 0x1 and fourth_nibble == 0xE) return Instruction::I_FX1E;
    if (third_nibble == 0x0 and fourth_nibble == 0xA) return Instruction::I_FX0A;
    if (third_nibble == 0x2 and fourth_nibble == 0x9) return Instruction::I_FX29;
    if (third_nibble == 0x3 and fourth_nibble == 0x3) return Instruction::I_FX33;
    if (third_nibble == 0x5 and fourth_nibble == 0x5) return Instruction::I_FX55;
    if (third_nibble == 0x6 and fourth_nibble == 0x5) return Instruction::I_FX65;

    return Instruction::UNINITIALIZED;
}

auto Chip8::execute(const Instruction instruction, const Nibbles nibbles) -> void
{
    auto [first_nibble, second_nibble,
        third_nibble, fourth_nibble] = nibbles;

    auto& VX = m_registers.at(second_nibble);
    const auto& VY = m_registers.at(third_nibble);

    switch (instruction)
    {
    case Instruction::I_00E0:
        OP_00E0();
        break;

    case Instruction::I_00EE:
        OP_00EE();
        break;

    case Instruction::I_1NNN:
        OP_1NNN(nibbles);
        break;

    case Instruction::I_2NNN:
        OP_2NNN(nibbles);
        break;

    case Instruction::I_3XNN:
        OP_3XNN(VX, nibbles);
        break;

    case Instruction::I_4XNN:
        OP_4XN(VX, nibbles);
        break;

    case Instruction::I_5XY0:
        OP_5XY0(VX, VY);
        break;

    case Instruction::I_9XY0:
        OP_9XY0(VX, VY);
        break;

    case Instruction::I_6XNN:
        OP_6XNN(VX, nibbles);
        break;

    case Instruction::I_7XNN:
        //carry flag?
        OP_7XNN(VX, nibbles);
        break;

    case Instruction::I_8XY0:
        OP_8XY0(VX, VY);
        break;

    case Instruction::I_8XY1:
        OP_8XY1(VX, VY);
        break;

    case Instruction::I_8XY2:
        OP_8XY2(VX, VY);
        break;

    case Instruction::I_8XY3:
        OP_8XY3(VX, VY);
        break;

    case Instruction::I_8XY4:
        OP_8XY4(VX, VY);
        break;

    case Instruction::I_8XY5:
        OP_8XY4(VX, VY);
        break;
    case Instruction::I_8XY7:
        OP_8XY7(VX, VY);
        break;
    case Instruction::I_8XY6: //TODO: Implement further options later
        OP_8XY6(VX, VY);
        break;

    case Instruction::I_8XYE: //TODO: Implement further options later
        OP_8XYE(VX, VY);
        break;

    case Instruction::I_ANNN:
        OP_ANNN(nibbles);
        break;

    case Instruction::I_BNNN: //TODO: Implement further options later
        OP_BNNN(nibbles);
        break;

    case Instruction::I_CXNN:
        OP_CXNN(nibbles);
        break;

    case Instruction::I_DXYN:
        OP_DXYN(nibbles);
        break;

    case Instruction::I_EX9E:
        OP_EX9E(VX);
        break;

    case Instruction::I_EXA1:
        OP_EXA1(VX);
        break;

    case Instruction::I_FX07:
        OP_FX07(VX);
        break;

    case Instruction::I_FX15:
        OP_FX15(VX);
        break;

    case Instruction::I_FX18:
        OP_FX18(VX);
        break;

    case Instruction::I_FX1E:
        OP_FX1E(VX);
        break;

    case Instruction::I_FX0A:
        OP_FX0A(VX);
        break;

    case Instruction::I_FX29: //Research instruction
        OP_FX29();
        break;

    case Instruction::I_FX33:
        OP_FX33(nibbles);
        break;

    case Instruction::I_FX55:
        OP_FX55(nibbles);
        break;

    case Instruction::I_FX65: //TODO: check if implemented correctly
        OP_FX65(nibbles);
        break;

    case Instruction::UNINITIALIZED:
    default:
        throw std::invalid_argument("Instruction is not valid!");
        break;
    }
}

auto Chip8::OP_00E0() -> void
{
    m_display.fill(false);
}

auto Chip8::OP_00EE() -> void
{
    m_program_counter = m_stack.top();
    m_stack.pop();
}

auto Chip8::OP_1NNN(const Nibbles nibbles) -> void
{
    m_program_counter = get_number_NNN(nibbles);
}

auto Chip8::OP_2NNN(const Nibbles nibbles) -> void
{
    m_stack.push(m_program_counter);
    m_program_counter = get_number_NNN(nibbles);
}

auto Chip8::OP_3XNN(const std::uint8_t& VX, const Nibbles nibbles) -> void
{
    if (VX == get_number_NN(nibbles)) m_program_counter += 2;
}

auto Chip8::OP_4XN(const std::uint8_t& VX, const Nibbles nibbles) -> void
{
    if (VX != get_number_NN(nibbles)) m_program_counter += 2;
}

auto Chip8::OP_5XY0(const std::uint8_t& VX, const std::uint8_t& VY) -> void
{
    if (VX == VY) m_program_counter += 2;
}

auto Chip8::OP_9XY0(const std::uint8_t& VX, const std::uint8_t& VY) -> void
{
    if (VX != VY) m_program_counter += 2;
}

auto Chip8::OP_6XNN(std::uint8_t& VX, const Nibbles nibbles) -> void
{
    VX = get_number_NN(nibbles);
}

auto Chip8::OP_7XNN(std::uint8_t& VX, const Nibbles nibbles) -> void
{
    //carry flag?
    VX += get_number_NN(nibbles);
}

auto Chip8::OP_8XY0(std::uint8_t& VX, const std::uint8_t& VY) -> void
{
    VX = VY;
}

auto Chip8::OP_8XY1(std::uint8_t& VX, const std::uint8_t& VY) -> void
{
    VX |= VY;
}

auto Chip8::OP_8XY2(std::uint8_t& VX, const std::uint8_t& VY) -> void
{
    VX &= VY;
}

auto Chip8::OP_8XY3(std::uint8_t& VX, const std::uint8_t& VY) -> void
{
    VX ^= VY;
}

auto Chip8::OP_8XY4(std::uint8_t& VX, const std::uint8_t& VY) -> void
{
    VX += VY;
}

auto Chip8::OP_8XY5(std::uint8_t& VX, const std::uint8_t& VY) -> void
{
    VX = VX - VY;
}

auto Chip8::OP_8XY7(std::uint8_t& VX, const std::uint8_t& VY) -> void
{
    VX = VY - VX;
}

auto Chip8::OP_8XY6(std::uint8_t& VX, const std::uint8_t& VY) -> void
{
    VX = VY >> 1;
}

auto Chip8::OP_8XYE(std::uint8_t& VX, const std::uint8_t& VY) -> void
{
    VX = VY << 1;
}

auto Chip8::OP_ANNN(const Nibbles nibbles) -> void
{
    m_index_register = get_number_NNN(nibbles);
}

auto Chip8::OP_BNNN(const Nibbles nibbles) -> void
{
    m_program_counter = get_number_NNN(nibbles) + m_registers.at(0);
}

auto Chip8::OP_CXNN(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 255);

    VX = dist(rng) & get_number_NN(nibbles);
}

auto Chip8::OP_DXYN(Nibbles nibbles) -> void
{
    auto [first_nibble, second_nibble,
        third_nibble, fourth_nibble] = nibbles;

    const auto& VX = m_registers.at(second_nibble);
    const auto& VY = m_registers.at(third_nibble);

    auto X = VX % 64;
    auto Y = VY % 32;

    auto& VF = m_registers.at(15);
    VF = 0; //set VF 0 -> pixel turned off?

    const auto pixel_bits = m_memory.at(m_index_register + fourth_nibble);

    std::uint8_t check_bits{0x80};
    int counter{0};

    while (true)
    {
        if (check_bits & pixel_bits)
        {
            if (m_display.at(X * Y) == 1)
            {
                m_display.at(X * Y) = false;
                VF = 1;
            }
            else
            {
                m_display.at(X * Y) = true;
            }
        }
        if (counter == 7) //right value?
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

        counter++;
    }
}

auto Chip8::OP_EX9E(const std::uint8_t& VX) -> void
{
    const auto val = get_value_char_to_key_map(getch());
    if (VX == val && val < 0xF) m_program_counter += 2;
}

auto Chip8::OP_EXA1(const std::uint8_t& VX) -> void
{
    const auto val = get_value_char_to_key_map(getch());
    if (VX != val && val < 0xF) m_program_counter += 2;
}

auto Chip8::OP_FX07(std::uint8_t& VX) const -> void
{
    VX = m_delay_timer;
}

auto Chip8::OP_FX15(const std::uint8_t& VX) -> void
{
    m_delay_timer = VX;
}

auto Chip8::OP_FX18(const std::uint8_t& VX) -> void
{
    m_sound_timer = VX;
}

auto Chip8::OP_FX1E(const std::uint8_t& VX) -> void
{
    m_index_register += VX;
}

auto Chip8::OP_FX0A(std::uint8_t& VX) -> void
{
    VX = get_value_char_to_key_map(std::getchar());
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

auto Chip8::OP_FX29() -> void
{
}

auto Chip8::OP_FX33(const Nibbles nibbles) -> void
{
    const auto& VX = m_registers.at(nibbles.second_nibble);

    auto number = VX;
    auto I = m_index_register;

    if (number > 100)
    {
        m_memory.at(I) = number / 100;
        number %= 100;
        I += 1;
    }
    if (number > 10)
    {
        m_memory.at(I) = number / 10;
        number %= 10;
        I += 1;
    }

    m_memory.at(I) = number;
}

auto Chip8::OP_FX55(const Nibbles nibbles) -> void
{
    auto I = m_index_register;
    if (nibbles.second_nibble != 0)
    {
        for (int index = 0; index < nibbles.second_nibble; index++)
        {
            m_memory.at(I) = m_registers.at(index);
            I++;
        }
    }
    else
    {
        m_memory.at(I) = m_registers.at(0);
    }
}

auto Chip8::OP_FX65(const Nibbles nibbles) -> void
{
    auto I = m_index_register;
    if (nibbles.second_nibble != 0)
    {
        for (int index = 0; index < nibbles.second_nibble; index++)
        {
            m_registers.at(I) = m_memory.at(index);
            I++;
        }
    }
    else
    {
        m_registers.at(0) = m_memory.at(I);
    }
}

auto Chip8::get_value_char_to_key_map(int key) -> std::uint8_t
{
    key = std::tolower(key);
    if (!CHAR_TO_KEYMAP.contains(key))
    {
        //TODO: Implement
        return static_cast<int>(std::numeric_limits<uint8_t>::max());
    }

    auto c = CHAR_TO_KEYMAP.at(key);

    return static_cast<std::uint8_t>(c);
}

auto Chip8::get_nibbles(const std::uint16_t instruction) -> Nibbles
{
    const Nibbles nibbles{
        .first_nibble = static_cast<std::uint8_t>((instruction & 0xF000) >> 12),
        .second_nibble = static_cast<std::uint8_t>((instruction & 0x0F00) >> 8),
        .third_nibble = static_cast<std::uint8_t>((instruction & 0x00F0) >> 4),
        .fourth_nibble = static_cast<std::uint8_t>((instruction & 0x000F) >> 0),
    };

    return nibbles;
}

auto Chip8::get_number_NN(const Nibbles nibbles) -> std::uint8_t
{
    auto [first_nibble, second_nibble,
        third_nibble, fourth_nibble] = nibbles;

    return third_nibble << 4 | fourth_nibble << 0;
}

auto Chip8::get_number_NNN(const Nibbles nibbles) -> std::uint16_t
{
    auto [first_nibble, second_nibble,
        third_nibble, fourth_nibble] = nibbles;

    return second_nibble << 8 | third_nibble << 4 | fourth_nibble << 0;
}

auto main(int argc, char** argv) -> int
{
    //std::string c = "\u2B1C";
    //std::print("{}",c); //works in real terminal

    try
    {
        //init ncurses
        //initscr();
        //raw();
        //noecho();

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
        chip8.main_loop();

        //end ncurses
        //endwin();
    }
    catch (const std::runtime_error& re)
    {
        std::println(std::cerr, "{}", re.what());
    }
    catch (const std::invalid_argument& ia)
    {
        std::println(std::cerr, "{}", ia.what());
    }
    catch (...)
    {
        std::println(std::cerr, "Unexpected exception occurred!");
    }

    return 0;
}
