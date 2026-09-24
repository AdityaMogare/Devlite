class Devlite < Formula
  desc "Developer environment doctor"
  homepage "https://github.com/AdityaMogare/Devlite"
  url "https://github.com/AdityaMogare/Devlite/archive/refs/tags/v0.1.0.tar.gz"
  sha256 "3e775f9d0581dd7deb666ed4892893099afb4b266f91f0f9e3340fad2e273125"
  head "https://github.com/AdityaMogare/Devlite.git", branch: "master"

  depends_on "cmake" => :build
  depends_on "cli11"
  depends_on "nlohmann-json"
  depends_on macos: :big_sur

  def install
    system "cmake", "-S", ".", "-B", "build",
           "-DDEVLITE_FETCH_DEPS=OFF",
           *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    assert_match(/\d+\.\d+\.\d+/, shell_output("#{bin}/devlite --version"))
  end
end
