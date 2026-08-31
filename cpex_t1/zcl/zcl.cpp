#include <iostream>
#include <zcl/zcl.hpp>

std::string zcl::file::read_file_to_string(std::string filePath) {
    std::string contentStr;
    auto file = std::ifstream(filePath);
    
    if (file.is_open()) {
        std::streampos fileSz;

        file.seekg(0, std::ios::end);
        fileSz = file.tellg();
        contentStr.resize(fileSz);
        file.seekg(0, std::ios::beg);

        file.read(&contentStr[0], fileSz);
        // std::cout << "(SIZE: " << fileSz << ")\n" << contentStr;
    }
    else {
        throw std::runtime_error(std::string("Could not open file `") + filePath + "`");
    }
    
    return contentStr;
}