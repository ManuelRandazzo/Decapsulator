#include "R_TRIG.hpp"


R_TRIG::R_TRIG() {};


/// @brief  Verifica se è stato triggerato o no in rising edge
/// @param CLK Variabile da interrogare
/// @return true se triggerato; false se non triggerato
void R_TRIG::CLK(const bool ToCLK)
{
    this->isTrigged = (!lastLevel && ToCLK);
    lastLevel = ToCLK;
}


/// @brief ritorna se è stato triggerato o no
bool R_TRIG::Q()
{
    return this->isTrigged;
}