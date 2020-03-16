/**
 * Parentheses Tree Based Multicast header processor
 *
 * Written by Andras Majdan
 * License: GNU General Public License Version 3
 *
 * Report bugs to <majdan.andras@gmail.com>
 */

#include <stdio.h>

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <set>
#include <bitset>
#include <stdexcept>

#include <cxxopts.hpp>

using namespace std;

const int HEADER_SIZE = 256;
const int PORT_SIZE = 4;
const int MAX_OPEN_BRACKETS = 43;
const int PORTS_START_AT = 84;

void setBits(bitset<HEADER_SIZE> &bs, size_t pos, unsigned int num)
{
  if(num > (1<<PORT_SIZE)-1 )
    throw runtime_error("Number out of range (" + to_string(PORT_SIZE) + " bits): " + to_string(num));

  for(size_t i=0; i<PORT_SIZE; i++)
  {
    bs[pos+i] = num%2;
    num /= 2;
  }
};

unsigned int readInt(bitset<HEADER_SIZE> &bs, size_t pos)
{
  unsigned int res=0, bm = 0;
  bool bit;

  for(size_t i=0; i<PORT_SIZE; i++)
  {
    bit = bs[pos+i];
    res += bit ? 1<<bm : 0;
    ++bm;
  }
  return res;
}

bitset<HEADER_SIZE> generateHeader(string br, vector<unsigned int> nums)
{
  bitset<HEADER_SIZE> bs(0); // just for safety, default: initialize to zero
  int totalOpenBrackets = 0;

  for(string::size_type i = 0; i < br.size(); i++)
  {
    if(br[i]=='(')
    {
      bs[i] = true;
      ++totalOpenBrackets;
    }
    else if (br[i]!=')')
      throw runtime_error("Invalid bracket: " + to_string(br[i]));
  }

  if(totalOpenBrackets>MAX_OPEN_BRACKETS)
    throw runtime_error("Too many open brackets, header limit overflow");

  if(totalOpenBrackets!=(int)nums.size())
    throw runtime_error("Number of open brackets != Number of numbers");

  for(int pos=PORTS_START_AT, i=0; i<totalOpenBrackets; i++, pos+=PORT_SIZE)
    setBits(bs, pos, nums[i]);

  return bs;
}

struct compare
{
	int key;
	compare(int const &i): key(i) { }

	bool operator()(int const &i)
	{
		return (i == key);
	}
};

void processNextRealSubtree(unsigned int port, bitset<HEADER_SIZE> bs, int &bracketPos, int &numPos, vector<unsigned int> &portToSend, vector<bitset<256>> &subtreesToSend)
{
  int newBracketPos = 0;
  int newNumPos = PORTS_START_AT;
  bitset<HEADER_SIZE> newBitset(0);

  int currOpenBrackets = 1;

  for(;bracketPos<PORTS_START_AT; bracketPos++, newBracketPos++)
  {
    if(bs[bracketPos])
    {
      // (
      newBitset[newBracketPos] = 1;
      setBits(newBitset, newNumPos, readInt(bs, numPos));
      numPos += PORT_SIZE;
      newNumPos += PORT_SIZE;
      ++currOpenBrackets;
    }
    else
    {
      // )
      if(!--currOpenBrackets)
        break;
    }
  }

  if(currOpenBrackets)
    throw runtime_error("Subtree has no closing bracket");

  portToSend.push_back(port);
  subtreesToSend.push_back(newBitset);

  ++bracketPos;
}

void processNextVirtualSubtree(unsigned int port, bitset<HEADER_SIZE> bs, int &bracketPos, int &numPos, vector<unsigned int> &portToSend, vector<bitset<HEADER_SIZE>> &subtreesToSend)
{
  if(!bs[bracketPos])
    throw runtime_error("Virtual port " + to_string(port) + " has no child at " + to_string(bracketPos+1));

  int virtualPortPair;
  int realPort;

  // (
  while(bracketPos<PORTS_START_AT && bs[bracketPos])  // (
  {
    virtualPortPair = readInt(bs, numPos);
    realPort = virtualPortPair + port * (1<<PORT_SIZE);

    numPos += PORT_SIZE;
    processNextRealSubtree(realPort, bs, ++bracketPos, numPos, portToSend, subtreesToSend);
  }

  ++bracketPos;
}

bool processNextSubtree(bitset<HEADER_SIZE> bs, int &bracketPos, int &numPos, vector<unsigned int> virtualPorts, vector<unsigned int> &portToSend, vector<bitset<HEADER_SIZE>> &subtreesToSend)
{
  if(!bs[bracketPos])
    return false;

  if(bracketPos >= PORTS_START_AT)
    throw runtime_error("bracketPos >= PORTS_START_AT");

  unsigned int currPort;

  // (
  currPort = readInt(bs, numPos);
  ++bracketPos;
  numPos += PORT_SIZE;

  if(any_of(virtualPorts.begin(), virtualPorts.end(), compare(currPort)))
    processNextVirtualSubtree(currPort, bs, bracketPos, numPos, portToSend, subtreesToSend);
  else
    processNextRealSubtree(currPort, bs, bracketPos, numPos, portToSend, subtreesToSend);

  return true;
}

