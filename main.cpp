#include <filesystem>
#include <iostream>
#include <print>
#include <fstream>
#include <stack>

class Chip8
{
public:
    enum class Instruction
    {
        I_00E0,
        I_00EE,
        I_1NNN,
        I_2NNN,
        I_3XNN,
        I_4XNN,
        I_5XY0,
        I_9XY0,
        I_6XNN,
        I_7XNN,
        I_8XY0,
        I_8XY1,
        I_8XY2,
        I_8XY3,
        I_8XY4,
        I_8XY5,
        I_8XY7,
        I_8XY6,
        I_8XYE,
        I_ANNN,
        I_BNNN,
        I_CXNN,
        I_DXYN,
        I_EX9E,
        I_EXA1,
        I_FX07,
        I_FX15,
        I_FX18,
        I_FX1E,
        I_FX0A,
        I_FX29,
        I_FX33,
        I_FX55,
        I_FX65,
        UNINITIALIZED,
    };

    struct Nibbles
    {
        std::uint8_t first_nibble;
        std::uint8_t second_nibble;
        std::uint8_t third_nibble;
        std::uint8_t fourth_nibble;
    };

    Chip8()
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

        // Write to memory from 0x50 to 0x9F
        for (int i = 0x50; const auto& f : font)
        {
            m_memory.at(i) = f;
            i++;
        }
    }

    auto read_rom(const std::filesystem::path& file_path) -> void
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

    [[noreturn]] auto main_loop() -> void
    {
        while (true)
        {
            const auto raw_instruction = fetch();
            auto instruction = decode(raw_instruction);
            execute(instruction);
        }
    }

    [[nodiscard]] auto fetch() -> std::uint16_t
    {
        const auto instruction_first_byte = m_memory.at(m_program_counter);
        const auto instruction_second_byte = m_memory.at(m_program_counter + 1);

        std::uint16_t instruction = 0x0000;
        instruction = instruction | instruction_first_byte << 8;
        instruction = instruction | instruction_second_byte;

        return instruction;
    }

    [[nodiscard]]auto decode(const std::uint16_t raw_instruction) -> Instruction
    {
        auto nibbles = get_nibbles(raw_instruction);
        auto& [first_nibble, second_nibble, third_nibble, fourth_nibble] = nibbles;

        Instruction instruction{Instruction::UNINITIALIZED};
        switch (first_nibble)
        {
        case 0x0:
            break;
        // Jump
        case 0x1:
            {
                //const std::uint16_t location = second_nibble << 8 | third_nibble << 4 | fourth_nibble;
                //m_program_counter = location;
            }
            break;
        case 0x2:
            break;
        case 0x3:
            break;
        case 0x4:
            break;
        case 0x5:
            break;
        case 0x6:
            break;
        case 0x7:
            break;
        case 0x8:
            break;
        case 0x9:
            break;
        case 0xA:
            break;
        case 0xB:
            break;
        case 0xC:
            break;
        case 0xD:
            break;
        case 0xE:
            break;
        case 0xF:

        default:
            throw std::runtime_error("Invalid OPCODE!");
        }
    }

    auto execute(Instruction instruction) -> void
    {
    }

    [[nodiscard]] static auto get_nibbles(std::uint16_t instruction) -> Nibbles
    {
        Nibbles nibbles{};
        nibbles.first_nibble = (instruction & 0x1000) >> 12;
        nibbles.second_nibble = (instruction & 0x0100) >> 8;
        nibbles.third_nibble = (instruction & 0x0010) >> 4;
        nibbles.fourth_nibble = (instruction & 0x0001) >> 0;
        return nibbles;
    }

    auto clear_screen() -> void
    {
        for (auto& pixel : m_display)
        {
            pixel = 0;
        }
    }

private:
    std::array<std::uint8_t, 16> m_registers{};
    std::uint16_t m_index_register{};
    std::uint16_t m_program_counter{};

    std::stack<std::uint16_t> m_stack{}; //16 elements or more, doesnt matter
    std::uint8_t m_stack_ptr{};

    std::uint8_t m_delay_timer{};
    std::uint8_t m_sound_timer{};

    std::array<std::uint8_t, 4096> m_memory{};

    std::array<int, 2048> m_display{}; //64x32 Pixels
};

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

    return 0;
}
