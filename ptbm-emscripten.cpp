/**
 * Parentheses Tree Based Multicast header processor
 *
 * Written by Andras Majdan
 * License: GNU General Public License Version 3
 *
 * Report bugs to <majdan.andras@gmail.com>
 */

#ifdef EMSCRIPTEN

#include <stdio.h>
#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

#include "ptbm.h"

#include <emscripten/bind.h>

using namespace std;

string ptbm_em_lines = "";

void ptbm_em_concatLines(string text)
{
  ptbm_em_lines.append(text + "\n");
}

void ptbm_em_readIntoVector(string text, vector<unsigned int> &nums)
{
  stringstream ss(text);

  for (int i; ss >> i;)
  {
    nums.push_back(i);
    if (ss.peek() == ',')
      ss.ignore();
  }
}

void ptbm_em_getBracketsAndNums(
    string text,  string &brackets, vector<unsigned int> &nums)
{
  int brStart = text.find(' ');
  brackets = text.substr(0,brStart);
  ptbm_em_readIntoVector(text.substr(brStart+1), nums);
}

void ptbm_em_setHeaderFromText(ptbm::Ptbm<> &pt, string text)
{
  string brackets;
  vector<unsigned int> nums;
  ptbm_em_getBracketsAndNums(text, brackets, nums);
  pt.setHeader(brackets, nums);
}

void ptbm_em_setVirtualPorts(ptbm::Ptbm<> &pt, string ports)
{
  vector<unsigned int> nums;
  ptbm_em_readIntoVector(ports, nums);
  pt.setVirtualPorts(nums);
}

string ptbm_em_processedHeader(ptbm::Ptbm<> &pt)
{
  ptbm_em_lines = "";
  pt.procHeader(ptbm_em_concatLines);
  return ptbm_em_lines;
}


string ptbm_em_textFromBitset(string bitset) {
  ptbm::Ptbm<> pt;
  pt.setHeaderBitset(bitset);
  return pt.getHeaderString();
}

string ptbm_em_bitsetFromText(string text)
{
 ptbm::Ptbm<> pt;
 ptbm_em_setHeaderFromText(pt, text);
 return pt.getHeaderBits().to_string();
}

string ptbm_em_processBitset(string bitset, string virtualPorts)
{
  ptbm::Ptbm<> pt;
  ptbm_em_setVirtualPorts(pt, virtualPorts);
  pt.setHeaderBitset(bitset);
  return ptbm_em_processedHeader(pt);
}

string ptbm_em_processText(string text, string virtualPorts)
{
  ptbm::Ptbm<> pt;
  ptbm_em_setVirtualPorts(pt, virtualPorts);
  ptbm_em_setHeaderFromText(pt, text);
  return ptbm_em_processedHeader(pt);
}

EMSCRIPTEN_BINDINGS(ptbm_module) {
    emscripten::function("ptbm_em_textFromBitset", &ptbm_em_textFromBitset);
    emscripten::function("ptbm_em_bitsetFromText", &ptbm_em_bitsetFromText);
    emscripten::function("ptbm_em_processBitset", &ptbm_em_processBitset);
    emscripten::function("ptbm_em_processText", &ptbm_em_processText);
}

#endif
