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
        I_00E0 = 0, I_00EE = 1,
        I_1NNN = 2,
        I_2NNN = 3,
        I_3XNN = 4,
        I_4XNN = 5,
        I_5XY0 = 6,
        I_6XNN = 7,
        I_7XNN = 8,
        I_8XY0 = 9, I_8XY1 = 10, I_8XY2 = 11, I_8XY3 = 12, I_8XY4 = 13,
        I_8XY5 = 14, I_8XY7 = 15, I_8XY6 = 16, I_8XYE = 17,
        I_9XY0 = 18,
        I_ANNN = 19,
        I_BNNN = 20,
        I_CXNN = 21,
        I_DXYN = 22,
        I_EX9E = 23, I_EXA1 = 24,
        I_FX07 = 25, I_FX15 = 26, I_FX18 = 27, I_FX1E = 28, I_FX0A = 29,
        I_FX29 = 30, I_FX33 = 31, I_FX55 = 32, I_FX65 = 33,
        UNINITIALIZED = 34,
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

    struct Keypress
    {
        std::atomic_bool is_pressed{false};
        std::chrono::time_point<std::chrono::system_clock> start_time;
    };

    struct Nibbles
    {
        std::uint8_t first_nibble;
        std::uint8_t second_nibble;
        std::uint8_t third_nibble;
        std::uint8_t fourth_nibble;
    };

    static inline const std::unordered_map<int, Keymap> CHAR_TO_KEYMAP{
            {49, Keymap::K_1}, {50, Keymap::K_2}, {51, Keymap::K_3}, {52, Keymap::K_4},
            {113, Keymap::K_Q}, {119, Keymap::K_W}, {101, Keymap::K_E}, {114, Keymap::K_R},
            {97, Keymap::K_A}, {115, Keymap::K_S}, {100, Keymap::K_D}, {102, Keymap::K_F},
            {121, Keymap::K_Y}, {120, Keymap::K_X}, {99, Keymap::K_C}, {118, Keymap::K_V},
        };

    static constexpr std::array<uint8_t, 80> FONTS{
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

    static constexpr std::uint16_t START_ADDRESS{0x200};
    static constexpr int FONTSET_START_ADDRESS{0x50};
    static constexpr int DISPLAY_WIDTH{64};
    static constexpr int DISPLAY_HEIGHT{32};

    Chip8();

    auto read_rom(const std::filesystem::path& file_path) -> void;
    auto main_loop(int clock_speed_ms) -> void;
    auto update_timer() -> void;

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
    auto OP_3XNN(Nibbles nibbles) -> void;
    auto OP_4XNN(Nibbles nibbles) -> void;
    auto OP_5XY0(Nibbles nibbles) -> void;
    auto OP_9XY0(Nibbles nibbles) -> void;
    auto OP_6XNN(Nibbles nibbles) -> void;
    auto OP_7XNN(Nibbles nibbles) -> void;
    auto OP_8XY0(Nibbles nibbles) -> void;
    auto OP_8XY1(Nibbles nibbles) -> void;
    auto OP_8XY2(Nibbles nibbles) -> void;
    auto OP_8XY3(Nibbles nibbles) -> void;
    auto OP_8XY4(Nibbles nibbles) -> void;
    auto OP_8XY5(Nibbles nibbles) -> void;
    auto OP_8XY7(Nibbles nibbles) -> void;
    auto OP_8XY6(Nibbles nibbles) -> void;
    auto OP_8XYE(Nibbles nibbles) -> void;
    auto OP_ANNN(Nibbles nibbles) -> void;
    auto OP_BNNN(Nibbles nibbles) -> void;
    auto OP_CXNN(Nibbles nibbles) -> void;
    auto OP_DXYN(Nibbles nibbles) -> void;
    auto OP_EX9E(Nibbles nibbles) -> void;
    auto OP_EXA1(Nibbles nibbles) -> void;
    auto OP_FX07(Nibbles nibbles) -> void;
    auto OP_FX15(Nibbles nibbles) -> void;
    auto OP_FX18(Nibbles nibbles) -> void;
    auto OP_FX1E(Nibbles nibbles) -> void;
    auto OP_FX0A(Nibbles nibbles) -> void;
    auto OP_FX29(Nibbles nibbles) -> void;
    auto OP_FX33(Nibbles nibbles) -> void;
    auto OP_FX55(Nibbles nibbles) -> void;
    auto OP_FX65(Nibbles nibbles) -> void;

    auto draw_display() const -> void;
    auto user_input_thread() -> void;

    [[nodiscard]]static auto get_value_char_to_key_map(int key) -> std::uint8_t;
    [[nodiscard]]static auto get_nibbles(std::uint16_t instruction) -> Nibbles;

    [[nodiscard]]static auto get_number_NN(Nibbles nibbles) -> std::uint8_t;
    [[nodiscard]]static auto get_number_NNN(Nibbles nibbles) -> std::uint16_t;

protected:
    std::atomic_bool m_run = true;

    std::array<std::uint8_t, 16> m_registers{};
    std::uint16_t m_index_register{};
    std::uint16_t m_program_counter{};

    std::stack<std::uint16_t> m_stack{};
    //std::uint8_t m_stack_ptr{}; //unnecesary with this implementation

    std::uint8_t m_delay_timer{};
    std::uint8_t m_sound_timer{};

    std::array<Keypress, 16> m_keymap{};

    std::array<std::uint8_t, 4096> m_memory{};
    std::array<bool, DISPLAY_WIDTH * DISPLAY_HEIGHT> m_display{};
};

class COSMAC_VIP: public Chip8
{

};

class CHIP_48: public Chip8
{

};

#endif //MAIN_H






