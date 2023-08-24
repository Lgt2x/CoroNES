#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <cstdint>
#include <string>
#include <utility>

#include "doctest.h"
#include "helpers/TestFixture.h"

TEST_CASE("CPU reset sets the program counter to the reset vector") {
  auto mapper = std::make_unique<DummyMapper>();
  auto bus = std::make_unique<Bus>(mapper.get());

  // Set reset vector
  uint16_t startAddress = 0x800;
  bus->writeByte(0xFFFC, startAddress & 0xFF);
  bus->writeByte(0xFFFD, startAddress >> 8);

  // Reset CPU
  auto cpu = std::make_unique<CPU_6502>(bus.get());
  cpu->reset();

  // PC should be set to reset vector
  CHECK(cpu->dumpRegisters().PC == 0x800);
}

TEST_CASE("CPU supports all Adressing modes") {
  // SUBCASE("X Pre-indexed indirect addressing") {
  //   auto fixture = TestFixture::setupTest({"LDX #$11", "LDA ($24,X)"},
  //   0x800);

  //   // X = $11, so $24+$11 = $35. The address read by the operation
  //   // is located in the zero-page at ($36 $35).
  //   fixture.bus->writeByte(0x35, 0x67);
  //   fixture.bus->writeByte(0x36, 0x02);
  //   fixture.bus->writeByte(0x0267, 0x56);

  //   CHECK(fixture.cpu->dumpRegisters().A == 0x00);
  //   fixture.cpu->step();
  //   CHECK(fixture.cpu->dumpRegisters().A == 0x56);
  // }

  SUBCASE("Zero-page addressing") {
    auto fixture = TestFixture::setupTest({"LDA $22"}, 0x800);
    fixture.bus->writeByte(0x22, 0x30);

    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters().A == 0x30);
  }

  SUBCASE("Immediate addressing") {
    auto fixture = TestFixture::setupTest({"LDA #$0A"}, 0x800);

    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters().A == 0x0A);
  }

  SUBCASE("Absolute addressing") {
    auto fixture = TestFixture::setupTest({"LDA $1234"}, 0x800);
    fixture.bus->writeByte(0x1234, 0x30);

    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters().A == 0x30);
  }

  // SUBCASE("Y Indirect post-indexed") {
  //   auto fixture = TestFixture::setupTest({"LDY #$11", "LDA ($12),Y"},
  //   0x800); fixture.bus->writeByte(0x12, 0x65); fixture.bus->writeByte(0x13,
  //   0x02); fixture.bus->writeByte(0x0276, 0x30);

  //   // $265 indexed by Y=$11 gives address $276, loading this byte in A
  //   fixture.cpu->step();
  //   CHECK(fixture.cpu->dumpRegisters().A == 0x30);
  // }

  // SUBCASE("X-indexed Zero-page addressing") {
  //   auto fixture = TestFixture::setupTest({"LDX #$11", "LDA $22,X"}, 0x800);
  //   fixture.bus->writeByte(0x33, 0x30);

  //   fixture.cpu->step();
  //   CHECK(fixture.cpu->dumpRegisters().A == 0x30);
  // }

  // SUBCASE("Absolute indexed by X addressing") {
  //   auto fixture = TestFixture::setupTest({"LDX #$11", "LDA $0230,X"},
  //   0x800); fixture.bus->writeByte(0x0241, 0x56);

  //   fixture.cpu->step();
  //   CHECK(fixture.cpu->dumpRegisters().A == 0x56);
  // }

  // SUBCASE("Absolute indexed by Y addressing") {
  //   auto fixture = TestFixture::setupTest({"LDY #$11", "LDA $0230,Y"},
  //   0x800); fixture.bus->writeByte(0x0241, 0x56);

  //   fixture.cpu->step();
  //   CHECK(fixture.cpu->dumpRegisters().A == 0x56);
  // }
}

enum flags { N_f, V_f, B_f, D_f, I_f, Z_f, C_f };

