/**
 * zcl::stream - Stream wrappers and helpers.
 * ZIK@MMXXVI
 */

#include <zcl/zcl.hpp>
#include <zcl/stream.hpp>

using namespace zcl;

stream::byteistream::byteistream(std::streambuf *buff):
    std::istream(buff) {}

template <>
stream::byteistream& stream::byteistream::operator>>(std::string &value) {
    // zcl::logger("ZCL")->info("[BYTEISTREAM] STRING READING DELEGATED TO GETLINE");
    // std::operator>><char, std::char_traits<char>, std::allocator<char>>((*this), value);
    std::getline(*this, value, '\0');

    if (!this->good()) {
        throw std::runtime_error("Failed to read from bytestream!");
    }

    return (*this);
};
