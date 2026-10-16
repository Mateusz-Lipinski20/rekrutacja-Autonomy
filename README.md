Rekrutacja PWR Racing Team – Podprojekt Autonomy (Zadanie 1 i 2)

1. Zastosowane narzędzia:
Język programowania: C++ (standard C++11/17) – implementacja głównych algorytmów obliczeniowych (`zad1.cpp`, `zad2.cpp`).
Kompilator i środowisko: GCC (`g++` MinGW-w64), Visual Studio Code.
Analiza danych i wizualizacja: Python 3 (środowisko Spyder) z wykorzystaniem bibliotek `pandas` oraz `matplotlib` do weryfikacji sygnałów telemetrycznych i wygenerowania wykresów dowodowych.


Zadanie 1 – Deduplikacja pachołków i najkrótsza otoczka

Sposób działania:
1. Program pobiera od użytkownika nazwę pliku tekstowego i wczytuje liczbę pachołków 'n', minimalny dopuszczalny dystans 'm' oraz współrzędne '(id, x, y)' każdego pachołka.
2. Podpunkt 1.1 (Deduplikacja): Każdy pachołek trafia początkowo do osobnego, jednoelementowego klastra (`vector<vector<Pacholek>>`). Pętla iteracyjnie przegląda pary grup i łączy je ze sobą, jeśli odległość między dowolnymi dwoma pachołkami z tych grup jest mniejsza niż 'm'. Dla grup połączonych wyznaczana jest nowa pozycja (średnia arytmetyczna współrzędnych 'x' i 'y'), nadawane jest nowe unikalne ID (kolejne liczby powyżej 'n'), a stare ID są sortowane i wypisywane na ekranie.
3. Podpunkt 1.2 (Najkrótsza otoczka): Na zaktualizowanym zbiorze pachołków (po deduplikacji) wyznaczana jest otoczka wypukła. Program oblicza obwód powstałego wielokąta i zwraca go wraz z listą ID pachołków wchodzących w skład otoczki.

Zastosowane algorytmy:
Iteracyjne klastrowanie przechodnie (Single-Linkage Clustering): Zastosowane do grupowania blisko leżących odczytów pachołków. Pozwala poprawnie połączyć w jeden klaster więcej niż dwa punkty (np. pachołki `[4, 9, 10]`).
Algorytm Andrew (Monotone Chain – Convex Hull): Wykorzystany do wyznaczenia najkrótszej otoczki wypukłej. Punkty są sortowane po współrzędnej 'X', a w razie równości po 'Y', a następnie za pomocą iloczynu wektorowego budowana jest dolna oraz górna część otoczki.

Napotkane edge case'y i problemy:
1. Przechodniość relacji bliskości (łączenie kaskadowe): Jeśli pachołek A leży blisko B, a B leży blisko C, wszystkie trzy muszą utworzyć jeden wspólny pachołek, nawet jeśli A i C leżą od siebie nieco dalej niż 'm'. Rozwiązano to poprzez strukturę wektora grup i restartowanie pętli po każdym scaleniu.
2. Optymalizacja i precyzja zmiennoprzecinkowa:** Zamiast kosztownej funkcji pierwiastkowania `sqrt()`, w warunku odległości zastosowano porównanie kwadratów odległości. Przy wyznaczaniu otoczki wypukłej uwzględniono tolerancję numeryczną `1e-9` dla punktów niemal współliniowych.

Dowód poprawności działania:
Dla załączonego pliku `example_input.txt` program generuje wyjście w pełni zgodne ze specyfikacją zadania:

CONE 13 10.300 0.400 [2,8]
CONE 14 5.067 9.700 [4,9,10]
ENCLOSURE 39.162 [1,11,13,3,14,5,12]


Zadanie 2 – Lokalizacja i fuzja sensorów na torze

*(Pełna analiza źródeł błędów do podpunktu 2.1 znajduje się również w osobnym dokumencie `Zadanie 2_1.pdf`).*

