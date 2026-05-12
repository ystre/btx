#include <libbtx/btx.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

[[nodiscard]] std::vector<std::uint8_t> read_input(std::string_view path) {
    if (path == "-") {
        return { std::istreambuf_iterator<char>(std::cin), {} };
    }
    std::ifstream f(path.data(), std::ios::binary);
    if (not f) {
        throw std::runtime_error("cannot open '" + std::string(path) + "'");
    }
    return { std::istreambuf_iterator<char>(f), {} };
}

int cmd_to_bin(std::string_view path) {
    const auto raw = read_input(path);

    std::uint8_t* out = nullptr;
    std::size_t out_len = 0;
    btx_error_t err = {};
    const btx_result_t r = btx_to_bin(reinterpret_cast<const char*>(raw.data()), raw.size(), &out, &out_len, &err);
    if (r != BTX_OK) {
        std::cerr << "btx: " << btx_strerror(r) << " at line " << err.line << " col " << err.col << '\n';
        return EXIT_FAILURE;
    }
    std::cout.write(reinterpret_cast<const char*>(out), static_cast<std::streamsize>(out_len));
    btx_free(out);
    return EXIT_SUCCESS;
}

int cmd_from_bin(std::string_view path) {
    const auto raw = read_input(path);

    char* out = nullptr;
    std::size_t out_len = 0;
    const btx_result_t r = btx_from_bin(raw.data(), raw.size(), &out, &out_len);
    if (r != BTX_OK) {
        std::cerr << "btx: " << btx_strerror(r) << '\n';
        return EXIT_FAILURE;
    }
    std::cout.write(out, static_cast<std::streamsize>(out_len));
    btx_free(out);
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
    try {
        bool reverse = false;
        std::string_view file;

        for (const std::string_view arg : std::span(argv + 1, argc - 1)) {
            if (arg == "--help" || arg == "-h") {
                std::cout <<
                    "Usage: btx [-r] <file>\n"
                    "\n"
                    "  <file>    encode binary to BTX text, write to stdout\n"
                    "  -r <file> decode BTX text to binary, write to stdout\n"
                    "\n"
                    "use - as <file> to read from stdin\n"
                    "\n"
                    "options:\n"
                    "  -r, --reverse  decode BTX text to binary (default: encode binary to BTX)\n"
                    "  -h, --help     show this help\n"
                    "  -V, --version  print version\n";
                return EXIT_SUCCESS;
            }
            if (arg == "--version" || arg == "-V") {
                std::cout << "btx " BTX_VERSION "\n";
                return EXIT_SUCCESS;
            }
            if (arg == "--reverse" || arg == "-r") {
                reverse = true;
            } else if (arg == "-" || not arg.starts_with('-')) {
                file = arg;
            } else {
                std::cerr << "btx: unknown option '" << arg << "'\ntry: btx --help\n";
                return EXIT_FAILURE;
            }
        }

        if (file.empty()) {
            std::cerr << "Usage: btx [-r] <file>\ntry: btx --help\n";
            return EXIT_FAILURE;
        }

        return reverse ? cmd_to_bin(file) : cmd_from_bin(file);
    } catch (const std::exception& ex) {
        std::cerr << "btx: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
