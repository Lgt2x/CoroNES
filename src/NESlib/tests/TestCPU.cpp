#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <string>
#include <cstdint>
#include <utility>

#include "helpers/TestFixture.h"
#include "doctest.h"



TEST_CASE("CPU supports all Adressing modes") {
  SUBCASE("Immediate addressing") {
    auto fixture = TestFixture::setupTest({"LDA #$0A"}, 0x800);

    CHECK(fixture.cpu->dumpRegisters()[0] == 0x00);
    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters()[0] == 0x0A);
  }

  SUBCASE("Zero-page addressing") {
    auto fixture = TestFixture::setupTest({"LDA $22"}, 0x800);
    fixture.bus->writeByte(0x22, 0x30);

    CHECK(fixture.cpu->dumpRegisters()[0] == 0x00);
    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters()[0] == 0x30);
  }

  SUBCASE("Absolute addressing") {
    auto fixture = TestFixture::setupTest({"LDA $0230"}, 0x800);
    fixture.bus->writeByte(0x0230, 0x56);

    fixture.cpu->step();
    CHECK(fixture.cpu->dumpRegisters()[0] == 0x56);
  }  
}

