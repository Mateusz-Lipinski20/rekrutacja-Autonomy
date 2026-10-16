#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm> // do sortowania starych ID
#include <iomanip>   // do zaokrąglenia do 3 miejsc po przecinku
#include <cmath>     // jeśli używasz std::hypot
using namespace std; 



struct Pacholek{
    int id;
    double x;   // tworzymy strukture ktora trzyma w sobie infortmacje o id_pacholka i jego poszczeglnych wspolrzednych 
    double y; 
};

bool czy_minimalnaOdleglosc(Pacholek& a,Pacholek& b,double m){  //sprawdzamy czy odległosc miedzy dwoma pacholkami jest dopuszczalna (>=m) 
    double pierwsza_wsp = a.x - b.x;                            //zastosowałem kwadrat zeby uniknac liczenia pierwiastkow 
    double druga_wsp = a.y - b.y;                             
    return (pierwsza_wsp*pierwsza_wsp+druga_wsp*druga_wsp)>=(m*m);   
}   

double iloczynWektorowy(Pacholek& a,Pacholek& b,Pacholek& c){  //liczymy iloczyn wektorowy, jezeli iloczyn>0 to skret w lewo
    return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);            //iloczyn<0 to w prawo, jezeli iloczyn=0 to punkty sa wspolliniowe 
}                                                              //my szukamy skretu w lewo, czyli przeciwnego do wskazowek zegara 

double Odleglosc(Pacholek& a, Pacholek& b) {  //funkcja liczaca odleglosc miedzy dwoma punktami,
    double dx = a.x - b.x;                    //przyda nam sie do policzenia obwodu 
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}



