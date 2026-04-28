#pragma once

class R_TRIG
{
    public :
        R_TRIG();

        /// @brief  Verifica se è stato triggerato o no in rising edge
        /// @param ToCLK Variabile da interrogare
        /// @return true se triggerato; false se non triggerato
        void CLK(const bool ToCLK);

        /// @brief ritorna se è stato triggerato o no
        bool Q();
    private :
        bool lastLevel = false;
        bool isTrigged = false;
};