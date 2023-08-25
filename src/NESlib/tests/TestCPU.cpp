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
  //   auto fixture = TestFixture::setupTest({"LDX #$11", "LDA ($24,X)"}
  //   );

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
    auto fixture = TestFixture::setupTest({"LDA $22"});
    fixture.bus->writeByte(0x22, 0x30);

    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters().A == 0x30);
  }

  SUBCASE("Immediate addressing") {
    auto fixture = TestFixture::setupTestAndExecute({"LDA #$0A"});

    CHECK(fixture.cpu->dumpRegisters().A == 0x0A);
  }

  SUBCASE("Absolute addressing") {
    auto fixture = TestFixture::setupTest({"LDA $1234"});
    fixture.bus->writeByte(0x1234, 0x30);

    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters().A == 0x30);
  }

  // SUBCASE("Y Indirect post-indexed") {
  //   auto fixture = TestFixture::setupTest({"LDY #$11", "LDA ($12),Y"},
  //   ); fixture.bus->writeByte(0x12, 0x65); fixture.bus->writeByte(0x13,
  //   0x02); fixture.bus->writeByte(0x0276, 0x30);

  //   // $265 indexed by Y=$11 gives address $276, loading this byte in A
  //   fixture.cpu->step();
  //   CHECK(fixture.cpu->dumpRegisters().A == 0x30);
  // }

  // SUBCASE("X-indexed Zero-page addressing") {
  //   auto fixture = TestFixture::setupTest({"LDX #$11", "LDA $22,X"});
  //   fixture.bus->writeByte(0x33, 0x30);

  //   fixture.cpu->step();
  //   CHECK(fixture.cpu->dumpRegisters().A == 0x30);
  // }

  // SUBCASE("Absolute indexed by X addressing") {
  //   auto fixture = TestFixture::setupTest({"LDX #$11", "LDA $0230,X"},
  //   ); fixture.bus->writeByte(0x0241, 0x56);

  //   fixture.cpu->step();
  //   CHECK(fixture.cpu->dumpRegisters().A == 0x56);
  // }

  // SUBCASE("Absolute indexed by Y addressing") {
  //   auto fixture = TestFixture::setupTest({"LDY #$11", "LDA $0230,Y"},
  //   ); fixture.bus->writeByte(0x0241, 0x56);

  //   fixture.cpu->step();
  //   CHECK(fixture.cpu->dumpRegisters().A == 0x56);
  // }
}

enum flags { N_f, V_f, B_f, D_f, I_f, Z_f, C_f };

