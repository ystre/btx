#include <libbtx/btx.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <variant>
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

[[nodiscard]] std::ostream& open_output(std::string_view path, std::ofstream& file) {
    if (path == "-") return std::cout;
    file.open(path.data(), std::ios::binary);
    if (not file) {
        throw std::runtime_error("cannot open '" + std::string(path) + "' for writing");
    }
    return file;
}

int cmd_to_bin(std::string_view infile, std::string_view outfile) {
    const auto raw = read_input(infile);

    std::uint8_t* out = nullptr;
    std::size_t out_len = 0;
    btx_error_t err = {};
    const btx_result_t r = btx_to_bin(reinterpret_cast<const char*>(raw.data()), raw.size(), &out, &out_len, &err);
    if (r != BTX_OK) {
        std::cerr << "btx: " << btx_strerror(r) << " at line " << err.line << " col " << err.col << '\n';
        return EXIT_FAILURE;
    }
    std::ofstream ofs;
    open_output(outfile, ofs).write(reinterpret_cast<const char*>(out), static_cast<std::streamsize>(out_len));
    btx_free(out);
    return EXIT_SUCCESS;
}

int cmd_from_bin(std::string_view infile, std::string_view outfile) {
    const auto raw = read_input(infile);

    char* out = nullptr;
    std::size_t out_len = 0;
    const btx_result_t r = btx_from_bin(raw.data(), raw.size(), &out, &out_len);
    if (r != BTX_OK) {
        std::cerr << "btx: " << btx_strerror(r) << '\n';
        return EXIT_FAILURE;
    }
    std::ofstream ofs;
    open_output(outfile, ofs).write(out, static_cast<std::streamsize>(out_len));
    btx_free(out);
    return EXIT_SUCCESS;
}

[[nodiscard]] std::string_view help() {
    return
        "Usage:\n"
        "       btx [options] [infile [outfile]]\n"
        "    or\n"
        "       btx -r [infile [outfile]]\n"
        "\n"
        "Options:\n"
        "    -h, --help          print this summary.\n"
        "    -r, --reverse       reverse operation: convert BTX to binary.\n"
        "    -V, --version       show version.\n"
    ;
}

struct cli_args {
    std::string_view infile = "-";
    std::string_view outfile = "-";
    bool reverse = false;
};

[[nodiscard]] auto parse_args(int argc, char* argv[])
        -> std::variant<cli_args, int>
{
    const auto argv_span = std::span(argv + 1, static_cast<std::size_t>(argc - 1));
    std::vector<std::string_view> positionals;
    cli_args ret;

    for (std::size_t i = 0; i < std::size(argv_span); ++i) {
        std::string_view arg = argv_span[i];
        if (arg == "-h" || arg == "--help") {
            std::cout << help();
            return EXIT_SUCCESS;
        } else if (arg == "-V" || arg == "--version") {
            std::cout << "btx " BTX_VERSION "\n";
            return EXIT_SUCCESS;
        } else if (arg == "-r" || arg == "--reverse") {
            ret.reverse = true;
        } else if (arg == "-" || not arg.starts_with('-')) {
            positionals.push_back(arg);
        } else {
            std::cerr << "btx: unknown option '" << arg << "'\ntry: btx --help\n";
            return EXIT_FAILURE;
        }
    }

    if (positionals.size() > 2) {
        std::cerr << "btx: too many arguments\ntry: btx --help\n";
        return EXIT_FAILURE;
    }
    if (positionals.size() > 0) ret.infile = positionals[0];
    if (positionals.size() > 1) ret.outfile = positionals[1];

    return ret;
}

int main(int argc, char* argv[]) {
    try {
        const auto result = parse_args(argc, argv);
        if (auto* rc = std::get_if<int>(&result)) return *rc;
        const auto& args = std::get<cli_args>(result);

        return args.reverse ? cmd_to_bin(args.infile, args.outfile) : cmd_from_bin(args.infile, args.outfile);
    } catch (const std::exception& ex) {
        std::cerr << "btx: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }
}
