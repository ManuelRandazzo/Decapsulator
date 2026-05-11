#include "F_TRIG.hpp"


F_TRIG::F_TRIG() {};


/// @brief  Verifica se è stato triggerato o no in falling edge
/// @param CLK Variabile da interrogare
/// @return true se triggerato; false se non triggerato
void F_TRIG::CLK(const bool ToCLK)
{
    this->isTrigged = (lastLevel && !ToCLK);
    lastLevel = ToCLK;
}


/// @brief ritorna se è stato triggerato o no
bool F_TRIG::Q()
{
    return this->isTrigged;
}