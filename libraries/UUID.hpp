// https://gist.github.com/javierquevedo/ec65d82eeeba1cee8215067b311199b9#file-uuid-hpp
// UUID.hpp
#ifndef UUID_hpp
#define UUID_hpp

#include <random>
#include <sstream>
#include <string>

// Random generation courtesy of https://lowrey.me/guid-generation-in-c-11/

class UUID {
public:
  /**
   * @brief Generates a uuid string in the form
   * b9317db-02a2-4882-9b94-d1e1defe8c56
   *
   * @return std::string
   */
  static std::string generate_uuid() {
    std::stringstream hexstream;
    hexstream << UUID::generate_hex(4) << "-" << UUID::generate_hex(2) << "-"
              << UUID::generate_hex(2) << "-" << UUID::generate_hex(2) << "-"
              << UUID::generate_hex(6);
    return hexstream.str();
  }

  /**
   * @brief
   *
   * @param len Length in bytes
   * @return std::string  String random hex chars (2x length of the bytes)
   */
  static std::string generate_hex(const unsigned int len) {
    std::stringstream ss;
    for (auto i = 0; i < len; i++) {
      const auto rc = random_char();
      std::stringstream hexstream;
      hexstream << std::hex << rc;
      auto hex = hexstream.str();
      ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
  }

private:
  /**
   * @brief Generates a safe pseudo-random character
   *
   * @return unsigned int
   */
  static unsigned int random_char() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return dis(gen);
  }
};


#endif