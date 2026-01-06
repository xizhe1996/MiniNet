#include <iostream>
#include <string>

#include "mininet/buffer.h"

using mininet::Buffer;

int main() {
  Buffer buf;

  // append 10000 bytes
  std::string big(10000, 'x');
  buf.append(big);
  std::cout << "After append big: readable=" << buf.readable_bytes()
            << " writable=" << buf.writable_bytes() << "\n";

  // consume 9000 bytes
  buf.retrieve(9000);
  std::cout << "After retrieve 9000: readable=" << buf.readable_bytes()
            << " writable=" << buf.writable_bytes() << "\n";

  // append 8000 bytes again
  std::string big2(8000, 'y');
  buf.append(big2);

  std::cout << "After append big2: readable=" << buf.readable_bytes()
            << " writable=" << buf.writable_bytes() << "\n";

  // check content prefix
  std::string head(buf.peek(), 10);
  std::cout << "peek head: " << head << "\n";

  return 0;
}
