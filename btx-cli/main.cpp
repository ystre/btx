#include <btx/btx.h>

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

int cmd_decode(const std::string& path) {
    const auto raw = read_input(path);

    std::uint8_t* out = nullptr;
    std::size_t out_len = 0;
    const btx_result_t r = btx_decode(reinterpret_cast<const char*>(raw.data()), raw.size(), &out, &out_len);
    if (r != BTX_OK) {
        std::cerr << "btx: " << btx_strerror(r) << '\n';
        return EXIT_FAILURE;
    }
    std::cout.write(reinterpret_cast<const char*>(out), static_cast<std::streamsize>(out_len));
    btx_free(out);
    return EXIT_SUCCESS;
}

int cmd_encode(const std::string& path) {
    const auto raw = read_input(path);

    char* out = nullptr;
    std::size_t out_len = 0;
    const btx_result_t r = btx_encode(raw.data(), raw.size(), &out, &out_len);
    if (r != BTX_OK) {
        std::cerr << "btx: " << btx_strerror(r) << '\n';
        return EXIT_FAILURE;
    }
    std::cout.write(out, static_cast<std::streamsize>(out_len));
    btx_free(out);
    return EXIT_SUCCESS;
}

int cmd_validate(const std::string& path) {
    const auto raw = read_input(path);

    const btx_result_t r = btx_validate(reinterpret_cast<const char*>(raw.data()), raw.size());
    if (r != BTX_OK) {
        std::cerr << "btx: " << btx_strerror(r) << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

[[nodiscard]] auto parse_args(int argc, char* argv[]) -> std::optional<po::variables_map> {
    auto visible = po::options_description("options");
    visible.add_options()
        ("help,h",    "show this help")
        ("version,V", "print version");

    auto hidden = po::options_description();
    hidden.add_options()
        ("command", po::value<std::string>())
        ("args",    po::value<std::vector<std::string>>());

    po::positional_options_description pos;
    pos.add("command", 1).add("args", -1);

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
            "Usage: btx [options] <command> [args]\n"
            "\n"
            "commands:\n"
            "  encode <file>     encode binary to BTX text, write to stdout\n"
            "  decode <file>     decode BTX text to binary, write to stdout\n"
            "  validate <file>   validate BTX text; exit 1 on error\n"
            "\n"
            "use - as <file> to read from stdin\n"
            "\n"
            << visible;
        return std::nullopt;
    }

    return vm;
}

[[nodiscard]] int entrypoint(const po::variables_map& args) {
    if (not args.contains("command")) {
        std::cerr << "Usage: btx [options] <command> [args]\ntry: btx --help\n";
        return EXIT_FAILURE;
    }

    const auto cmd      = args["command"].as<std::string>();
    const auto cmd_args = args.contains("args")
        ? args["args"].as<std::vector<std::string>>()
        : std::vector<std::string>{};

    if (cmd != "encode" and cmd != "decode" and cmd != "validate") {
        std::cerr << "btx: unknown command '" << cmd << "'\ntry: btx --help\n";
        return EXIT_FAILURE;
    }
    if (cmd_args.empty()) {
        std::cerr << "btx: '" << cmd << "' requires a file argument\ntry: btx --help\n";
        return EXIT_FAILURE;
    }
    if (cmd_args.size() > 1) {
        std::cerr << "btx: unexpected argument '" << cmd_args[1] << "'\ntry: btx --help\n";
        return EXIT_FAILURE;
    }

    if (cmd == "decode")   return cmd_decode(cmd_args[0]);
    if (cmd == "encode")   return cmd_encode(cmd_args[0]);
    if (cmd == "validate") return cmd_validate(cmd_args[0]);

    return EXIT_FAILURE;
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
