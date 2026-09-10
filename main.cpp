#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cfloat>

// struktura danych dla zapisywania punktu w ukladzie wspolrzednych
struct Point {
    double x;
    double y;
};
// struktura danych do zadania 2.3 trzyma 2 punkty i odleglosc miedzy nimi
struct Winner {
    double distance;
    Point p1;
    Point p2;
};
// 2.1 iloczyn wektorowy do sprawdzenia czy punkt lezy na lewo czy prawo od wektora
double cross_product(const Point& a, const Point& b, const Point& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}
// 2.2 dlugosc boku a-b do sprawdzenia dlugosci miedzy prostymi z wzrou na pole trojkata
double triangle_base(const Point& a, const Point& b) {
    return (std::sqrt((b.x - a.x)*(b.x - a.x) + (b.y - a.y)*(b.y - a.y)));
}
// 2.2 wzor na pole trojkata przez iloczyn wektorowy
double distance_for_parallel(double cross_prod, double base) {
    return (std::abs(cross_prod) / base);
}
// 2.3 wzor na odleglosc miedzy 2 punktami
double distance_for_point(const Point& p1, const Point& p2) {
    return (std::sqrt((p2.x - p1.x)*(p2.x - p1.x) + (p2.y - p1.y)*(p2.y - p1.y)));
}
// 2.3 porownanie kazdy z kazdym punktem kiedy mamy <= 3 punkty
Winner less_than_3(const std::vector<Point>& points, int left, int right) {
    Winner best = {DBL_MAX, {0,0}, {0,0}};
    for (int i = left; i <= right; ++i) {
        for (int j = i + 1; j <= right; ++j) {
            double d = distance_for_point(points[i], points[j]);
            if (d < best.distance) {
                best = {d, points[i], points[j]};
            }
        }
    }
    return best;
}
// 2.3 dziel i zwyciezaj do wyznaczenia najblizszych punktow z lepsza wydajnoscia niz porownywanie kazdy z kazdym 
// sortujemy punkty po x
// rekurnecja wbijamy sie tak gleboko zeby podzielic na czesci z 3 lub mniej punktami
// szukamy najmniejszej odleglosci miedzy punktami w danym kawalku
// nastepnie patrzymy na prostokat o ktorego srodkiem jest punkt podzialu o szerokoci 2d gdzie d to najmniejsza odleglosc z lewej i prawej
// wysokosc prostokata to d zeby sprawdzac tylko te punkty ktore moga byc blizej niz d a sortujemy punkty od y wiec lecimy od dolu
Winner closest_pair(const std::vector<Point>& points, int left, int right) {
    int count = right - left + 1;
    if (count <= 3) {
        return less_than_3(points, left, right);
    }
    int mid = (left + right) / 2;
    Winner left_winner = closest_pair(points, left, mid);
    Winner right_winner = closest_pair(points, mid + 1, right);
    Winner best = (left_winner.distance < right_winner.distance) ? left_winner : right_winner;
    std::vector<Point> vertical_strip;
    for (int i = left; i <= right; ++i) {
        if (std::abs(points[i].x - points[mid].x) < best.distance) {
            vertical_strip.push_back(points[i]);
        }
    }
    std::sort(vertical_strip.begin(), vertical_strip.end(), [](const Point& a, const Point& b) {
        return a.y < b.y;
    });
    for (int i = 0; i < vertical_strip.size(); ++i) {
        for (int j = i + 1; j < vertical_strip.size() && (vertical_strip[j].y - vertical_strip[i].y) < best.distance; ++j) {
            double d = distance_for_point(vertical_strip[i], vertical_strip[j]);
            if (d < best.distance) {
                best = {d, vertical_strip[i], vertical_strip[j]};
            }
        }
    }
    return best;
}

    

