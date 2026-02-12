#include "../src/phase5_interface/cli.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        helixforge::CLIApp app;
        return app.run(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error" << std::endl;
        return 1;
    }
}
