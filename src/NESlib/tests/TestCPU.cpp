#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <cstdint>
#include <string>
#include <utility>

#include "doctest.h"
#include "helpers/TestFixture.h"

TEST_CASE("CPU reset sets the program counter to the reset vector") {
  auto bus = std::make_unique<Bus>(new DummyMapper());

  // Set reset vector
  uint16_t startAddress = 0x800;
  bus->writeByte(0xFFFC, startAddress & 0xFF);
  bus->writeByte(0xFFFD, startAddress >> 8);

  // Reset CPU
  auto cpu = std::make_unique<CPU_6502>(bus.get());
  cpu->reset();

  // PC should be set to reset vector
  CHECK(cpu->dumpRegisters()[5] == (0x800 & 0xFF));
  CHECK(cpu->dumpRegisters()[6] == (0x800 >> 8));
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

  //   CHECK(fixture.cpu->dumpRegisters()[0] == 0x00);
  //   fixture.cpu->step();
  //   CHECK(fixture.cpu->dumpRegisters()[0] == 0x56);
  // }

  SUBCASE("Zero-page addressing") {
    auto fixture = TestFixture::setupTest({"LDA $22"}, 0x800);
    fixture.bus->writeByte(0x22, 0x30);

    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters()[0] == 0x30);
  }

  SUBCASE("Immediate addressing") {
    auto fixture = TestFixture::setupTest({"LDA #$0A"}, 0x800);

    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters()[0] == 0x0A);
  }

  SUBCASE("Absolute addressing") {
    auto fixture = TestFixture::setupTest({"LDA $1234"}, 0x800);
    fixture.bus->writeByte(0x1234, 0x30);

    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters()[0] == 0x30);
  }

  // SUBCASE("Y Indirect post-indexed") {
  //   auto fixture = TestFixture::setupTest({"LDY #$11", "LDA ($12),Y"},
  //   0x800); fixture.bus->writeByte(0x12, 0x65); fixture.bus->writeByte(0x13,
  //   0x02); fixture.bus->writeByte(0x0276, 0x30);

  //   // $265 indexed by Y=$11 gives address $276, loading this byte in A
  //   fixture.cpu->step();
  //   CHECK(fixture.cpu->dumpRegisters()[0] == 0x30);
  // }

  // SUBCASE("X-indexed Zero-page addressing") {
  //   auto fixture = TestFixture::setupTest({"LDX #$11", "LDA $22,X"}, 0x800);
  //   fixture.bus->writeByte(0x33, 0x30);

  //   fixture.cpu->step();
  //   CHECK(fixture.cpu->dumpRegisters()[0] == 0x30);
  // }

  // SUBCASE("Absolute indexed by X addressing") {
  //   auto fixture = TestFixture::setupTest({"LDX #$11", "LDA $0230,X"},
  //   0x800); fixture.bus->writeByte(0x0241, 0x56);

  //   fixture.cpu->step();
  //   CHECK(fixture.cpu->dumpRegisters()[0] == 0x56);
  // }

  // SUBCASE("Absolute indexed by Y addressing") {
  //   auto fixture = TestFixture::setupTest({"LDY #$11", "LDA $0230,Y"},
  //   0x800); fixture.bus->writeByte(0x0241, 0x56);

  //   fixture.cpu->step();
  //   CHECK(fixture.cpu->dumpRegisters()[0] == 0x56);
  // }
}

TEST_CASE("CPU supports all 6502 opcodes") {
  SUBCASE("ORA") {
    auto fixture = TestFixture::setupTest(
        {"LDA #%10100110", "STA $20", "LDA#%11001100", "ORA $20"}, 0x800);

    //    10100110
    // OR 11001100
    //  = 11101110

    fixture.cpu->step(4);
    CHECK(fixture.cpu->dumpRegisters()[0] == 0b11101110);
  }

  SUBCASE("AND") {
    auto fixture = TestFixture::setupTest(
        {"LDA #%10100110", "STA $20", "LDA#%11001100", "AND $20"}, 0x800);

    //     10100110
    // AND 11001100
    //   = 10000100

    fixture.cpu->step(4);
    CHECK(fixture.cpu->dumpRegisters()[0] == 0b10000100);
  }

  SUBCASE("EOR") {
    auto fixture = TestFixture::setupTest(
        {"LDA #%10100110", "STA $20", "LDA#%11001100", "EOR $20"}, 0x800);

    //     10100110
    // EOR 11001100
    //   = 01101010

    fixture.cpu->step(4);
    CHECK(fixture.cpu->dumpRegisters()[0] == 0b01101010);
  }

  SUBCASE("ADC") {
    auto fixture = TestFixture::setupTest(
        {"LDA #%10100110", "STA $20", "LDA#%11001100", "ADC $20"}, 0x800);

    //      10100110
    // +    11001100
    //   = 101110010

    fixture.cpu->step(4);
    CHECK(fixture.cpu->dumpRegisters()[0] == 0b01110010);
  }

  SUBCASE("STA") {
    auto fixture = TestFixture::setupTest({"LDA #$0A", "STA $20"}, 0x800);
    fixture.cpu->step();
    fixture.cpu->step();
    CHECK(fixture.bus->readByte(0x20) == 0x0A);
  }

  SUBCASE("LDA") {
    auto fixture =
        TestFixture::setupTest({"LDA #$0A", "LDA #$FE", "LDA #$00"}, 0x800);

    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters()[0] == 0x0A);

    // Check flags
    // fixture.cpu->step();
    // CHECK(fixture.cpu->dumpRegisters()[0] == 0xFE);
    // CHECK((fixture.cpu->dumpRegisters()[4]&0b1000000) == 1);

    // fixture.cpu->step();
    // CHECK(fixture.cpu->dumpRegisters()[0] == 0x00);
    // CHECK((fixture.cpu->dumpRegisters()[4]&0b00000010) == 0b10);
  }

  SUBCASE("CMP") {}

  SUBCASE("SBC") {
    auto fixture = TestFixture::setupTest(
        {"LDA #%10100110", "STA $20", "LDA#%11001100", "SBC $20"}, 0x800);

    //     11001100
    // -   10100110
    //   = 00100110 

    fixture.cpu->step(4);
    CHECK(fixture.cpu->dumpRegisters()[0] == 0b100110);
  }
}