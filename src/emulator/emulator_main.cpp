#include <emulator/emulator.hpp>

#include <common/exceptions.hpp>

#include <iostream>

int main(int argc, char* argv[])
{
  if(argc != 2)
  {
    throw common::RuntimeError("Greska! Ispravna sintaksa: ./emulator putanja_do_fajla");
  }

  try
  {
    emulator_core::Emulator emulator(argv[0]);
    emulator.emulate();
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return -1;
  }
  
}