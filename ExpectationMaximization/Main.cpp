#include <iostream>
#include <chrono>
#include "ExpectationMaximization.hpp"

int main(void)
{
    auto i{std::chrono::high_resolution_clock::now()};
    ExpectationMaximization em{"dataset.txt"};
    auto f{std::chrono::high_resolution_clock::now()};
    std::cout << "\nEntrada de dados conluída em " << (std::chrono::duration_cast<std::chrono::nanoseconds>(f-i).count()) << " Nanosegundos!\n";
    i = std::chrono::high_resolution_clock::now();
    em.ProcessarEntradas();
    f = std::chrono::high_resolution_clock::now();
    std::cout << "\nProcessamento concluído em " << (std::chrono::duration_cast<std::chrono::nanoseconds>(f-i).count()) << " Nanosegundos!\n";
    while (true)
    {
        std::cout << "\n------------------------------------------------------------\n";
        unsigned short peso;
        double altura;
        std::cout << "\nQual é o peso da pessoa em questão em quilos? ";
        std::cin >> peso;
        std::cout << "Qual é a altura da pessoa em questão em metros? ";
        std::cin >> altura;
        std::cout << "\nEstado mais provável da pessoa em questão: " << em.Classificar(peso,altura) << "!\n";
    }
    return 0;
}