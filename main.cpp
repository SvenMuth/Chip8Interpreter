#include <iostream>
#include <print>
#include <fstream>
#include <random>
#include <thread>

#include "main.h"

using namespace std::chrono_literals;


extern "C"{
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

    void enable_raw_mode_input() {
        struct termios term;
        tcgetattr(STDIN_FILENO, &term);
        term.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
    }
}


Chip8::Chip8()
{
    m_program_counter = START_ADDRESS;

    //Write font to memory from 0x50 to 0x9F
    for (int i{FONTSET_START_ADDRESS}; const auto& f : FONTS)
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

        std::this_thread::sleep_for(5ms);
        const auto opcode = fetch();

        const auto nibbles = get_nibbles(opcode);
        const auto instruction = decode(nibbles);
        std::println("{}", static_cast<int>(instruction));

        execute(instruction, nibbles);

        if (m_delay_timer > 0)
        {
            m_delay_timer--;
        }

        if (m_sound_timer > 0)
        {
            m_sound_timer--;
        }
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
    switch (nibbles.first_nibble)
    {
    case 0x0: return get_instruction_0XXX(nibbles);
    case 0x1: return Instruction::I_1NNN;
    case 0x2: return Instruction::I_2NNN;
    case 0x3: return Instruction::I_3XNN;
    case 0x4: return Instruction::I_4XNN;
    case 0x5: return Instruction::I_5XY0;
    case 0x6: return Instruction::I_6XNN;
    case 0x7: return Instruction::I_7XNN;
    case 0x8: return get_instruction_8XXX(nibbles);
    case 0x9: return Instruction::I_9XY0;
    case 0xA: return Instruction::I_ANNN;
    case 0xB: return Instruction::I_BNNN;
    case 0xC: return Instruction::I_CXNN;
    case 0xD: return Instruction::I_DXYN;
    case 0xE: return get_instruction_EXXX(nibbles);
    case 0xF: return get_instruction_FXXX(nibbles);
    default: throw std::invalid_argument("Invalid opcode!");
    }
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
    switch (instruction)
    {
    case Instruction::I_00E0: OP_00E0(); break;
    case Instruction::I_00EE: OP_00EE(); break;
    case Instruction::I_1NNN: OP_1NNN(nibbles); break;
    case Instruction::I_2NNN: OP_2NNN(nibbles); break;
    case Instruction::I_3XNN: OP_3XNN(nibbles); break;
    case Instruction::I_4XNN: OP_4XNN(nibbles); break;
    case Instruction::I_5XY0: OP_5XY0(nibbles); break;
    case Instruction::I_9XY0: OP_9XY0(nibbles); break;
    case Instruction::I_6XNN: OP_6XNN(nibbles); break;
    case Instruction::I_7XNN: OP_7XNN(nibbles); break;
    case Instruction::I_8XY0: OP_8XY0(nibbles); break;
    case Instruction::I_8XY1: OP_8XY1(nibbles); break;
    case Instruction::I_8XY2: OP_8XY2(nibbles); break;
    case Instruction::I_8XY3: OP_8XY3(nibbles); break;
    case Instruction::I_8XY4: OP_8XY4(nibbles); break;
    case Instruction::I_8XY5: OP_8XY5(nibbles); break;
    case Instruction::I_8XY7: OP_8XY7(nibbles); break;
    case Instruction::I_8XY6: OP_8XY6(nibbles); break;
    case Instruction::I_8XYE: OP_8XYE(nibbles); break;
    case Instruction::I_ANNN: OP_ANNN(nibbles); break;
    case Instruction::I_BNNN: OP_BNNN(nibbles); break;
    case Instruction::I_CXNN: OP_CXNN(nibbles); break;
    case Instruction::I_DXYN: OP_DXYN(nibbles); break;
    case Instruction::I_EX9E: OP_EX9E(nibbles); break;
    case Instruction::I_EXA1: OP_EXA1(nibbles); break;
    case Instruction::I_FX07: OP_FX07(nibbles); break;
    case Instruction::I_FX15: OP_FX15(nibbles); break;
    case Instruction::I_FX18: OP_FX18(nibbles); break;
    case Instruction::I_FX1E: OP_FX1E(nibbles); break;
    case Instruction::I_FX0A: OP_FX0A(nibbles); break;
    case Instruction::I_FX29: OP_FX29(nibbles); break;
    case Instruction::I_FX33: OP_FX33(nibbles); break;
    case Instruction::I_FX55: OP_FX55(nibbles); break;
    case Instruction::I_FX65: OP_FX65(nibbles); break;
    case Instruction::UNINITIALIZED:
    default: throw std::invalid_argument("Instruction is not valid!");
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

auto Chip8::OP_3XNN(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    if (VX == get_number_NN(nibbles))
    {
        m_program_counter += 2;
    }
}

auto Chip8::OP_4XNN(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    if (VX != get_number_NN(nibbles))
    {
        m_program_counter += 2;
    }
}

auto Chip8::OP_5XY0(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    const auto& VY = m_registers.at(nibbles.third_nibble);

    if (VX == VY)
    {
        m_program_counter += 2;
    }
}

auto Chip8::OP_9XY0(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    const auto& VY = m_registers.at(nibbles.third_nibble);

    if (VX != VY)
    {
        m_program_counter += 2;
    }
}

auto Chip8::OP_6XNN(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    VX = get_number_NN(nibbles);
}

auto Chip8::OP_7XNN(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    VX += get_number_NN(nibbles);
}

auto Chip8::OP_8XY0(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    const auto& VY = m_registers.at(nibbles.third_nibble);

    VX = VY;
}

auto Chip8::OP_8XY1(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    const auto& VY = m_registers.at(nibbles.third_nibble);

    VX |= VY;
}

auto Chip8::OP_8XY2(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    const auto& VY = m_registers.at(nibbles.third_nibble);

    VX &= VY;
}

auto Chip8::OP_8XY3(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    const auto& VY = m_registers.at(nibbles.third_nibble);

    VX ^= VY;
}

auto Chip8::OP_8XY4(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    const auto& VY = m_registers.at(nibbles.third_nibble);
    const std::uint16_t sum = VX + VY;

    if (sum > 255)
    {
        m_registers.at(0xF) = 1;
    }
    else
    {
        m_registers.at(0xF) = 0;
    }

    VX = sum & 0xFF;
}

auto Chip8::OP_8XY5(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    const auto& VY = m_registers.at(nibbles.third_nibble);

    if (VX > VY)
    {
        m_registers.at(0xF) = 1;
    }
    else
    {
        m_registers.at(0xF) = 0;
    }

    VX = VX - VY;
}

auto Chip8::OP_8XY7(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    const auto& VY = m_registers.at(nibbles.third_nibble);

    if (VY > VX)
    {
        m_registers.at(0xF) = 1;
    }
    else
    {
        m_registers.at(0xF) = 0;
    }

    VX = VY - VX;
}

auto Chip8::OP_8XY6(const Nibbles nibbles) -> void
{
    //TODO: Implement further options later
    auto& VX = m_registers.at(nibbles.second_nibble);
    m_registers.at(0xF) = VX & 0x1;

    VX >>= 1;
}

auto Chip8::OP_8XYE(const Nibbles nibbles) -> void
{
    //TODO: Implement further options later
    auto& VX = m_registers.at(nibbles.second_nibble);
    m_registers.at(0xF) = (VX & 0x80u) >> 7u;

    VX <<= 1;
}

auto Chip8::OP_ANNN(const Nibbles nibbles) -> void
{
    m_index_register = get_number_NNN(nibbles);
}

auto Chip8::OP_BNNN(const Nibbles nibbles) -> void
{
    //TODO: Implement further options later
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

auto Chip8::OP_DXYN(const Nibbles nibbles) -> void
{
    auto [first_nibble, second_nibble,
        third_nibble, fourth_nibble] = nibbles;

    const auto VX = m_registers.at(second_nibble);
    const auto VY = m_registers.at(third_nibble);

    const auto X = VX % DISPLAY_WIDTH;
    const auto Y = VY % DISPLAY_HEIGHT;

    auto& VF = m_registers.at(15);
    VF = 0;

    for (int row{0}; row < fourth_nibble; row++)
    {
        const uint8_t sprite_byte = m_memory.at(m_index_register + row);
        for (unsigned int col = 0; col < 8; col++)
        {
            const uint8_t sprite_pixel = sprite_byte & (0x80 >> col);
            bool& screen_pixel = m_display.at((Y + row) * DISPLAY_WIDTH + (X + col));

            if (sprite_pixel)
            {
                if (screen_pixel)
                {
                    VF = 1;
                }

                screen_pixel = !screen_pixel;
            }
        }
    }

    //Clear terminal
    std::print("\033[H\033[J");
    std::print("\n\n\t\t\t\t");
    for (int counter{1}; const auto& pixel: m_display)
    {
        if (pixel)
        {
            //White Large Square Unicode
            std::print("\u2B1C");
        }
        else
        {
            //Black Large Square Unicode
            std::print("\u2B1B");
        }

        if (counter % DISPLAY_WIDTH == 0)
        {
            std::print("\n\t\t\t\t");
        }
        counter++;
    }

    constexpr auto keymap_str = R"(
                                1 2 3 4      1 2 3 C
                                Q W E R  =>  4 5 6 D
                                A S D F      7 8 9 E
                                Y X C V      A 0 B F)";
    std::println("{}", keymap_str);
}

auto Chip8::OP_EX9E(const Nibbles nibbles) -> void
{
    //have to be tested
    enable_raw_mode_input();

    auto& VX = m_registers.at(nibbles.second_nibble);
    const auto ch = std::getchar();

    if (ch == EOF)
    {
        return;
    }

    const auto val = get_value_char_to_key_map(ch);
    if (VX == val && val <= 0xF)
    {
        m_program_counter += 2;
    }
}

auto Chip8::OP_EXA1(const Nibbles nibbles) -> void
{
    //have to be tested
    enable_raw_mode_input();
    auto& VX = m_registers.at(nibbles.second_nibble);

    const auto ch = std::getchar();
    const auto val = get_value_char_to_key_map(ch);

    if (VX != val && val <= 0xF)
    {
        m_program_counter += 2;
    }
}

auto Chip8::OP_FX07(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    VX = m_delay_timer;
}

auto Chip8::OP_FX15(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    m_delay_timer = VX;
}

auto Chip8::OP_FX18(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    m_sound_timer = VX;
}

auto Chip8::OP_FX1E(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    m_index_register += VX;
}

auto Chip8::OP_FX0A(const Nibbles nibbles) -> void
{
    enable_raw_mode_input();

    auto& VX = m_registers.at(nibbles.second_nibble);

    const auto ch = std::getchar();
    const auto val = get_value_char_to_key_map(ch);

    if (val > 0xF or ch == EOF)
    {
        m_program_counter -= 2;
        return;
    }

    VX = val;
}

auto Chip8::OP_FX29(const Nibbles nibbles) -> void
{
    auto& VX = m_registers.at(nibbles.second_nibble);
    m_index_register = FONTSET_START_ADDRESS + (5 * VX);
}

auto Chip8::OP_FX33(const Nibbles nibbles) -> void
{
    //have to be tested
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
    const auto VX = nibbles.second_nibble;

    //Is the if statement needed?
    if (VX != 0)
    {
        for (unsigned int index = 0; index < VX; index++)
        {
            m_memory.at(I + index) = m_registers.at(index);
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
    //TODO: check if implemented correctly
    auto I = m_index_register;
    const auto VX = nibbles.second_nibble;

    //Is the if statement needed?
    if (VX != 0)
    {
        for (unsigned int index = 0; index < VX; index++)
        {
            m_registers.at(index) = m_memory.at(I + index);
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
        chip8.main_loop();
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
