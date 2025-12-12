# ᗧ••• Pac-Man Clone (C + Raylib)

![GitHub repo size](https://img.shields.io/github/repo-size/SEU_USUARIO/NOME_DO_REPOSITORIO?style=for-the-badge&color=800080)
![GitHub language count](https://img.shields.io/github/languages/count/SEU_USUARIO/NOME_DO_REPOSITORIO?style=for-the-badge&color=800080)
![License](https://img-shields.io/badge/License-MIT-blue.svg?style=for-the-badge)

> Um clone do clássico jogo Pac-Man desenvolvido em **C** utilizando a biblioteca gráfica **Raylib**. O projeto implementa lógica de fantasmas, gerenciamento de mapa, placar e persistência de dados.

---

### 📋 Pré-requisitos e Instalação da Raylib

Para compilar, você precisará de um compilador C (GCC) e da biblioteca Raylib instalada.

#### 1. Instalação no Linux (Debian/Ubuntu)

```bash
# Atualiza os pacotes e instala ferramentas essenciais (GCC)
sudo apt update
sudo apt install build-essential

# Instala a biblioteca Raylib
sudo apt install libraylib-dev

gcc src/*.c -o pacman.exe -lraylib -lopengl32 -lgdi32 -lwinmm

./pacman.exe