void processHeader(
  bitset<HEADER_SIZE> bs,
  vector<unsigned int> virtualPorts,
  vector<unsigned int> &portToSend,
  vector<bitset<HEADER_SIZE>> &subtreesToSend)
{
  if(!bs[0])
  {
    // It is for me
    portToSend.push_back(0);
    subtreesToSend.push_back(bitset<HEADER_SIZE>(0));
    return;
  }

  int bracketPos = 0;
  int numPos = PORTS_START_AT;

  while(
    processNextSubtree(bs, bracketPos, numPos, virtualPorts, portToSend, subtreesToSend)
  );
}

string headerToString(bitset<HEADER_SIZE> bs)
{
  string s = "";
  int currOpenBrackets = 0;
  int totalOpenBrackets = 0;
  bool checkOnlyOpenBrackets = false;

  int pos = 0;

  for(;pos<PORTS_START_AT; pos++)
  {
    if(checkOnlyOpenBrackets && bs[pos])
      throw runtime_error("Open bracket at a wrong place: " + to_string(pos+1));

    if(bs[pos])
    {
      ++currOpenBrackets;
      ++totalOpenBrackets;
      s.push_back('(');
    }
    else
    {
      if(currOpenBrackets <=0)  // just for safety, can't be < 0
      {
        // There must be no more open brackets
        checkOnlyOpenBrackets = true;
        continue;
      }
      --currOpenBrackets;
      s.push_back(')');
    }
  }

  if(currOpenBrackets)
    throw runtime_error("Not all brackets have been closed (after all)");

  pos = PORTS_START_AT; // just for safety, pos has been set by the for cycle
  bool firstReadInt = true;

  while(totalOpenBrackets-- > 0) // just for safety, can't be < 0
  {
    if(firstReadInt)
    {
     s.push_back(' ');
     firstReadInt = false;
    }
    else
     s.push_back(',');

    s.append(to_string(readInt(bs, pos)));
    pos += PORT_SIZE;
  }

  return s;
}

void printProcessedHeader(bitset<HEADER_SIZE> bs, cxxopts::ParseResult opts)
{
  vector<unsigned int> portToSend;
  vector<bitset<HEADER_SIZE>> subtreesToSend;
  vector<unsigned int> virtualPorts = {};

  if(opts.count("virtual"))
    virtualPorts = opts["virtual"].as<vector<unsigned int>>();

  processHeader(bs, virtualPorts, portToSend, subtreesToSend);

  int cntPorts = portToSend.size();

  for(int n=0; n<cntPorts; n++)
  {
    string strHeader = headerToString(subtreesToSend[n]);
    cout << to_string(portToSend[n]) << " " << (strHeader.size() ? strHeader : "*") << endl;
  }
}

int main(int argc, char **argv)
{
  cxxopts::Options options("PTBM", "Parentheses Tree Based Multicast");

  options.add_options()
    ("b,brackets", "Brackets in the header", cxxopts::value<string>()->default_value(""))
    ("n,numbers", "Numbers in the header", cxxopts::value<vector<unsigned int>>())
    ("v,virtual", "Virtual ports", cxxopts::value<vector<unsigned int>>())
    ("g,generate", "Generate header (binary)", cxxopts::value<bool>()->default_value("false"))
    ("i,input", "Input header (binary)", cxxopts::value<bool>()->default_value("false"))
    ("p,print", "Print header", cxxopts::value<bool>()->default_value("false"))
    ("help", "Print usage")
    ;

  auto opts = options.parse(argc, argv);

  if (opts.count("help"))
  {
    std::cout << options.help() << std::endl;
     exit(0);
  }

  if(opts["input"].as<bool>())
  {
    if(opts.count("brackets") || opts.count("numbers") || opts.count("generate"))
    {
      throw cxxopts::OptionException("input option cannot be used with: brackets, numbers, generate");
    }

    string bits;
    cin >> bits;
    bitset<HEADER_SIZE> bs (bits);

    if(opts.count("print"))
      cout << headerToString(bs);
    else
      printProcessedHeader(bs, opts);

    exit(0);
  }

  string brackets = opts["brackets"].as<string>();
  vector<unsigned int> nums = {};

  if(opts["numbers"].count())
    nums = opts["numbers"].as<vector<unsigned int>>();

  if(opts["generate"].as<bool>())
  {
    if(opts.count("input") || opts.count("print"))
      throw cxxopts::OptionException("generate option cannot be used with: input, print");

    cout << generateHeader(brackets, nums);
    exit(0);
  }

  printProcessedHeader(generateHeader(brackets, nums), opts);

  return 0;
}

/*// Set with contains() function (from C++20)
template <class T>
class set20 : public set<T>
{
#if __cplusplus <= 201703L
  public:
  bool contains(T param) { return this->find(param) != this->end(); }
#endif
};

// Extended bitset
template<size_t N>
class ebitset : public bitset<N>
{
  public:
  size_t setBits(size_t pos, size_t bitsnum, unsigned int num)
  {
    for(size_t i=0; i<bitsnum; i++)
    {
      this->set(pos+i, num%2);
      num /= 2;
    }

    return pos+bitsnum;
  };

  unsigned int readInt(size_t pos, size_t bitsnum)
  {
    unsigned int res=0, bm = 0;
    bool bit;

    for(size_t i=0; i<bitsnum; i++)
    {
      bit = this->operator[](pos+i);
      cout << bit;
      res += bit ? 1<<bm : 0;
      ++bm;
    }
    cout << ">" << res << "<";
    return res;
  }

};
*/
