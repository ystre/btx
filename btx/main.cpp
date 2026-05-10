#include <libbtx/btx.h>

#include <boost/program_options.hpp>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace po = boost::program_options;

[[nodiscard]] std::vector<std::uint8_t> read_input(const std::string& path) {
    if (path == "-") {
        return { std::istreambuf_iterator<char>(std::cin), {} };
    }
    std::ifstream f(path, std::ios::binary);
    if (not f) {
        throw std::runtime_error("cannot open '" + path + "'");
    }
    return { std::istreambuf_iterator<char>(f), {} };
}

int cmd_to_bin(const std::string& path) {
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

int cmd_from_bin(const std::string& path) {
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

[[nodiscard]] auto parse_args(int argc, char* argv[]) -> std::optional<po::variables_map> {
    auto visible = po::options_description("options");
    visible.add_options()
        ("reverse,r", "decode BTX text to binary (default: encode binary to BTX)")
        ("help,h",    "show this help")
        ("version,V", "print version")
    ;

    auto hidden = po::options_description();
    hidden.add_options()
        ("file", po::value<std::string>());

    po::positional_options_description pos;
    pos.add("file", 1);

    auto all = po::options_description();
    all.add(visible).add(hidden);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(all).positional(pos).run(), vm);
    po::notify(vm);

    if (vm.contains("version")) {
        std::cout << "btx " BTX_VERSION "\n";
        return std::nullopt;
    }

    if (vm.contains("help")) {
        std::cout <<
            "Usage: btx [-r] <file>\n"
            "\n"
            "  <file>    encode binary to BTX text, write to stdout\n"
            "  -r <file> decode BTX text to binary, write to stdout\n"
            "\n"
            "use - as <file> to read from stdin\n"
            "\n"
            << visible;
        return std::nullopt;
    }

    return vm;
}

[[nodiscard]] int entrypoint(const po::variables_map& args) {
    if (not args.contains("file")) {
        std::cerr << "Usage: btx [-r] <file>\ntry: btx --help\n";
        return EXIT_FAILURE;
    }

    const auto file = args["file"].as<std::string>();

    if (args.contains("reverse")) return cmd_to_bin(file);
    return cmd_from_bin(file);
}

int main(int argc, char* argv[]) {
    try {
        const auto args = parse_args(argc, argv);
        if (not args.has_value()) {
            return EXIT_SUCCESS;
        }
        return entrypoint(*args);
    } catch (const std::exception& ex) {
        std::cerr << "btx: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