TEST_CASE("CPU supports all 6502 opcodes") {
  SUBCASE("Opcodes ending by 0b00") {
    SUBCASE("CLC & SEC") {
      auto fixture = TestFixture::setupTest({
          "SEC",
          "CLC",
          "SEC",
      });

      fixture.cpu->step();
      CHECK(fixture.cpu->dumpRegisters().flags[C_f]);
      fixture.cpu->step();
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[C_f]);
      fixture.cpu->step();
      CHECK(fixture.cpu->dumpRegisters().flags[C_f]);
    }

    SUBCASE("CLI & SEI") {
      auto fixture = TestFixture::setupTest({
          "SEI",
          "CLI",
          "SEI",
      });

      fixture.cpu->step();
      CHECK(fixture.cpu->dumpRegisters().flags[I_f]);
      fixture.cpu->step();
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[I_f]);
      fixture.cpu->step();
      CHECK(fixture.cpu->dumpRegisters().flags[I_f]);
    }

    SUBCASE("CLV") {
      auto fixture = TestFixture::setupTest({
          "LDA #$40",
          "STA $20",
          "ADC $20", // This operation sets the overflow flag
          "CLV",
      });

      fixture.cpu->step(3);
      CHECK(fixture.cpu->dumpRegisters().flags[V_f]);
      fixture.cpu->step();
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[V_f]);
    }

    SUBCASE("CLD & SED") {
      auto fixture = TestFixture::setupTest({
          "SED",
          "CLD",
          "SED",
      });

      fixture.cpu->step();
      CHECK(fixture.cpu->dumpRegisters().flags[D_f]);
      fixture.cpu->step();
      CHECK_FALSE(fixture.cpu->dumpRegisters().flags[D_f]);
      fixture.cpu->step();
      CHECK(fixture.cpu->dumpRegisters().flags[D_f]);
    }

    SUBCASE("TYA") {
      auto fixture = TestFixture::setupTest({
          "LDA #$20",
          "STA $10",
          "LDY $10",
          "LDA #$10",
          "TYA",
      });

      // TODO
      // fixture.cpu->step(4);
      // CHECK(fixture.cpu->dumpRegisters().A == 0x10);
      // fixture.cpu->step();
      // CHECK(fixture.cpu->dumpRegisters().A == 0x20);
    }

    SUBCASE("TAY") {
      auto fixture = TestFixture::setupTestAndExecute({
          "LDA #$10",
          "TAY",
      });

      CHECK(fixture.cpu->dumpRegisters().Y == 0x10);
    }

    SUBCASE("DEY") {
      auto fixture = TestFixture::setupTestAndExecute({
          "LDA #$10",
          "TAY",
          "DEY",
      });

      CHECK(fixture.cpu->dumpRegisters().Y == 0x0F);
      // TODO : test overflow / underflow, flags
    }

    SUBCASE("INY") {
      auto fixture = TestFixture::setupTestAndExecute({
          "LDA #$10",
          "TAY",
          "INY",
      });

      CHECK(fixture.cpu->dumpRegisters().Y == 0x11);
    }

    SUBCASE("INX") {
      auto fixture = TestFixture::setupTestAndExecute({
          "LDX #$10",
          "INX",
      });

      CHECK(fixture.cpu->dumpRegisters().X == 0x11);
    }

    SUBCASE("Branch instructions") {
      SUBCASE("Branch equal with Positive offset") {
        auto fixture = TestFixture::setupTest({
            "LDA #$20",
            "STA $10",
            "CMP $10",
            "BEQ $04",
        });

        fixture.cpu->step(3);
        CHECK(fixture.cpu->dumpRegisters().PC == 0x806);
        CHECK(fixture.cpu->dumpRegisters().flags[Z_f]);
        fixture.cpu->step();
        CHECK(fixture.cpu->dumpRegisters().PC == 0x80C);
      }

      SUBCASE("Branch with negative offset") {
        auto fixture = TestFixture::setupTest({
            "LDA #$20", "STA $10", "CMP $10",
            "BEQ %11111100", // -4
        });

        fixture.cpu->step(3);
        CHECK(fixture.cpu->dumpRegisters().PC == 0x806);
        CHECK(fixture.cpu->dumpRegisters().flags[Z_f]);
        fixture.cpu->step();
        CHECK(fixture.cpu->dumpRegisters().PC == 0x804);
      }

      SUBCASE("Branch with null offset (=NOP)") {
        auto fixture = TestFixture::setupTest({
            "LDA #$20",
            "STA $10",
            "CMP $10",
            "BEQ $00",
        });

        fixture.cpu->step(3);
        CHECK(fixture.cpu->dumpRegisters().PC == 0x806);
        CHECK(fixture.cpu->dumpRegisters().flags[Z_f]);
        fixture.cpu->step();
        CHECK(fixture.cpu->dumpRegisters().PC == 0x808);
      }

      SUBCASE("BPL jump") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$20",
            "BPL $02",
            "LDA #$12",
            "LDA #$15",
        });

        fixture.cpu->step(3);
        CHECK(fixture.cpu->dumpRegisters().A == 0x15);
      }

      SUBCASE("BPL no jump") {
        auto fixture = TestFixture::setupTest({
            "LDA #$80",
            "BPL $02",
            "LDA #$12",
            "LDA #$15",
        });

        fixture.cpu->step(3);
        CHECK(fixture.cpu->dumpRegisters().A == 0x12);
      }

      SUBCASE("BMI jump") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$80",
            "BMI $02",
            "LDA #$FF",
            "NOP",
        });

        fixture.cpu->step(3);
        CHECK(fixture.cpu->dumpRegisters().A == 0x80);
      }

      SUBCASE("BVC jump") {
        auto fixture = TestFixture::setupTestAndExecute({
            "CLV",
            "LDA #$20",
            "BVC $02",
            "LDA #$FF",
            "NOP",
        });

        fixture.cpu->step(5);
        CHECK(fixture.cpu->dumpRegisters().A == 0x20);
      }

      SUBCASE("BVS jump") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$40",
            "STA $20",
            "ADC $20", // This operation sets the overflow flag
            "BVS $02",
            "LDA #$FF",
            "NOP",
        });

        fixture.cpu->step(5);
        CHECK(fixture.cpu->dumpRegisters().A == 0x80);
      }

      SUBCASE("BCC jump") {
        auto fixture = TestFixture::setupTestAndExecute({
            "CLC",
            "LDA #$20",
            "BCC $02",
            "LDA #$FF",
            "NOP",
        });

        fixture.cpu->step(4);
        CHECK(fixture.cpu->dumpRegisters().A == 0x20);
      }

      SUBCASE("BCS jump") {
        auto fixture = TestFixture::setupTestAndExecute({
            "SEC",
            "LDA #$20",
            "BCS $02",
            "LDA #$FF",
            "NOP",
        });

        fixture.cpu->step(4);
        CHECK(fixture.cpu->dumpRegisters().A == 0x20);
      }

      SUBCASE("BNE jump") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$20",
            "CMP $00",
            "BNE $02",
            "LDA #$FF",
            "NOP",
        });

        fixture.cpu->step(4);
        CHECK(fixture.cpu->dumpRegisters().A == 0x20);
      }
    }

    SUBCASE("Stack operations") {
      SUBCASE("PHA") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$20",
            "PHA",
        });

        CHECK(fixture.cpu->dumpRegisters().A == 0x20);
        CHECK(fixture.cpu->dumpRegisters().SP == 0xFC);
        CHECK(fixture.bus->readByte(0xFD) == 0x20);
      }

      SUBCASE("PLA") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$20",
            "PHA",
            "LDA #$40",
            "PLA",
        });

        CHECK(fixture.cpu->dumpRegisters().A == 0x20);
        CHECK(fixture.cpu->dumpRegisters().SP == 0xFD);
      }

      SUBCASE("PHP") {
        auto fixture = TestFixture::setupTestAndExecute({
            "PHP",
        });

        CHECK(fixture.cpu->dumpRegisters().SP == 0xFC);
        CHECK(fixture.bus->readByte(0xFD) == 0x34); // CPU status startup value
      }

      SUBCASE("PLP") {
        auto fixture = TestFixture::setupTest({
            "LDA #$00", // set 0 flag
            "PHP",
            "LDA #$80", // set negative flag
            "PLP",
        });

        fixture.cpu->step();
        CHECK(fixture.cpu->dumpRegisters().flags[Z_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);

        fixture.cpu->step(2);
        CHECK(fixture.cpu->dumpRegisters().SP == 0xFC);
        CHECK(fixture.cpu->dumpRegisters().flags[N_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);

        fixture.cpu->step();
        CHECK(fixture.cpu->dumpRegisters().SP == 0xFD);
        CHECK(fixture.cpu->dumpRegisters().flags[Z_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
      }
    }
  }
  SUBCASE("Opcodes ending by 0b01") {
    SUBCASE("ORA") {
      auto fixture = TestFixture::setupTestAndExecute({
          "LDA #%10100110",
          "STA $20",
          "LDA#%11001100",
          "ORA $20",
      });

      //    10100110
      // OR 11001100
      //  = 11101110

      CHECK(fixture.cpu->dumpRegisters().A == 0b11101110);
    }

    SUBCASE("AND") {
      auto fixture = TestFixture::setupTestAndExecute({
          "LDA #%10100110",
          "STA $20",
          "LDA#%11001100",
          "AND $20",
      });

      //     10100110
      // AND 11001100
      //   = 10000100

      CHECK(fixture.cpu->dumpRegisters().A == 0b10000100);
    }

    SUBCASE("EOR") {
      auto fixture = TestFixture::setupTestAndExecute({
          "LDA #%10100110",
          "STA $20",
          "LDA#%11001100",
          "EOR $20",
      });

      //     10100110
      // EOR 11001100
      //   = 01101010

      CHECK(fixture.cpu->dumpRegisters().A == 0b01101010);
    }

    SUBCASE("ADC") {
      auto fixture = TestFixture::setupTestAndExecute({
          "LDA #%10100110",
          "STA $20",
          "LDA#%11001100",
          "ADC $20",
      });

      //      10100110
      // +    11001100
      //   = 101110010

      CHECK(fixture.cpu->dumpRegisters().A == 0b01110010);

      // TODO : flags, carry...
    }

    SUBCASE("STA") {
      auto fixture = TestFixture::setupTestAndExecute({
          "LDA #$0A",
          "STA $20",
      });

      CHECK(fixture.bus->readByte(0x20) == 0x0A);
    }

    SUBCASE("LDA") {
      auto fixture = TestFixture::setupTest({
          "LDA #$0A",
          "LDA #$FE",
          "LDA #$00",
      });

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
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$3E",
            "STA $20",
            "CMP $20",
        });

        CHECK(fixture.cpu->dumpRegisters().flags[Z_f]);
        CHECK(fixture.cpu->dumpRegisters().flags[C_f]);
      }

      SUBCASE("CMP in superiority case ") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$02",
            "STA $20",
            "LDA #$0E",
            "CMP $20",
        });
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
        CHECK(fixture.cpu->dumpRegisters().flags[C_f]);
      }

      SUBCASE("CMP in inferiority case ") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$5E",
            "STA $20",
            "LDA #$33",
            "CMP $20",
        });
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[C_f]);
      }
    }

    SUBCASE("SBC") {
      auto fixture = TestFixture::setupTestAndExecute({
          "LDA #%10100110",
          "STA $20",
          "LDA#%11001100",
          "SBC $20",
      });

      //     11001100
      // -   10100110
      //   = 00100110

      CHECK(fixture.cpu->dumpRegisters().A == 0b100110);
    }
  }

  SUBCASE("Instructions ending by 0b11") {
    SUBCASE("ASL") {
      SUBCASE("Shift left, set carry") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #%10100110",
            "ASL",
        });

        //     10100110 Shift left once
        // 1 | 01001100

        CHECK(fixture.cpu->dumpRegisters().A == 0b01001100);
        CHECK(fixture.cpu->dumpRegisters().flags[C_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
      }
      SUBCASE("Shift left memory, set negative flag") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #%01100110",
            "STA $20",
            "ASL $20",
        });

        //     01100110 Shift left once
        // 0 | 11001100

        CHECK(fixture.bus->readByte(0x20) == 0b11001100);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[C_f]);
        CHECK(fixture.cpu->dumpRegisters().flags[N_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
      }
    }

    SUBCASE("ROL") {
      SUBCASE("Rotate Accumulator left, set carry") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #%10100110",
            "ROL",
        });

        //     10100110 Shifted left once
        // 1 | 01001100

        CHECK(fixture.cpu->dumpRegisters().A == 0b01001100);
        CHECK(fixture.cpu->dumpRegisters().flags[C_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
      }
      SUBCASE("Rotate memory left, set negative flag") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #%01100110",
            "STA $20",
            "ROL $20",
        });

        //     1100110 Shifted left once
        // 0 | 11001100

        CHECK(fixture.bus->readByte(0x20) == 0b11001100);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[C_f]);
        CHECK(fixture.cpu->dumpRegisters().flags[N_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
      }
      SUBCASE("Rotate accumulator left, shift carry in") {
        auto fixture = TestFixture::setupTestAndExecute({
            "SEC",
            "LDA #%01100110",
            "STA $20",
            "ROL $20",
        });
        // TODO : SEC

        //     01100110 shift left, insert carry = 1
        // 0 | 11001101

        // CHECK_FALSE(fixture.cpu->dumpRegisters().flags[C_f]);
        // CHECK(fixture.bus->readByte(0x20) == 0b11001101);
      }
    }

    SUBCASE("LSR") {
      SUBCASE("Shift accumulator right, set carry") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #%10100101",
            "LSR",
        });

        //    10100101     Shift right once
        // >> 01010010 | 1

        CHECK(fixture.cpu->dumpRegisters().A == 0b01010010);
        CHECK(fixture.cpu->dumpRegisters().flags[C_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
      }
      SUBCASE("Shift memory right") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #%01100110",
            "STA $20",
            "LSR $20",
        });

        //    01100110      Shift right once
        // >> 00110011 | 0

        CHECK(fixture.bus->readByte(0x20) == 0b00110011);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[C_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
      }
    }

    SUBCASE("ROR") {
      SUBCASE("Rotate accumulator right, set carry") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #%10100101",
            "ROR",
        });

        //    10100101     Shift right once
        // >> 01010010 | 1

        CHECK(fixture.cpu->dumpRegisters().A == 0b01010010);
        CHECK(fixture.cpu->dumpRegisters().flags[C_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
      }
      SUBCASE("Shift memory right") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #%01100110",
            "STA $20",
            "ROR $20",
        });

        //    01100110      Shift right once
        // >> 00110011 | 0

        CHECK(fixture.bus->readByte(0x20) == 0b00110011);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[C_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
      }
      SUBCASE("Rotate accumulator right, shift carry in") {
        auto fixture = TestFixture::setupTestAndExecute({
            "SEC",
            "LDA #%01100110",
            "STA $20",
            "ROR $20",
        });
        // TODO : SEC

        // 01100110     shift right, insert carry = 1
        // 10110011 | 0

        // CHECK_FALSE(fixture.cpu->dumpRegisters().flags[C_f]);
        // CHECK(fixture.cpu->dumpRegisters().flags[N_f]);
        // CHECK(fixture.bus->readByte(0x20) == 0b10110011);
      }
    }

    SUBCASE("STX") {
      SUBCASE("Store X zero-page") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDX #$CA",
            "STX $12",
        });

        CHECK(fixture.bus->readByte(0x12) == 0xCA);
      }
      SUBCASE("Store X absolute") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDX #$CA",
            "STX $1445",
        });
        CHECK(fixture.bus->readByte(0x1445) == 0xCA);
      }
      // SUBCASE("Load X zero-page Y indexed") {
      //   auto fixture = TestFixture::setupTestAndExecute(
      //       {
      //           "LDA #$CA",
      //           "LDY #$02",
      //           "STA 0x14",
      //           "LDX 0x12,Y",
      //       },
      //       );

      //   CHECK(fixture.cpu->dumpRegisters().X == 0xCA);
      // }
    }

    SUBCASE("TXA") {
      auto fixture = TestFixture::setupTestAndExecute({
          "LDX #$CA",
          "TXA",
      });
      CHECK(fixture.cpu->dumpRegisters().A == 0xCA);
    }

    SUBCASE("TXS") {
      auto fixture = TestFixture::setupTestAndExecute({
          "LDX #$CA",
          "TXS",
      });
      CHECK(fixture.cpu->dumpRegisters().SP == 0xCA);
    }

    SUBCASE("LDX") {
      SUBCASE("Load X immediate") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDX #$45",
        });
        CHECK(fixture.cpu->dumpRegisters().X == 0x45);
      }
      SUBCASE("Load X absolute") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$CA",
            "STA 0x1445",
            "LDX 0x1445",
        });
        CHECK(fixture.cpu->dumpRegisters().X == 0xCA);
      }
      // SUBCASE("Load X absolute, Y-indexed") {
      //   auto fixture = TestFixture::setupTestAndExecute(
      //       {
      //           "LDA #$CA",
      //           "LDY #$05",
      //           "STA 0x1450",
      //           "LDX 0x1445,Y",
      //       }
      //       );

      //   CHECK(fixture.cpu->dumpRegisters().X == 0xCA);
      // }
      SUBCASE("Load X zero-page") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$CA",
            "STA 0x12",
            "LDX 0x12",
        });
        CHECK(fixture.cpu->dumpRegisters().X == 0xCA);
      }
      // SUBCASE("Load X zero-page Y indexed") {
      //   auto fixture = TestFixture::setupTestAndExecute(
      //       {
      //           "LDA #$CA",
      //           "LDY #$02",
      //           "STA 0x14",
      //           "LDX 0x12,Y",
      //       }
      //       );
      //   CHECK(fixture.cpu->dumpRegisters().X == 0xCA);
      // }
      SUBCASE("Load X, set flags") {
        auto fixture = TestFixture::setupTest({
            "LDX #$F0",
            "LDX #$00",
        });
        fixture.cpu->step();
        CHECK(fixture.cpu->dumpRegisters().flags[N_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);
        fixture.cpu->step();
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
        CHECK(fixture.cpu->dumpRegisters().flags[Z_f]);
      }
    }

    SUBCASE("TAX") {
      auto fixture = TestFixture::setupTestAndExecute({
          "LDA #$CA",
          "TAX",
      });
      CHECK(fixture.cpu->dumpRegisters().X == 0xCA);
    }

    SUBCASE("TSX") {
      auto fixture = TestFixture::setupTestAndExecute({
          "TSX",
      });
      CHECK(fixture.cpu->dumpRegisters().X == 0xFD);
    }

    SUBCASE("DEC") {
      SUBCASE("Decrement zero page") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$20",
            "STA $10",
            "DEC $10",
        });
        CHECK(fixture.bus->readByte(0x10) == 0x1f);
      }
      SUBCASE("Decrement X register") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDX #$20",
            "DEX",
        });
        CHECK(fixture.cpu->dumpRegisters().X == 0x1f);
      }
      SUBCASE("Decrement absolute address") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$20",
            "STA $1045",
            "DEC $1045",

        });
        CHECK(fixture.bus->readByte(0x1045) == 0x1f);
      }
      SUBCASE("Decrement zero page X indexed") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$20",
            "LDX #$03",
            "STA $11",
            "DEC $0E,X",

        });
        CHECK(fixture.bus->readByte(0x11) == 0x1f);
      }
      SUBCASE("Decrement absolute X indexed") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$22",
            "LDX #$03",
            "STA $1134",
            "DEC $1131,X",

        });
        CHECK(fixture.bus->readByte(0x1134) == 0x21);
      }
      SUBCASE("Decrement, set flags") {
        auto fixture = TestFixture::setupTest({
            "LDX #$82",
            "DEX",
            "LDX #$01",
            "DEX",

        });
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
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$20",
            "STA $10",
            "INC $10",
        });

        CHECK(fixture.bus->readByte(0x10) == 0x21);
      }
      SUBCASE("Increment absolute address") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$20",
            "STA $1045",
            "INC $1045",

        });
        CHECK(fixture.bus->readByte(0x1045) == 0x21);
      }
      SUBCASE("Increment zero page X indexed") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$20",
            "LDX #$03",
            "STA $11",
            "INC $0E,X",

        });
        CHECK(fixture.bus->readByte(0x11) == 0x21);
      }
      SUBCASE("Increment absolute X indexed") {
        auto fixture = TestFixture::setupTestAndExecute({
            "LDA #$20",
            "LDX #$03",
            "STA $1134",
            "INC $1131,X",

        });
        CHECK(fixture.bus->readByte(0x1134) == 0x21);
      }
      SUBCASE("Increment, set flags") {
        auto fixture = TestFixture::setupTest({
            "LDX #$7F",
            "STX $20",
            "INC $20",
            "LDX #$FF",
            "STX $20",
            "INC $20",

        });
        fixture.cpu->step(3);
        CHECK(fixture.cpu->dumpRegisters().flags[N_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[Z_f]);

        fixture.cpu->step(3);
        CHECK(fixture.cpu->dumpRegisters().flags[Z_f]);
        CHECK_FALSE(fixture.cpu->dumpRegisters().flags[N_f]);
      } // TODO : test overflow
    }
  }
}