TEST_CASE("CPU supports all 6502 opcodes") {
  SUBCASE("ORA") {
    auto fixture = TestFixture::setupTest(
        {
            "LDA #%10100110",
            "STA $20",
            "LDA#%11001100",
            "ORA $20",
        },
        0x800);

    //    10100110
    // OR 11001100
    //  = 11101110

    fixture.cpu->step(4);
    CHECK(fixture.cpu->dumpRegisters().A == 0b11101110);
  }

  SUBCASE("AND") {
    auto fixture = TestFixture::setupTest(
        {
            "LDA #%10100110",
            "STA $20",
            "LDA#%11001100",
            "AND $20",
        },
        0x800);

    //     10100110
    // AND 11001100
    //   = 10000100

    fixture.cpu->step(4);
    CHECK(fixture.cpu->dumpRegisters().A == 0b10000100);
  }

  SUBCASE("EOR") {
    auto fixture = TestFixture::setupTest(
        {
            "LDA #%10100110",
            "STA $20",
            "LDA#%11001100",
            "EOR $20",
        },
        0x800);

    //     10100110
    // EOR 11001100
    //   = 01101010

    fixture.cpu->step(4);
    CHECK(fixture.cpu->dumpRegisters().A == 0b01101010);
  }

  SUBCASE("ADC") {
    auto fixture = TestFixture::setupTest(
        {
            "LDA #%10100110",
            "STA $20",
            "LDA#%11001100",
            "ADC $20",
        },
        0x800);

    //      10100110
    // +    11001100
    //   = 101110010

    fixture.cpu->step(4);
    CHECK(fixture.cpu->dumpRegisters().A == 0b01110010);
  }

  SUBCASE("STA") {
    auto fixture = TestFixture::setupTest(
        {
            "LDA #$0A",
            "STA $20",
        },
        0x800);
    fixture.cpu->step();
    fixture.cpu->step();
    CHECK(fixture.bus->readByte(0x20) == 0x0A);
  }

  SUBCASE("LDA") {
    auto fixture = TestFixture::setupTest(
        {
            "LDA #$0A",
            "LDA #$FE",
            "LDA #$00",
        },
        0x800);

    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters().A == 0x0A);

    // Check flags
    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters().A == 0xFE);
    CHECK(fixture.cpu->dumpRegisters().flags[N_f]);

    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters().A == 0x00);
    CHECK(fixture.cpu->dumpRegisters().flags[Z_f]);
  }

  SUBCASE("CMP") {
    SUBCASE("CMP in equality case ") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #$3E",
              "STA $20",
              "CMP $20",
          },
          0x800);
      fixture.cpu->step(3);
      CHECK(fixture.cpu->dumpRegisters().flags[Z_f]);
      CHECK(fixture.cpu->dumpRegisters().flags[C_f]);
    }

    SUBCASE("CMP in superiority case ") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #$02",
              "STA $20",
              "LDA #$0E",
              "CMP $20",
          },
          0x800);
      fixture.cpu->step(4);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
      CHECK(fixture.cpu->dumpRegisters().flags[C_f]);
    }

    SUBCASE("CMP in inferiority case ") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #$5E",
              "STA $20",
              "LDA #$33",
              "CMP $20",
          },
          0x800);
      fixture.cpu->step(4);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[C_f]);
    }
  }

  SUBCASE("SBC") {
    auto fixture = TestFixture::setupTest(
        {
            "LDA #%10100110",
            "STA $20",
            "LDA#%11001100",
            "SBC $20",
        },
        0x800);

    //     11001100
    // -   10100110
    //   = 00100110

    fixture.cpu->step(4);
    CHECK(fixture.cpu->dumpRegisters().A == 0b100110);
  }

  SUBCASE("ASL") {
    SUBCASE("Shift left, set carry") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #%10100110",
              "ASL",
          },
          0x800);

      //     10100110 Shift left once
      // 1 | 01001100

      fixture.cpu->step(2);
      CHECK(fixture.cpu->dumpRegisters().A == 0b01001100);
      CHECK(fixture.cpu->dumpRegisters().flags[C_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
    }
    SUBCASE("Shift left memory, set negative flag") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #%01100110",
              "STA $20",
              "ASL $20",
          },
          0x800);

      //     01100110 Shift left once
      // 0 | 11001100

      fixture.cpu->step(3);
      CHECK(fixture.bus->readByte(0x20) == 0b11001100);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[C_f]);
      CHECK(fixture.cpu->dumpRegisters().flags[N_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
    }
  }

  SUBCASE("ROL") {
    SUBCASE("Rotate Accumulator left, set carry") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #%10100110",
              "ROL",
          },
          0x800);

      //     10100110 Shifted left once
      // 1 | 01001100

      fixture.cpu->step(2);
      CHECK(fixture.cpu->dumpRegisters().A == 0b01001100);
      CHECK(fixture.cpu->dumpRegisters().flags[C_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
    }
    SUBCASE("Rotate memory left, set negative flag") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #%01100110",
              "STA $20",
              "ROL $20",
          },
          0x800);

      //     1100110 Shifted left once
      // 0 | 11001100

      fixture.cpu->step(3);
      CHECK(fixture.bus->readByte(0x20) == 0b11001100);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[C_f]);
      CHECK(fixture.cpu->dumpRegisters().flags[N_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
    }
    SUBCASE("Rotate accumulator left, shift carry in") {
      auto fixture = TestFixture::setupTest(
          {
              "SEC",
              "LDA #%01100110",
              "STA $20",
              "ROL $20",
          },
          0x800);
      // TODO : SEC

      //     01100110 shift left, insert carry = 1
      // 0 | 11001101

      // fixture.cpu->step(4);
      // CHECK_FALSE(fixture.cpu->dumpRegisters().flags[C_f]);
      // CHECK(fixture.bus->readByte(0x20) == 0b11001101);
    }
  }
  SUBCASE("LSR") {
    SUBCASE("Shift accumulator right, set carry") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #%10100101",
              "LSR",
          },
          0x800);

      //    10100101     Shift right once
      // >> 01010010 | 1

      fixture.cpu->step(2);
      CHECK(fixture.cpu->dumpRegisters().A == 0b01010010);
      CHECK(fixture.cpu->dumpRegisters().flags[C_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
    }
    SUBCASE("Shift memory right") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #%01100110",
              "STA $20",
              "LSR $20",
          },
          0x800);

      //    01100110      Shift right once
      // >> 00110011 | 0

      fixture.cpu->step(3);
      CHECK(fixture.bus->readByte(0x20) == 0b00110011);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[C_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
    }
  }

  SUBCASE("ROR") {
    SUBCASE("Rotate accumulator right, set carry") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #%10100101",
              "ROR",
          },
          0x800);

      //    10100101     Shift right once
      // >> 01010010 | 1

      fixture.cpu->step(2);
      CHECK(fixture.cpu->dumpRegisters().A == 0b01010010);
      CHECK(fixture.cpu->dumpRegisters().flags[C_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
    }
    SUBCASE("Shift memory right") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #%01100110",
              "STA $20",
              "ROR $20",
          },
          0x800);

      //    01100110      Shift right once
      // >> 00110011 | 0

      fixture.cpu->step(3);
      CHECK(fixture.bus->readByte(0x20) == 0b00110011);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[C_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
    }
    SUBCASE("Rotate accumulator right, shift carry in") {
      auto fixture = TestFixture::setupTest(
          {
              "SEC",
              "LDA #%01100110",
              "STA $20",
              "ROR $20",
          },
          0x800);
      // TODO : SEC

      // 01100110     shift right, insert carry = 1
      // 10110011 | 0

      // fixture.cpu->step(4);
      // CHECK_FALSE(fixture.cpu->dumpRegisters().flags[C_f]);
      // CHECK(fixture.cpu->dumpRegisters().flags[N_f]);
      // CHECK(fixture.bus->readByte(0x20) == 0b10110011);
    }
  }
  SUBCASE("STX") {
    SUBCASE("Store X zero-page") {
      auto fixture = TestFixture::setupTest(
          {
              "LDX #$CA",
              "STX $12",
          },
          0x800);
      fixture.cpu->step(2);
      CHECK(fixture.bus->readByte(0x12) == 0xCA);
    }
    SUBCASE("Store X absolute") {
      auto fixture = TestFixture::setupTest(
          {
              "LDX #$CA",
              "STX $1445",
          },
          0x800);
      fixture.cpu->step(2);
      CHECK(fixture.bus->readByte(0x1445) == 0xCA);
    }
    // SUBCASE("Load X zero-page Y indexed") {
    //   auto fixture = TestFixture::setupTest(
    //       {
    //           "LDA #$CA",
    //           "LDY #$02",
    //           "STA 0x14",
    //           "LDX 0x12,Y",
    //       },
    //       0x800);
    //   fixture.cpu->step(4);
    //   CHECK(fixture.cpu->dumpRegisters().X == 0xCA);
    // }
  }
  SUBCASE("TXA") {
    auto fixture = TestFixture::setupTest(
        {
            "LDX #$CA",
            "TXA",
        },
        0x800);
    fixture.cpu->step(2);
    CHECK(fixture.cpu->dumpRegisters().A == 0xCA);
  }
  SUBCASE("TXS") {
    auto fixture = TestFixture::setupTest(
        {
            "LDX #$CA",
            "TXS",
        },
        0x800);
    fixture.cpu->step(2);
    CHECK(fixture.cpu->dumpRegisters().SP == 0xCA);
  }
  SUBCASE("LDX") {
    SUBCASE("Load X immediate") {
      auto fixture = TestFixture::setupTest(
          {
              "LDX #$45",
          },
          0x800);
      fixture.cpu->step(1);
      CHECK(fixture.cpu->dumpRegisters().X == 0x45);
    }
    SUBCASE("Load X absolute") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #$CA",
              "STA 0x1445",
              "LDX 0x1445",
          },
          0x800);
      fixture.cpu->step(3);
      CHECK(fixture.cpu->dumpRegisters().X == 0xCA);
    }
    // SUBCASE("Load X absolute, Y-indexed") {
    //   auto fixture = TestFixture::setupTest(
    //       {
    //           "LDA #$CA",
    //           "LDY #$05",
    //           "STA 0x1450",
    //           "LDX 0x1445,Y",
    //       },
    //       0x800);
    //   fixture.cpu->step(3);
    //   CHECK(fixture.cpu->dumpRegisters().X == 0xCA);
    // }
    SUBCASE("Load X zero-page") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #$CA",
              "STA 0x12",
              "LDX 0x12",
          },
          0x800);
      fixture.cpu->step(3);
      CHECK(fixture.cpu->dumpRegisters().X == 0xCA);
    }
    // SUBCASE("Load X zero-page Y indexed") {
    //   auto fixture = TestFixture::setupTest(
    //       {
    //           "LDA #$CA",
    //           "LDY #$02",
    //           "STA 0x14",
    //           "LDX 0x12,Y",
    //       },
    //       0x800);
    //   fixture.cpu->step(4);
    //   CHECK(fixture.cpu->dumpRegisters().X == 0xCA);
    // }
    SUBCASE("Load X, set flags") {
      auto fixture = TestFixture::setupTest(
          {
              "LDX #$F0",
              "LDX #$00",
          },
          0x800);
      fixture.cpu->step();
      CHECK(fixture.cpu->dumpRegisters().flags[N_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
      fixture.cpu->step();
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
      CHECK(fixture.cpu->dumpRegisters().flags[Z_f]);
    }
  }
  SUBCASE("TAX") {
    auto fixture = TestFixture::setupTest(
        {
            "LDA #$CA",
            "TAX",
        },
        0x800);
    fixture.cpu->step(2);
    CHECK(fixture.cpu->dumpRegisters().X == 0xCA);
  }
  SUBCASE("TSX") {
    auto fixture = TestFixture::setupTest(
        {
            "TSX",
        },
        0x800);
    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters().X == 0xFD);
  }

  SUBCASE("DEC") {
    SUBCASE("Decrement zero page") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #$20",
              "STA $10",
              "DEC $10",
          },
          0x800);
      fixture.cpu->step(3);
      CHECK(fixture.bus->readByte(0x10) == 0x1f);
    }
    SUBCASE("Decrement X register") {
      auto fixture = TestFixture::setupTest(
          {
              "LDX #$20",
              "DEX",
          },
          0x800);
      fixture.cpu->step(2);
      CHECK(fixture.cpu->dumpRegisters().X == 0x1f);
    }
    SUBCASE("Decrement absolute address") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #$20",
              "STA $1045",
              "DEC $1045",

          },
          0x800);
      fixture.cpu->step(3);
      CHECK(fixture.bus->readByte(0x1045) == 0x1f);
    }
    SUBCASE("Decrement zero page X indexed") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #$20",
              "LDX #$03",
              "STA $11",
              "DEC $0E,X",

          },
          0x800);
      fixture.cpu->step(4);
      CHECK(fixture.bus->readByte(0x11) == 0x1f);
    }
    SUBCASE("Decrement absolute X indexed") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #$22",
              "LDX #$03",
              "STA $1134",
              "DEC $1131,X",

          },
          0x800);
      fixture.cpu->step(4);
      CHECK(fixture.bus->readByte(0x1134) == 0x21);
    }
    SUBCASE("Decrement, set flags") {
      auto fixture = TestFixture::setupTest(
          {
              "LDX #$82",
              "DEX",
              "LDX #$01",
              "DEX",

          },
          0x800);
      fixture.cpu->step(2);
      CHECK(fixture.cpu->dumpRegisters().flags[N_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);

      fixture.cpu->step(2);
      CHECK(fixture.cpu->dumpRegisters().flags[Z_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
    } // TODO : test overflow
  }
  SUBCASE("INC") {
    SUBCASE("Increment zero page") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #$20",
              "STA $10",
              "INC $10",
          },
          0x800);
      fixture.cpu->step(3);
      CHECK(fixture.bus->readByte(0x10) == 0x21);
    }
    SUBCASE("Increment absolute address") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #$20",
              "STA $1045",
              "INC $1045",

          },
          0x800);
      fixture.cpu->step(3);
      CHECK(fixture.bus->readByte(0x1045) == 0x21);
    }
    SUBCASE("Increment zero page X indexed") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #$20",
              "LDX #$03",
              "STA $11",
              "INC $0E,X",

          },
          0x800);
      fixture.cpu->step(4);
      CHECK(fixture.bus->readByte(0x11) == 0x21);
    }
    SUBCASE("Increment absolute X indexed") {
      auto fixture = TestFixture::setupTest(
          {
              "LDA #$20",
              "LDX #$03",
              "STA $1134",
              "INC $1131,X",

          },
          0x800);
      fixture.cpu->step(4);
      CHECK(fixture.bus->readByte(0x1134) == 0x21);
    }
    SUBCASE("Increment, set flags") {
      auto fixture = TestFixture::setupTest(
          {
              "LDX #$7F",
              "STX $20",
              "INC $20",
              "LDX #$FF",
              "STX $20",
              "INC $20",

          },
          0x800);
      fixture.cpu->step(3);
      CHECK(fixture.cpu->dumpRegisters().flags[N_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);

      fixture.cpu->step(3);
      CHECK(fixture.cpu->dumpRegisters().flags[Z_f]);
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
    } // TODO : test overflow
  }
}