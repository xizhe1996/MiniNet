#include <iostream>

#include "mininet/buffer.h"

using mininet::Buffer;

int main() {
  Buffer buf;
  buf.append("hello", 5);
  buf.append(std::string(" world"));
  std::cout << "readable: " << buf.readable_bytes() << "\n";
  std::cout << "peek: " << std::string(buf.peek(), buf.readable_bytes())
            << "\n";

  buf.retrieve(6);  // "hello "
  std::cout << "after retrieve(6), readable: " << buf.readable_bytes() << "\n";
  std::cout << "peek: " << std::string(buf.peek(), buf.readable_bytes())
            << "\n";

  std::string str = buf.retrieve_all_as_string();
  std::cout << "after retrieve_all, string: " << str << "\n";
  std::cout << "after retrieve_all, readable: " << buf.readable_bytes() << "\n";

  return 0;
}