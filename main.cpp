#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <numeric>
#include <thread>

using namespace std;

struct Nodo {
    int x, y;
    char id;
};

void intercambiarAleatorio(std::vector<int>& vec) {
    srand(static_cast<unsigned int>(time(0)));

    int n = vec.size();
    if (n < 2) return;

    int max_intercambios = std::min(3, n / 2);

    for (int i = 0; i < max_intercambios; ++i) {
        int idx1 = rand() % n;
        int idx2 = rand() % n;

        while (idx1 == idx2) {
            idx2 = rand() % n;
        }
        std::swap(vec[idx1], vec[idx2]);
    }
}

void orderCrossover(vector<int>& parent1, vector<int>& parent2) {
    int n = parent1.size();
    vector<int> child1(n, -1);
    vector<int> child2(n, -1);
    vector<bool> patron(n);
    for (int i = 0; i < n; ++i) patron[i] = rand() % 2;

    for (int i = 0; i < n; i++) {
        if (patron[i]) {
            child1[i] = parent2[i];
            child2[i] = parent1[i];
        }
    }

    auto it_aux = parent1.begin();
    for (auto& elem : child1) {
        if (elem == -1) {
            while (it_aux != parent1.end() && std::find(child1.begin(), child1.end(), *it_aux) != child1.end()) {
                ++it_aux;
            }
            if (it_aux != parent1.end()) {
                elem = *it_aux;
                ++it_aux;
            }
        }
    }

    auto it_aux2 = parent2.begin();
    for (auto& elem : child2) {
        if (elem == -1) {
            while (it_aux2 != parent2.end() && std::find(child2.begin(), child2.end(), *it_aux2) != child2.end()) {
                ++it_aux2;
            }
            if (it_aux2 != parent2.end()) {
                elem = *it_aux2;
                ++it_aux2;
            }
        }
    }

    parent1.assign(child1.begin(), child1.end());
    parent2.assign(child2.begin(), child2.end());
}

void sumar_recorrido(vector<int>& recorridos, vector<vector<double>> distancias, vector<vector<int>> camino, int n, int i) {
    for (int j = 0; j < n - 1; j++) {
        recorridos[i] += distancias[camino[i][j]][camino[i][j + 1]];
    }
    recorridos[i] += distancias[camino[i][0]][camino[i][n - 1]];
}

double calcularDistancia(Nodo& a, Nodo& b) {
    return std::sqrt(std::pow(b.x - a.x, 2) + std::pow(b.y - a.y, 2));
}

void dibujarNodosYAristas(sf::RenderWindow& window, const vector<Nodo>& nodos, const vector<vector<double>>& distancias, const vector<int>& recorrido) {
    for (const auto& nodo : nodos) {
        // Dibujar el nodo
        sf::CircleShape circle(5); // Radio del nodo
        circle.setPosition(nodo.x, nodo.y);
        circle.setFillColor(sf::Color::Blue);
        window.draw(circle);

        // Dibujar el identificador del nodo
        static sf::Font font; // Fuente estática
        if (font.getInfo().family.empty()) { // Cargar la fuente solo una vez
            if (!font.loadFromFile("arial.ttf")) { // Asegúrate de tener este archivo de fuente
                std::cerr << "Error al cargar la fuente\n";
                return; // Terminar si hay error
            }
        }
        sf::Text text(std::string(1, nodo.id), font, 15);
        text.setFillColor(sf::Color::White);
        text.setPosition(nodo.x, nodo.y);
        window.draw(text);
    }

    for (size_t i = 0; i < nodos.size(); ++i) {
        for (size_t j = i + 1; j < nodos.size(); ++j) {
            if (distancias[i][j] > 0) {
                sf::Vertex line[] = {
                    sf::Vertex(sf::Vector2f(nodos[i].x, nodos[i].y), sf::Color::Red),
                    sf::Vertex(sf::Vector2f(nodos[j].x, nodos[j].y), sf::Color::Red)
                };
                window.draw(line, 2, sf::Lines);
            }
        }
    }
    for (size_t k = 0; k < recorrido.size(); ++k) {
        int start = recorrido[k];
        int end = recorrido[(k + 1) % recorrido.size()];
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(nodos[start].x, nodos[start].y), sf::Color::Green),
            sf::Vertex(sf::Vector2f(nodos[end].x, nodos[end].y), sf::Color::Green)
        };
        window.draw(line, 2, sf::Lines);
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0)));

    int n;
    cout << "Ingrese el numero de nodos: ";
    cin >> n;
    cout << endl;

    vector<Nodo> nodos(n);
    for (int i = 0; i < n; ++i) {
        nodos[i].x = rand() % 1001;
        nodos[i].y = rand() % 1001;
        nodos[i].id = 'A' + i;
    }

    vector<vector<double>> distancias(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double distancia = calcularDistancia(nodos[i], nodos[j]);
            distancias[i][j] = distancia;
            distancias[j][i] = distancia;
        }
    }
    int poblacion = 50;
    vector<vector<int>> camino(poblacion, vector<int>(n));
    for (int i = 0; i < poblacion; i++) {
        iota(camino[i].begin(), camino[i].end(), 0);
    }
    vector<int> recorridos(poblacion, 0);

    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Nodos y Aristas");
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        for (int i = 0; i < poblacion; i++) {
            recorridos[i] = 0;
        }

        vector<thread> hilos(poblacion);
        for (int i = 0; i < poblacion; i++) {
            hilos[i] = thread(sumar_recorrido, ref(recorridos), distancias, camino, n, i);
        }
        for (int i = 0; i < poblacion; i++) {
            hilos[i].join();
        }

        auto min_iter = std::min_element(recorridos.begin(), recorridos.end());
        int indice_min = std::distance(recorridos.begin(), min_iter);

        window.clear(sf::Color::Black);
        dibujarNodosYAristas(window, nodos, distancias, camino[indice_min]);
        vector<thread> hilos2(poblacion);
        for (int i = 0;i < poblacion;i++)
        {
            if (i != indice_min) hilos2[i] = thread(intercambiarAleatorio, ref(camino[i]));
        }
        for (int i = 0;i < poblacion;i++) if (i != indice_min) hilos2[i].join();
        /*for (int j = 0; j < poblacion; j++)
        {
            for (int i = 0;i < n;i++)
            {
                if (j == indice_min)
                {
                cout << camino[j][i] << "#";
                continue;
                }
                cout << camino[j][i] << " ";
            }
            cout << recorridos[j] << endl;
        }
        */
       
        window.display();
    }

    return 0;
}
