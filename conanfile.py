from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, cmake_layout


class MetamorfnoTestiranjeConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    default_options = {"qpdf/*:with_ssl": "internal"}

    def layout(self):
        cmake_layout(self)

    def validate(self):
        check_min_cppstd(self, 20)

    def requirements(self):
        self.requires("cpp-httplib/0.56.0")
        self.requires("nlohmann_json/3.12.0")
        self.requires("qpdf/12.3.2")
        self.requires("b64/2.0.0.1")
        self.requires("fmt/12.1.0")
        self.requires("spdlog/1.17.0")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
