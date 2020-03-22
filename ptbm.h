/**
 * Parentheses Tree Based Multicast header processor
 *
 * Written by Andras Majdan
 * License: GNU General Public License Version 3
 *
 * Report bugs to <majdan.andras@gmail.com>
 */

#ifndef PTBM_H
#define PTBM_H

#include <bitset>
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

namespace ptbm
{

// HEADER_SIZE  The size of the header in bits
// PORT_SIZE    The size of the port numbers in bits
// MAX_OPEN_BRACKETS  The maximal number of open brackets in the header
// PORTS_START_AT     At which bit does the list of ports start
template
  <int HEADER_SIZE=256,
   int PORT_SIZE=4,
   int MAX_OPEN_BRACKETS=43,
   int PORTS_START_AT=84>
class Ptbm
{

public:

Ptbm()
{
  pbs = bitset<HEADER_SIZE>(0);
}

/// Sets the header from bits (in string form)
void
setHeaderBitset(string bits)
{
  pbs = bitset<HEADER_SIZE>(bits);
}

/// Sets the header from brackets and port numbers
void
setHeader(string brackets, vector<unsigned int> nums)
{
  pbs = generateHeader(brackets, nums);
}

/// Gets the header in string form (of brackets and port numbers)
string
getHeaderString()
{
  return headerToString(pbs);
}

/// Returns the header in bitset form
bitset<HEADER_SIZE>
getHeaderBits()
{
  return pbs;
}

/// Sets the virtual ports list
void
setVirtualPorts(vector<unsigned int> vports)
{
  pvports = vports;
}

/// Process the header as a router and call procFunc for each port output
void
procHeader(void (*procFunc)(string))
{
  vector<unsigned int> portToSend;
  vector<bitset<HEADER_SIZE>> subtreesToSend;

  processHeader(pbs, pvports, portToSend, subtreesToSend);

  int cntPorts = portToSend.size();

  for(int n=0; n<cntPorts; n++)
  {
    string strHeader = headerToString(subtreesToSend[n]);
    procFunc(to_string(portToSend[n]) + " " +       // Output port
             (strHeader.size() ? strHeader : "*")); // Subtree to send
  }
}

private:
bitset<HEADER_SIZE> pbs;
vector<unsigned int> pvports = {};

struct compare
{
        int key;
        compare(int const &i): key(i) { }

        bool operator()(int const &i)
        {
                return (i == key);
        }
};

/// Reads a number from a position in a bitset
unsigned int
readInt(bitset<HEADER_SIZE> &bs, size_t pos)
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

/// Sets bits of a number from a position in a bitset
void
setBits(bitset<HEADER_SIZE> &bs, size_t pos, unsigned int num)
{
  if(num > (1<<PORT_SIZE)-1 )
    throw runtime_error(
        "Number out of range: "
        + to_string(num) + " cannot fit in "
        + to_string(PORT_SIZE) + " bits");

  for(size_t i=0; i<PORT_SIZE; i++)
  {
    bs[pos+i] = num%2;
    num /= 2;
  }
}

// Gets bitset from the textual form (brackets and ports numbers) of the header
bitset<HEADER_SIZE>
generateHeader(string br, vector<unsigned int> nums)
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

/// Process a real subtree (no virtual port)
void
processNextRealSubtree(
    unsigned int port,
    bitset<HEADER_SIZE> bs,
    int &bracketPos,
    int &numPos,
    vector<unsigned int> &portToSend,
    vector<bitset<HEADER_SIZE>> &subtreesToSend)
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


/// Process a virtual subtree (virtual port passed as parameter)
void
processNextVirtualSubtree(
    unsigned int port,
    bitset<HEADER_SIZE> bs,
    int &bracketPos,
    int &numPos,
    vector<unsigned int> &portToSend,
    vector<bitset<HEADER_SIZE>> &subtreesToSend)
{
  if(!bs[bracketPos])
    throw runtime_error(
        "Virtual port " + to_string(port) +
        " has no child at " + to_string(bracketPos+1));

  int virtualPortPair;
  int realPort;

  // (
  while(bracketPos<PORTS_START_AT && bs[bracketPos])  // (
  {
    virtualPortPair = readInt(bs, numPos);
    // +1 is mandatory because min(realPort) must be > max(normal port number)
    realPort = virtualPortPair + (port+1) * (1<<PORT_SIZE);

    numPos += PORT_SIZE;
    processNextRealSubtree(
          realPort, bs, ++bracketPos, numPos, portToSend, subtreesToSend);
  }

  ++bracketPos;
}

/// Call real or virtual subtree processor based on the root port
bool
processNextSubtree(
    bitset<HEADER_SIZE> bs,
    int &bracketPos,
    int &numPos,
    vector<unsigned int> virtualPorts,
    vector<unsigned int> &portToSend,
    vector<bitset<HEADER_SIZE>> &subtreesToSend)
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
    processNextVirtualSubtree(
          currPort, bs, bracketPos, numPos, portToSend, subtreesToSend);
  else
    processNextRealSubtree(
          currPort, bs, bracketPos, numPos, portToSend, subtreesToSend);

  return true;
}

/// Process the header, go through the subtrees
void
processHeader(
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
    processNextSubtree(
          bs, bracketPos, numPos, virtualPorts, portToSend, subtreesToSend)
  );
}

/// Convert a header bitset to its textual form (brackets and port numbers)
string
headerToString(bitset<HEADER_SIZE> bs)
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

};

}

#endif // PTBM_H
