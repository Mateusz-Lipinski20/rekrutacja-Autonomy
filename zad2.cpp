#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath> 
using namespace std;


 
struct Pomiar{  //struktura reprezentujaca stan telemetryczny bolidu
    double t;   //czas
    double x;   //wspolrzedna x - pozycja wzdłużna (oś +X w przód)
    double y;   //wspolrzedna y - pozycja poprzeczna (oś +Y w lewo)
}; 



vector<Pomiar> wczytajCSV(const string& sciezka) {    // Funkcja wczytująca plik CSV
    vector<Pomiar> dane;
    ifstream plik(sciezka);

    if (!plik.is_open()) {
        cerr << "Blad otwarcia pliku: " << sciezka << endl;    
        return dane;
    }

    string linia;  //Pomijamy nagłówek (pierwszą linię: timestamp,x,y)
    getline(plik, linia);

    //Czytamy wiersz po wierszu
    while (getline(plik, linia)) {    //Czytamy wiersz po wierszu
        if (linia.empty()) continue;

        stringstream ss(linia);
        string s_t, s_x, s_y;   //Zmienne pomocnicze na odczytane pola z wiersza

        
        if (getline(ss, s_t, ',') &&  // Rozbijamy linię po przecinkach: t, x, y
            getline(ss, s_x, ',') &&  //czytamy i wsadzamy posczegolne dane do zmiennych
            getline(ss, s_y, ',')) {
            
            Pomiar p;    //tworzymy nasz pojedynczy pomiar 
            p.t = stod(s_t);  //i zamieniamy dane z tekstu na double zeby moc na nich operować
            p.x = stod(s_x);
            p.y = stod(s_y);
            dane.push_back(p);
        }
    }

    plik.close();
    return dane;   //zamykamy plik i zwracamy wektor z danymi 
}


Pomiar Interpoluj(vector<Pomiar>& dane, double T){    //funkcja wyzanczajaca ciagła pozycje bolidu
    if(T<=dane.front().t)                             // funkcja ta na na podstawie dwóch sąsiednich 
        return dane.front();                          // próbek czasowych, (przed i po zadanym czasie T),
    if(T>=dane.back().t)                              // na podstawie wspolczynnika alfa rozmieszcza w drodze pomiedzy 
        return dane.back();                           //probkami nowy punkt i przypisuje mu wspolrzedne 

    for(int i=0;i<(int)dane.size()-1;i++){
        if(dane[i].t<=T && T<=dane[i+1].t){
            double dt = dane[i+1].t-dane[i].t;
            double alfa = (T-dane[i].t)/dt;           //wspolczynnik proporcji postepu w przedziale [0.0-1.0] 

            Pomiar wynik;
            wynik.t=T;
            wynik.x = dane[i].x + alfa*(dane[i+1].x-dane[i].x);
            wynik.y = dane[i].y + alfa*(dane[i+1].y-dane[i].y);
            return wynik; 
        }
    }
    return dane.back();
}