int main() {
    std::string nazwa;
    std::cout << "Podaj nazwe pliku: ";
    std::cin >> nazwa;
    std::ifstream file(nazwa);

    if (!file.is_open()) {
        std::cerr << "Nie mozna otworzyc pliku" << std::endl;
        return 1;
    }

    // sprawdzenie ilosci punktow zeby wiedziec czy w ogole da sie liczyc
    int n;
    if (!(file >> n)) {
        std::cerr << "Blad: Nie udalo sie odczytac liczby punktow" << std::endl;
        return 1;
    }
    if (n <= 2) {
        std::cerr << "Za mala ilosc punktow" << std::endl;
        return 1;
    }

    // tablica zeby przechowywac punkty
    std::vector<Point> points;
    points.reserve(n);

    for (int i = 0; i < n; ++i) {
        Point p;
        if (file >> p.x >> p.y) {
            points.push_back(p);
        }
    }
    file.close();

    std::sort(points.begin(), points.end(), [](const Point& a, const Point& b) {
        if (a.x != b.x) return a.x < b.x;
        return a.y < b.y; 
    });

    // jezeli crossproduct > 0 to jest skret w lewo, jezeli < 0 to w prawo
    // jezeli == 0 to sa wspolliniowe
    
    // stos -dolna czesc otoczki
    std::vector<Point> lower;
    
    for (const auto& p : points) {
        // dopóki mamy z kim porównać I zakręt nie jest w lewo
        // punkt jest w srodku otoczki to wylatuje
        while (lower.size() >= 2 && cross_product(lower[lower.size() - 2], lower.back(), p) <= 0) {
            lower.pop_back();
        }

        // punkt jest w otocze wpada do nas
        lower.push_back(p);
    }

    // to samo robimy dla gornej czesci otoczki od tylu
    std::vector<Point> upper;
    for (int i = points.size() - 1; i >= 0; --i) {
        const auto& p = points[i];
        while (upper.size() >= 2 && cross_product(upper[upper.size() - 2], upper.back(), p) <= 0) {
            upper.pop_back();
        }
        upper.push_back(p);
    }
    // usuwamy duplikaty bo ostatni element z dolu i gornej czesci sa te same
    lower.pop_back();
    upper.pop_back();

    // scalamy otoczke
    std::vector<Point> hull = lower;
    hull.insert(hull.end(), upper.begin(), upper.end());

    std::cout << "2.1 Punkty tworzace otoczke:\n";
    for (const auto& p : hull) {
        std::cout << p.x << " " << p.y << std::endl;
    }
    // zadanie najwezsze pasmo
    int h = hull.size();
    // duza wartosc do porownan z malymi
    double min_width = DBL_MAX;
    // 3 punkty z ktorych wyznaczamy
    Point best_a, best_b, best_p;

    for (int i = 0; i < h; ++i) {
        Point a = hull[i];
        // modulo zeby nie wyjsc poza indeks do sprawdzenia ostatniego punktu z pierwszym
        Point b = hull[(i + 1) % h];
        double base = triangle_base(a, b);

        double max_dist_for_edge = 0.0;
        Point furthest_point;

        // szukamy punktu leżącego najdalej od krawędzi a-b zeby obejmowac wszystkie punkty w prostych
        for (int j = 0; j < h; ++j) {
            double cp = cross_product(a, b, hull[j]);
            double d = distance_for_parallel(cp, base);

            if (d > max_dist_for_edge) {
                max_dist_for_edge = d;
                furthest_point = hull[j];
            }
        }

        // sprawdzamy czy to pasmo jest najwęższe z dotychczasowych
        if (max_dist_for_edge < min_width) {
            min_width = max_dist_for_edge;
            best_a = a;
            best_b = b;
            best_p = furthest_point;
        }
    }

    std::cout << "\n2.2 Najwezsze pasmo: " << min_width << std::endl;

    Winner closest = closest_pair(points, 0, points.size() - 1);
    std::cout << "\n2.3 Para najblizszych punktow: (" << closest.p1.x << ", " << closest.p1.y << "), (" << closest.p2.x << ", " << closest.p2.y << "), " << closest.distance << std::endl;
    
    return 0;
}
