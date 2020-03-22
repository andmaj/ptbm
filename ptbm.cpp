/**
 * Parentheses Tree Based Multicast header processor
 *
 * Written by Andras Majdan
 * License: GNU General Public License Version 3
 *
 * Report bugs to <majdan.andras@gmail.com>
 */

#include <stdio.h>
#include <string>
#include <stdexcept>

#include <cxxopts.hpp>
#include "ptbm.h"

using namespace std;

void setVirtualPorts(ptbm::Ptbm<> &p, cxxopts::ParseResult opts)
{
  if(opts.count("virtual"))
    p.setVirtualPorts(opts["virtual"].as<vector<unsigned int>>());
}

void printProcLine(string line)
{
  cout << line << endl;
}

int main(int argc, char **argv)
{
  cxxopts::Options options("PTBM", "Parentheses Tree Based Multicast");

  options.add_options()
    ("b,brackets", "Brackets in the header",
      cxxopts::value<string>()->default_value(""))
    ("n,numbers", "Numbers in the header",
      cxxopts::value<vector<unsigned int>>())
    ("v,virtual", "Virtual ports",
      cxxopts::value<vector<unsigned int>>())
    ("g,generate", "Generate header (binary)",
      cxxopts::value<bool>()->default_value("false"))
    ("i,input", "Input header (binary)",
      cxxopts::value<bool>()->default_value("false"))
    ("p,print", "Print header",
      cxxopts::value<bool>()->default_value("false"))
    ("help", "Print usage")
    ;

  auto opts = options.parse(argc, argv);

  if (opts.count("help"))
  {
    std::cout << options.help() << std::endl;
     exit(0);
  }

  ptbm::Ptbm<> pt;

  if(opts["input"].as<bool>())
  {
    if(opts.count("brackets") ||
       opts.count("numbers") ||
       opts.count("generate"))
    {
      throw cxxopts::OptionException(
              "input option cannot be used with: brackets, numbers, generate");
    }

    string bits;
    cin >> bits;
    pt.setHeaderBitset(bits);

    if(opts.count("print"))
      cout << pt.getHeaderString();
    else
    {
      setVirtualPorts(pt, opts);
      pt.procHeader(printProcLine);
    }

    exit(0);
  }

  string brackets = opts["brackets"].as<string>();
  vector<unsigned int> nums = {};

  if(opts["numbers"].count())
    nums = opts["numbers"].as<vector<unsigned int>>();

  if(opts["generate"].as<bool>())
  {
    if(opts.count("input") || opts.count("print"))
      throw cxxopts::OptionException(
          "generate option cannot be used with: input, print");

    pt.setHeader(brackets, nums);
    cout << pt.getHeaderBits();
    exit(0);
  }

  pt.setHeader(brackets, nums);
  setVirtualPorts(pt, opts);
  pt.procHeader(printProcLine);

  return 0;
}
