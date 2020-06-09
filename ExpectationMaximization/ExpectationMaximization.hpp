#ifndef _EM_
#define _EM_

#include <vector>
#include <set>
#include <fstream>
#include <cmath>
#include "Definicoes.hpp"

struct Amostra
{
    unsigned short Peso;
    double Altura;
    double IMC;
    std::string Classe;
};

struct Cluster
{
    std::string Rotulo;
    std::vector<const Amostra*> Amostras{};
    double Media, Desvio;
};

class ExpectationMaximization
{
private:
    std::vector<Amostra> Amostras{};
    Cluster Clusters[NumClasses];
    double Verossimilhanca(double imc, const double media, const double desvio)
    {
        double div{1 / (std::sqrt(2 * PI * (desvio * desvio)))};
        double exp{((std::pow((imc - media), 2)) * -1) / (2 * (desvio * desvio))};
        return (div * (std::pow(NumeroDeEuler, exp)));
    }
    double IMC(const double peso, const double altura) {return (peso / (altura * altura));}
    void DistribuirAmostras()
    {
        for (std::vector<Amostra>::const_iterator it{this->Amostras.cbegin()}; it < this->Amostras.cend(); it++)
        {
            std::set<std::pair<double, unsigned short>> veros;
            for (unsigned short i{0}; i < NumClasses; i++)
                veros.emplace(std::make_pair(this->Verossimilhanca(it->IMC, this->Clusters[i].Media, this->Clusters[i].Desvio), i));
            this->Clusters[veros.crbegin()->second].Amostras.push_back(&(*it));
        }
        this->RecalcularGaussianos();
    }
    void RecalcularGaussianos()
    {
        for (unsigned short i{0}; i < NumClasses; i++)
        {
            // Recalcular Média:
            double media{0};
            for (std::vector<const Amostra*>::const_iterator it{this->Clusters[i].Amostras.cbegin()}; it < this->Clusters[i].Amostras.cend(); it++)
                media += (*it)->IMC;
            if (media > 0)
                this->Clusters[i].Media = (media / this->Clusters[i].Amostras.size());
            else
                this->Clusters[i].Media = 25;
            // Recalcular Desvio:
            double desvio{0};
            for (std::vector<const Amostra*>::const_iterator it{this->Clusters[i].Amostras.cbegin()}; it < this->Clusters[i].Amostras.cend(); it++)
                desvio += std::pow(((*it)->IMC - this->Clusters[i].Media), 2);
            if (desvio > 0)
                this->Clusters[i].Desvio = std::sqrt(desvio / this->Clusters[i].Amostras.size());
            else
                this->Clusters[i].Desvio = (this->Amostras.size() / NumClasses);
        }
    }
    bool ReposicionarAmostras()
    {
        bool ret{false};
        for (unsigned short i{0}; i < NumClasses; i++) // i aponta para cada cluster
        {
            // Calcular a verossimilhança do elemento em cada cluster:
            std::set<std::pair<double,unsigned short>> dists{};
            for (std::vector<const Amostra*>::const_iterator it{this->Clusters[i].Amostras.cbegin()}; it < this->Clusters[i].Amostras.cend(); it++) // it aponta para cada Amostra do cluster
            {    
                for (unsigned short j{0}; j < NumClasses; j++) // j aponta para cada cluster para calcular a verossimilhança
                    dists.emplace(std::make_pair(this->Verossimilhanca((*it)->IMC, this->Clusters[j].Media, this->Clusters[j].Desvio), j));
                // Trocar o elemento de cluster se necessário:
                if (dists.crbegin()->second != i)
                {
                    ret = true;
                    this->Clusters[dists.crbegin()->second].Amostras.push_back(*it);
                    this->Clusters[i].Amostras.erase(it);
                }
            }
        }
        return ret;
    }
    void RotularClusters()
    {
        for (unsigned short i{0}; i < NumClasses; i++)
        {
            // Contar ocorrências de cada classe:
            unsigned short cont[NumClasses]{0,0,0,0,0};
            for (std::vector<const Amostra*>::const_iterator it{this->Clusters[i].Amostras.cbegin()}; it < this->Clusters[i].Amostras.cend(); it++)
            {
                if ((*it)->Classe == "Subpeso")
                    cont[0]++;
                else if ((*it)->Classe == "Normal")
                    cont[1]++;
                else if ((*it)->Classe == "Sobrepeso")
                    cont[2]++;
                else if ((*it)->Classe == "Obesidade")
                    cont[3]++;
                else if ((*it)->Classe == "ObesidadeGrave")
                    cont[4]++;
            }
            // Determinar qual classe é mais frequente:
            unsigned short maior = 0, indice = 0;
            for (unsigned short j{0}; j < NumClasses; j++)
                if (cont[j] > maior)
                {
                    maior = cont[j];
                    indice = j;
                }
            // Inserir rótulo:
            switch(indice)
            {
                case 0: this->Clusters[i].Rotulo = "Subpeso"; break;
                case 1: this->Clusters[i].Rotulo = "Normal"; break;
                case 2: this->Clusters[i].Rotulo = "Sobrepeso"; break;
                case 3: this->Clusters[i].Rotulo = "Obesidade"; break;
                case 4: this->Clusters[i].Rotulo = "Obesidade Grave"; break;
            }
        }
    }
public:
    ExpectationMaximization(const std::string arquivo)
    {
        // Entrada de dados:
        std::ifstream fs{arquivo};
        for (unsigned short i{0}; i < NumEntradas; i++)
        {
            unsigned short peso;
            double altura;
            std::string classe;
            fs >> peso >> altura >> classe;
            Amostra a;
            a.Peso = peso;
            a.Altura = altura;
            a.IMC = this->IMC(peso, altura);
            a.Classe = classe;
            this->Amostras.push_back(a);
        }
        // Inicialização dos Gaussianos:
        std::srand((unsigned)time(0));
        for (unsigned short i{0}; i < NumClasses; i++)
        {
            this->Clusters[i].Media = this->Amostras[std::rand() % this->Amostras.size()].IMC;
            this->Clusters[i].Desvio = (this->Amostras.size() / NumClasses);
        }
    }
    void ProcessarEntradas()
    {
        this->DistribuirAmostras();
        while (this->ReposicionarAmostras())
            this->RecalcularGaussianos();
        this->RotularClusters();
    }
    std::string Classificar(const double peso, const double altura)
    {
        double imc{this->IMC(peso, altura)};
        // Calcular de qual cluster a entrada está mais próxima:
        std::set<std::pair<double,unsigned short>> veros{};
        for (unsigned short i{0}; i < NumClasses; i++)
            veros.emplace(std::make_pair((this->Verossimilhanca(imc, this->Clusters[i].Media, this->Clusters[i].Desvio)), i));
        // Retorna o rótulo da classe mais provável
        return this->Clusters[veros.crbegin()->second].Rotulo;
    }
};

#endif