Sposób działania:
1. Program wczytuje dane z plików `sensor1.csv` (~40 Hz, 1339 próbek) oraz `sensor2.csv` (~2 Hz, 67 próbek).
2. Odwracany jest znak współrzędnej poprzecznej Y w Sensorze 2 (y := -y).
3. Dane z Sensora 2 są czyszczone z błędów grubych (pojedynczych skoków pomiarowych).
4. Dla każdej próbki czasowej z Sensora 1 wyznaczana jest pozycja referencyjna z Sensora 2 za pomocą interpolacji liniowej i obliczany jest surowy uchyb w osiach X oraz Y.
5. Uchyb zostaje wygładzony dwukierunkowym filtrem dolnoprzepustowym i dodany do pomiarów Sensora 1, co eliminuje dryf przy zachowaniu gładkiej geometrii zakrętów.
6. Program uruchamia interaktywną pętlę w terminalu, która przyjmuje czas T z zakresu [0.000, 33.000 s] i zwraca dokładną pozycję bolidu (x, y).

Zastosowane algorytmy:
Interpolacja liniowa (Linear Interpolation / LERP): Wyznaczanie pozycji pośrednich pomiędzy dyskretnymi pomiarami czasowymi na podstawie wagi proporcjonalnej alfa.
Filtr sąsiedztwa (Outlier Rejection): Wykrywanie skoków pomiarowych w Sensorze 2 przekraczających próg 2.0 m względem średniej z sąsiednich próbek i zastępowanie ich wartością uśrednioną.
Dwukierunkowy filtr dolnoprzepustowy (Zero-Phase Low-Pass Filter): Filtracja uchybu w przód oraz w tył ze współczynnikiem alfa_filtr = 0.08 i ustawieniem na sztywno punktu końcowego.

Napotkane edge case'y i problemy:
1. Odwrócona polaryzacja osi Y w Sensorze 2: Zgodnie z regulaminem Formula Student przejazd Skidpad rozpoczyna się od dwóch okrążeń prawego koła (wartości Y < 0). Sensor 2 raportował na starcie wartości dodatnie.
2. Asymetryczny dryf Sensora 1 w osi X:** Przez pierwsze 15 s Sensor 1 działał poprawnie, natomiast podczas przejścia przez środek ósemki (t ok. 15-18 s) z powodu uślizgu opon i całkowania odometrii środek drugiego okręgu opadł o ok. 5.8 m. Zamiast sztucznego kodowania przedziałów czasowych „na sztywno”, zastosowano ciągłą filtrację uchybu względem Sensora 2.
3. Zjawisko „ścinania zakrętów” (Chord Error): Bezpośrednie zastąpienie współrzędnej X w Sensorze 1 wartościami wyinterpolowanymi z rzadkiego Sensora 2 (2 Hz) powodowało deformację okręgów (prowadzenie trasy po cięciwach wewnątrz zakrętu). Zastosowanie filtru dolnoprzepustowego na samym uchybie pozwoliło podnieść opadnięte koło bez zniekształcania idealnego, gładkiego łuku z Sensora 1 (40 Hz).
4. Zapytania w momentach pomiędzy próbkami oraz na krańcach zakresu: Zabezpieczono funkcję interpolacji warunkami brzegowymi dla T = 0.000 oraz T = 33.000, a wartości spoza zakresu są odrzucane komunikatem "Wartosc spoza zakresu!".

Wizualizacja i dowód poprawności rozwiązania:

Porównanie surowych sygnałów wejściowych (przed korekcją):
(porowanie_sensorow.png)

Zrekonstruowana trajektoria bolidu po fuzji sensorów (wynik działania programu):
(prawidlowa_pozycja_bolidu.png)


Instrukcja kompilacji i uruchomienia (Windows / PowerShell)

Upewnij się, że pliki wejściowe (`example_input.txt`, `sensor1.csv`, `sensor2.csv`) znajdują się w tym samym katalogu, co pliki źródłowe (`zad1.cpp`, `zad2.cpp`).

Zadanie 1

Kompilacja:
g++ -o zadanie1 zad1.cpp
Uruchamianie:
.\zadanie1.exe

Po uruchomieniu program poprosi o podanie nazwy pliku wejściowego, wiec wpisujemy:
example_input.txt

Zadanie 2

Kompilacja:
g++ -o zadanie2 zad2.cpp
Uruchamianie:
.\zadanie2.exe

Po uruchomieniu w terminalu będziemy mogli wybrać liczbę z danego zakresu. Przykład zabawy z petlą jest poniżej:
<< Podaj T
>> 38.163
<< Wartosc spoza zakresu!
<< Podaj T
>> 26.821
<< 7.225 11.050