int main(){
    vector<Pomiar> sensor_1 = wczytajCSV("sensor1.csv");     //wczutujemy sobie dane z obu sensorow
    vector<Pomiar> sensor_2 = wczytajCSV("sensor2.csv");     //i wsdzamy je do kontenerow 


    for(int i=0;i<(int)sensor_2.size();i++){   //na poczatku po obserwacjach musimy odwrocic wszystkie Y 
        sensor_2[i].y=-sensor_2[i].y;          //w sensorze dwa po zostały zmierzone z przeciwnym znakiem 
    }

    for (int i = 1; i < (int)sensor_2.size() - 1; i++) {                    //eliminujemy bledy grube z sensora 2
        double srednia_x = 0.5 * (sensor_2[i - 1].x + sensor_2[i + 1].x);   //usuwamy nagle skoki gps
        double srednia_y = 0.5 * (sensor_2[i - 1].y + sensor_2[i + 1].y);   //liczymy srednia, czyli gdzie powinien sie znalezc punkt 
        double skok = hypot(sensor_2[i].x - srednia_x, sensor_2[i].y - srednia_y);  //pomiedzy wczesniejszym i nastepnym punktem
                        //nastepnie hypot liczy tw. Pitagorasa, czyli rzeczywista odl. miedzy srednia zmierzona i tym co pokazal czujnik
        if (skok > 2.0) {   //i jezeli ten skok jest wiekszy niz 2m to znaczy ze to blad gruby, bo maksymalne ugięcie łuku toru bolida 
            sensor_2[i].x = srednia_x;    //przy kroku 0.5 s wynosi ok. 0.6 mc ,
            sensor_2[i].y = srednia_y;
        }
    }

    int n = (int)sensor_1.size();               //tutaj wyznaczamy rozbieznosc pozycji wzdluznej i poprzecznej gestego sensora 1
    vector<double> surowy_blad_x(n, 0.0);       //wzgledem stabilnego sensora 2 
    vector<double> surowy_blad_y(n, 0.0);       //zainicjowalismy vectory na n pozycji w ktorej kazda wartosc to 0.0
    for (int i = 0; i < n; i++) {
        Pomiar ref = Interpoluj(sensor_2, sensor_1[i].t); //tutaj porownujemy gdzie byl odczyt z sensora 1 w senorze dwa
        surowy_blad_x[i] = ref.x - sensor_1[i].x;  //i nasz surowy blad wynosi odczyt z sensora 2 - odczyt z sensora 1 
        surowy_blad_y[i] = ref.y - sensor_1[i].y;  
    }

    double alfa_filtr = 0.08;   //ustawiamy sile tlumienia naszego filtru 
    vector<double> blad_x = surowy_blad_x;   //i tworzymy vectory bedace kopiami 
    vector<double> blad_y = surowy_blad_y;   //naszych surowych bledow zeby nie starcic danych

    for (int i = 1; i < n; i++) {   //przejscie w przod: wstepne tlumienie szumy i wygladzenie trajektorii bledu
        blad_x[i] = alfa_filtr * blad_x[i] + (1.0 - alfa_filtr) * blad_x[i - 1];  //teraz nakladamy filtry na te bledu
        blad_y[i] = alfa_filtr * blad_y[i] + (1.0 - alfa_filtr) * blad_y[i - 1];  //bo chcemy zrobic lagodna fale 
    }

    blad_x[n - 1] = surowy_blad_x[n - 1]; //ostatni punkt ustawiamy z surowym uchybem zeby meta byla faktycznie
    blad_y[n - 1] = surowy_blad_y[n - 1]; //w tym miejscu gdzie był koniec na sensorze 2

    for (int i = n - 2; i >= 0; i--) {  //przejscie w tył: znosimy opoznienie powstałe w kroku filracji w przod
        blad_x[i] = alfa_filtr * blad_x[i] + (1.0 - alfa_filtr) * blad_x[i + 1];
        blad_y[i] = alfa_filtr * blad_y[i] + (1.0 - alfa_filtr) * blad_y[i + 1];
    }

    for (int i = 0; i < n; i++) {
        sensor_1[i].x += blad_x[i];  //nasze poprawki stosujemy do oczytów z sensora pierwszego zeby uzyskac 
        sensor_1[i].y += blad_y[i];  //odfiltrowana idealna linie z licznymi (gestymi) odczytami pozycji w czasie 
    }

    
    double T;                               //finalne zadanie kodu, czyli petla pytajaca w kolko o 
    while (true){                           //pozycje bolidu w zadanym przez uzytkownika momencie czasu 
        cout << "<< Podaj T" << endl;       //jezeli jest to pozycja spoza zakresu to uzytkownik dostaje infomracje
        cout << ">> ";                      //zwrotna ze wybrał czas spoza zakresu przejazdu bolidu
        if (!(cin >> T)) break;

        if (T < 0.0 || T > 33.0) {
            cout << "<< Wartosc spoza zakresu!" << endl;
        } else {
            Pomiar pozycja = Interpoluj(sensor_1, T);
            cout << fixed << setprecision(3);
            cout << "<< " << pozycja.x << " " << pozycja.y << endl;
        }
    }
    return 0; 
}