int main(){
    string nazwa_pliku;  

    cout<<"Podaj nazwe pliku z rozszerzeniem: ";
    cin>>nazwa_pliku;         //pobieramy od uzytkownika nazwe pliku w ktorym trzyma dane odnosnie pacholkow 

    std::ifstream plik(nazwa_pliku);  
    
    if(!plik.is_open()){
        cout<<"Nie udalo sie otworzyc pliku: "<<nazwa_pliku<<endl;  //jezeli uzytkownik poda bledna nazwe pliku natychmiast konczymy program
        return 1;
    }

    int n;  //liczba punktów w zbiorze
    double m;    //minimalna dopuszczalna odległosc miedzy dwoma pacholkami 
    plik>>n>>m; //wczutujemy liczbe pkt i dopuszczalna odleglosc z pliku .txt


    vector<vector<Pacholek>> Grupy; //kazdy pacholek jest osobno umieszczany w vektorze, czyli tworzymy liste dwuwymiarowa 
    Grupy.reserve(n); //dla optymalizacji rezerujemy wielkosc tablicy Grupy na tyle ile jest ogolnie pacholkow  

    for (int i=0;i<n;i++){  
        Pacholek p;        //czytamy kolejne linijki kodu i tworzymy pacholki majace pola jak nasza struktura i od razu wpakowujemy 
        plik>>p.id>>p.x>>p.y;  // je do listy dwuwymiarowej zeby kazdy pacholek byl osobno 
        Grupy.push_back({p});  //lista inicjalizacyjna tworzy element w locie i mozna zaoszczedzic linijki kodu 
    }
    plik.close();

    bool czyScalono=true;
    while (czyScalono){     //generalnie petla działa w ten sposob ze dopoki Grupy miedzy ktorymi odleglosc jest mnijesza niz dopuszczalna to 
        czyScalono=false;    //scalamy je w jedna grupe 
        for(int i=0;i<Grupy.size();i++){
            for(int j=i+1;j<Grupy.size();j++){
                bool saBlisko= false;
                for (int a=0;a<Grupy[i].size();a++){  //tutaj sprawdzamy czy jakikolwiek element z grupy i jest w odleglosci >= m z elementem z j
                    for(int b=0;b<Grupy[j].size();b++){  //jezeli tak to nic jezeli nie to przerywamy
                        if (czy_minimalnaOdleglosc(Grupy[i][a],Grupy[j][b],m)){
                        }else{ 
                            saBlisko=true;
                            break;
                        }
                    } if (saBlisko) break; 
                } if (saBlisko){    // jezeli wlasnie te nasze elementy sa blisko siebie to do Grupy i na koniec trafiaja elementy z Grupy j 
                    Grupy[i].insert(Grupy[i].end(),Grupy[j].begin(),Grupy[j].end());  
                    Grupy.erase(Grupy.begin()+j);  //nastepnie usuwamy z vectora Grupy cala grupe z pozycji j 
                    czyScalono=true;
                    break; 
                }
            } if (czyScalono) break; //i jezeli juz w tym obiegu scalilismy grupy to przerywamy i od nowa zaczynamy sprawdzac, bo indeksy sie przesuwaja
        }
    }

    int nowe_n=n; //tworzymy pomocnicza zmianna ilosc pacholkow, przyda nam sie do id 
    vector<Pacholek> koncowe_pacholki;   //tworzymy wektor z koncowymi ustawieniami pacholkow 
    cout<<fixed<<setprecision(3); //ustawiamy dokladnosc wyswietlania liczb po przecinku na 3


    for (int i=0;i<Grupy.size();i++){   //teraz robimy petle ktora wpisuje nasze stare pacholki do nowego vectora
        if(Grupy[i].size()>1){          //i zapamieta te ktore zostały usuniete i w ich miejsce powstal nowy 
            double suma_x=0;   //jezeli w wektorze bylo wiecej niz jeden pacholek to wchodzimy w tego if 
            double suma_y=0;   //na poczatku tworzymy sobie sume wspolrzednych x i y 
            vector<int> scalone_id;   //oraz tworzymy wektor na scalone id 
            for(int j=0;j<Grupy[i].size();j++){   //dodajemy sobie wspolrzedne tych starych pacholkowe i zapamietujemy wszystkie id 
                suma_x+=Grupy[i][j].x;
                suma_y+=Grupy[i][j].y;
                scalone_id.push_back(Grupy[i][j].id);}
            nowe_n+=1;      
            sort(scalone_id.begin(), scalone_id.end());   //sortujemy w kolejnosci rosnacej nasze stare id 

            double srednia_x=suma_x/Grupy[i].size();    //wyliczamy pozycje usrednionego pacholka 
            double srednia_y=suma_y/Grupy[i].size();

            cout<<"CONE "<<nowe_n<<" "<<srednia_x<<" "<<srednia_y<<" [";   //wypisujemy na ekran nowe id  oraz wspolrzedne pacholka oraz 
            for (int k=0;k<scalone_id.size();k++)                          // id pacholkow ktore zostaly scalone zeby stworzyc nowy pacholek
                cout<<scalone_id[k]<<(k + 1 < scalone_id.size() ? "," : "");
            cout<<"]"<<endl;

            Pacholek nowy_Pacholek;    //a tutaj dodajemy te nowe pacholki do wektora z nowymi pacholkami prawidlowo ustawionymi     
            nowy_Pacholek.id=nowe_n;
            nowy_Pacholek.x=srednia_x;
            nowy_Pacholek.y=srednia_y;
            koncowe_pacholki.push_back(nowy_Pacholek);
        }else{
            koncowe_pacholki.push_back(Grupy[i][0]);   //jezeli grupa jest sama to od razu trafia do vectora koncowe pacholki poniewaz
        }                                              //pozycja jednego pacholka nie wymaga korekcji i scalenia 
    }



    for (int i = 0; i < koncowe_pacholki.size(); ++i) {   //ta petla sortuje nam pacholki od lewej do prawej na osi x i y 
        for (int j = i + 1; j < koncowe_pacholki.size(); ++j) { //po posortowaniu mamy z lewej punktu z najmnijeszym x, a jezeli sa takmie same
                                                                //to z najmnijeszym y 
            bool zamien = false;

            if (koncowe_pacholki[j].x < koncowe_pacholki[i].x) {
                zamien = true;
            } 
            else if (koncowe_pacholki[j].x == koncowe_pacholki[i].x) {
                if (koncowe_pacholki[j].y < koncowe_pacholki[i].y) {
                    zamien = true;
                }
            }
            if (zamien) {
                swap(koncowe_pacholki[i], koncowe_pacholki[j]);
            }
        }
    }

    vector <Pacholek> otoczka;     //Konstruujemy otoczke algorytmem Andrew (Monotone Chain)


    for (int i=0;i<koncowe_pacholki.size();i++){
        while (otoczka.size()>=2 && iloczynWektorowy(otoczka[otoczka.size()-2], otoczka[otoczka.size()-1],koncowe_pacholki[i])<=1e-9 ){
            otoczka.pop_back();
        }
        otoczka.push_back(koncowe_pacholki[i]);    //budujemy dolna czesc otoczki sprawdzajac ilosczynem wektorowym czy skret 3 ostatnich punktow
    }                                              //jest w lewo? jeszeli jezeli tak to dokladamy na stos ten punkt, czyli do naszej tablicy


    int dolny_rozmiar=otoczka.size()+1;   //tutaj tak samo budujemy gorna czesc otoczki 
    for (int i=(int)koncowe_pacholki.size()-2;i>=0;i--){
        while(otoczka.size()>=dolny_rozmiar && iloczynWektorowy(otoczka[otoczka.size()-2],otoczka[otoczka.size()-1],koncowe_pacholki[i])<=1e-9){
            otoczka.pop_back();
        }
        otoczka.push_back(koncowe_pacholki[i]);
    }

    otoczka.pop_back();   //usuwamy powielajacy sie pierwszy punkt otoczki  

    double obwod=0;
    for (int i=0;i<(int)otoczka.size()-1;i++){   //liczmy obwod naszej otoczki 
        obwod+=Odleglosc(otoczka[i],otoczka[i+1]); 
    }
    obwod += Odleglosc(otoczka.back(), otoczka[0]);



    cout << "ENCLOSURE " << obwod << " [";    //wyswietlsmy wyniki, jaka jest dlugosc calej otoczki oraz jakie punkty sie na nia skladaja  
    for (int i = 0; i < otoczka.size();i++) {
        cout << otoczka[i].id;
        if (i + 1 < otoczka.size()) {
            cout << ",";
        }
    }
    cout << "]" << endl;

    return 0;
}