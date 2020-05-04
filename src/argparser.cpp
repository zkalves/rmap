#include "cxxopts.hpp"

void parse(int argc, char* argv[])
{
  try
  {
    cxxopts::Options options(argv[0], " - Register map generation tool");

    options
      .add_options()
      ("f, file", "File", cxxopts::value<std::vector<std::string>>(), "FILE")
      ("h,help", "Print help")
    ;

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
      std::cout << options.help({"", "Group"}) << std::endl;
      exit(0);
    }

    if (result.count("f"))
    {
      auto& ff = result["f"].as<std::vector<std::string>>();
      std::cout << "Files" << std::endl;
      for (const auto& f : ff)
      {
        std::cout << f << std::endl;
      }
    }

    auto arguments = result.arguments();
  }
  catch (const cxxopts::OptionException& e)
  {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(1);
  }
}

