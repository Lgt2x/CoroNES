#ifndef NES_CARTRIDGE_H
#define NES_CARTRIDGE_H

#include <string>
#include <memory>
#include <vector>

/**
 Reads iNES files
 */
class Cartridge {
public:
  explicit Cartridge(const std::string& filename);
  Cartridge(Cartridge& cartridge) = delete;

  bool extended();
  const std::vector<uint8_t>& getPRG_ROM();
  const std::vector<uint8_t>& getCHR_ROM();
private:
  std::vector<uint8_t> prg_rom;
  std::vector<uint8_t> chr_rom;
};


#endif //NES_CARTRIDGE_H
