//
// Created by louis on 17/01/23.
//

#ifndef NES_CARTRIDGE_H
#define NES_CARTRIDGE_H

#include <string>
#include <memory>
#include <vector>

class Cartridge {
public:
  Cartridge() = default;
  Cartridge(Cartridge& cartridge) = delete;
  void loadCart(const std::string& filename);
  bool extended();
  const std::vector<uint8_t>& getPRG_ROM();
  const std::vector<uint8_t>& getCHR_ROM();
private:
  std::vector<uint8_t> prg_rom;
  std::vector<uint8_t> chr_rom;
};


#endif //NES_CARTRIDGE_H
