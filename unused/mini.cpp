#include <sys/stat.h>
#include <string>
#include <iostream>
#include <fstream>
#include <bitset>
#include <vector>



// Get filesize in bytes.
uint16_t get_file_size_bytes(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

// Read the bytes of a file into a u16 array.
uint16_t* read_file_into_u16_arr(std::istream& file, uint16_t length)
{
    char* bytes = new char[length];
    file.read(bytes, length);
    uint16_t* result = (uint16_t*) bytes;
    return result;
}

int main(int argc, char const *argv[]) {
    // Read all the bytes in the file into memory.
    std::string filename = "temp.txt";
    std::ifstream file(filename, std::ios::in | std::ios::binary);

    uint16_t u8_length;
    uint16_t u16_length;

    u8_length = get_file_size_bytes(filename);
    u16_length = u8_length / 2;
    uint16_t* arr = read_file_into_u16_arr(file, u8_length);

    // Print the binary representation of the number.
    for (size_t i = 0; i < u16_length; i++) {
        std::cout << (std::bitset<16>)arr[i] << '\n';
    }

    // Initialize the virtual machine.
    uint16_t MEMORY_SIZE = 1024;
    uint16_t* memory = new uint16_t[MEMORY_SIZE];
    std::vector<uint16_t> stack;
    uint16_t pointer = 0;

    LispVirtualMachine virtual_machine { memory, stack, pointer };

    return 0;
}
