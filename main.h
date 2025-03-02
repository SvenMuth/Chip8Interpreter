//
// Created by sven on 01.03.25.
//

#ifndef MAIN_H
#define MAIN_H

#include <filesystem>
#include <stack>
#include <unordered_map>

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

    enum class Keymap
    {
        /*
        * Keymap
        * 1 2 3 4      1 2 3 C
        * Q W E R  =>  4 5 6 D
        * A S D F      7 8 9 E
        * Y X C V      A 0 B F
        */

        K_1 = 0x1, K_2 = 0x2, K_3 = 0x3, K_4 = 0xC,
        K_Q = 0x4, K_W = 0x5, K_E = 0x6, K_R = 0xD,
        K_A = 0x7, K_S = 0x8, K_D = 0x9, K_F = 0xE,
        K_Y = 0xA, K_X = 0x0, K_C = 0xB, K_V = 0xF,
    };

    struct Nibbles
    {
        std::uint8_t first_nibble;
        std::uint8_t second_nibble;
        std::uint8_t third_nibble;
        std::uint8_t fourth_nibble;
    };

    static constexpr std::uint16_t START_ADDRESS{0x200};
    static constexpr int FONTSET_START_ADDRESS{0x50};

    static inline const std::unordered_map<int, Keymap> CHAR_TO_KEYMAP{
            {49, Keymap::K_1}, {50, Keymap::K_2}, {51, Keymap::K_3}, {52, Keymap::K_4},
            {81, Keymap::K_Q}, {87, Keymap::K_W}, {69, Keymap::K_E}, {82, Keymap::K_R},
            {65, Keymap::K_A}, {83, Keymap::K_S}, {68, Keymap::K_D}, {70, Keymap::K_F},
            {89, Keymap::K_Y}, {88, Keymap::K_X}, {67, Keymap::K_C}, {86, Keymap::K_V},
        };

    static constexpr std::array<uint8_t, 80> FONT{
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

    Chip8();

    auto read_rom(const std::filesystem::path& file_path) -> void;
    [[noreturn]] auto main_loop() -> void;

    [[nodiscard]] auto fetch() -> std::uint16_t;
    [[nodiscard]] static auto decode(Nibbles nibbles) -> Instruction;

    [[nodiscard]] static auto get_instruction_0XXX(Nibbles nibbles) -> Instruction;
    [[nodiscard]] static auto get_instruction_8XXX(Nibbles nibbles) -> Instruction;
    [[nodiscard]] static auto get_instruction_EXXX(Nibbles nibbles) -> Instruction;
    [[nodiscard]] static auto get_instruction_FXXX(Nibbles nibbles) -> Instruction;

    auto execute(Instruction instruction, Nibbles nibbles) -> void;

    auto OP_00E0() -> void;
    auto OP_00EE() -> void;
    auto OP_1NNN(Nibbles nibbles) -> void;
    auto OP_2NNN(Nibbles nibbles) -> void;
    auto OP_3XNN(const std::uint8_t& VX, Nibbles nibbles) -> void;
    auto OP_4XN(const std::uint8_t& VX, Nibbles nibbles) -> void;
    auto OP_5XY0(const std::uint8_t& VX, const std::uint8_t& VY) -> void;
    auto OP_9XY0(const std::uint8_t& VX, const std::uint8_t& VY) -> void;
    static auto OP_6XNN(std::uint8_t& VX, Nibbles nibbles) -> void;
    static auto OP_7XNN(std::uint8_t& VX, Nibbles nibbles) -> void;
    static auto OP_8XY0(std::uint8_t& VX, const std::uint8_t& VY) -> void;
    static auto OP_8XY1(std::uint8_t& VX, const std::uint8_t& VY) -> void;
    static auto OP_8XY2(std::uint8_t& VX, const std::uint8_t& VY) -> void;
    static auto OP_8XY3(std::uint8_t& VX, const std::uint8_t& VY) -> void;
    static auto OP_8XY4(std::uint8_t& VX, const std::uint8_t& VY) -> void;
    static auto OP_8XY5(std::uint8_t& VX, const std::uint8_t& VY) -> void;
    static auto OP_8XY7(std::uint8_t& VX, const std::uint8_t& VY) -> void;
    static auto OP_8XY6(std::uint8_t& VX, const std::uint8_t& VY) -> void;
    static auto OP_8XYE(std::uint8_t& VX, const std::uint8_t& VY) -> void;
    auto OP_ANNN(Nibbles nibbles) -> void;
    auto OP_BNNN(Nibbles nibbles) -> void;
    auto OP_CXNN(Nibbles nibbles) -> void;
    auto OP_DXYN(Nibbles nibbles) -> void;
    auto OP_EX9E(const std::uint8_t& VX) -> void;
    auto OP_EXA1(const std::uint8_t& VX) -> void;
    auto OP_FX07(std::uint8_t& VX) const -> void;
    auto OP_FX15(const std::uint8_t& VX) -> void;
    auto OP_FX18(const std::uint8_t& VX) -> void;
    auto OP_FX1E(const std::uint8_t& VX) -> void;
    static auto OP_FX0A(std::uint8_t& VX) -> void;
    auto OP_FX29() -> void;
    auto OP_FX33(Nibbles nibbles) -> void;
    auto OP_FX55(Nibbles nibbles) -> void;
    auto OP_FX65(Nibbles nibbles) -> void;

    [[nodiscard]] static auto get_value_char_to_key_map(int key) -> std::uint8_t;
    [[nodiscard]] static auto get_nibbles(std::uint16_t instruction) -> Nibbles;

    [[nodiscard]]static auto get_number_NN(Nibbles nibbles) -> std::uint8_t;
    [[nodiscard]]static auto get_number_NNN(Nibbles nibbles) -> std::uint16_t;

private:
    std::array<std::uint8_t, 16> m_registers{};
    std::uint16_t m_index_register{};
    std::uint16_t m_program_counter{};

    std::stack<std::uint16_t> m_stack{}; //16 elements or more, doesnt matter
    std::uint8_t m_stack_ptr{};

    std::uint8_t m_delay_timer{};
    std::uint8_t m_sound_timer{};

    std::array<std::uint8_t, 4096> m_memory{};

    std::array<bool, 2048> m_display{}; //64x32 Pixels
};

#endif //MAIN_H






