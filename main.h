//
// Created by sven on 01.03.25.
//

#ifndef MAIN_H
#define MAIN_H

#include <filesystem>
#include <print>
#include <stack>

class Chip8
{
public:
    enum class Instruction
    {
        I_00E0, I_00EE,
        I_1NNN,
        I_2NNN,
        I_3XNN,
        I_4XNN,
        I_5XY0,
        I_6XNN,
        I_7XNN,
        I_8XY0, I_8XY1, I_8XY2, I_8XY3, I_8XY4, I_8XY5, I_8XY7, I_8XY6, I_8XYE,
        I_9XY0,
        I_ANNN,
        I_BNNN,
        I_CXNN,
        I_DXYN,
        I_EX9E, I_EXA1,
        I_FX07, I_FX15, I_FX18, I_FX1E, I_FX0A, I_FX29, I_FX33, I_FX55, I_FX65,
        UNINITIALIZED,
    };

    struct Nibbles
    {
        std::uint8_t first_nibble;
        std::uint8_t second_nibble;
        std::uint8_t third_nibble;
        std::uint8_t fourth_nibble;
    };

    Chip8();

    auto read_rom(const std::filesystem::path& file_path) -> void;
    [[noreturn]] auto main_loop() -> void;

    [[nodiscard]] auto fetch() const -> std::uint16_t;
    [[nodiscard]] static auto decode(std::uint16_t opcode) -> Instruction;

    [[nodiscard]] static auto get_instruction_0XXX(std::uint8_t third_nibble,
        std::uint8_t fourth_nibble) -> Instruction;

    [[nodiscard]] static auto get_instruction_8XXX(std::uint8_t fourth_nibble) -> Instruction;
    [[nodiscard]] static auto get_instruction_EXXX(std::uint8_t third_nibble,
        std::uint8_t fourth_nibble) -> Instruction;

    [[nodiscard]] static auto get_instruction_FXXX(std::uint8_t third_nibble,
        std::uint8_t fourth_nibble) -> Instruction;

    auto execute(Instruction instruction) -> void;
    [[nodiscard]] static auto get_nibbles(std::uint16_t instruction) -> Nibbles;
    auto clear_screen() -> void;

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

#endif //MAIN_H
