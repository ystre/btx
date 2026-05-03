from conan import ConanFile
from conan.tools.cmake import cmake_layout


class BTX(ConanFile):
    package_type = "application"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        for req in self.conan_data.get("requirements", []):
            self.requires(req)
