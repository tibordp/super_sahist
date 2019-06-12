#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <map>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <thread>
#include <mutex>
#include <vector>

using namespace std;
using namespace std::rel_ops;

namespace Sahist {

enum class Figure {
	nic, 
	kralj, kraljica, trdnjava,
	tekac, konj, kmet
};

enum class Barva {
	nobena, bela, crna
};

enum class Poteze {
	konec, standardna, velika_rosada, mala_rosada, promocija, mimohod
};

struct Figura {
	Figure tip;
	Barva barva;
	
	bool operator==(Figura rhs) const;

	static Figura prazna()  {
		return {Figure::nic, Barva::nobena };
	}

	static Barva nasprotna_barva(Barva barva) {
		return (barva == Barva::bela) ? Barva::crna : Barva::bela;
	}
};

struct Koordinata {
	int stolpec;
	int vrstica;
	bool operator==(Koordinata rhs) const;
	Koordinata operator+(Koordinata rhs) const;
	Koordinata& operator+=(Koordinata rhs);
	inline operator bool() const;

	static Koordinata neveljavna(){
		return {-1,-1};
	}

	Koordinata() = default;

	Koordinata(int stolpec_, int vrstica_) 
		: stolpec(stolpec_), vrstica(vrstica_) 
	{}
};

struct Poteza {
	Poteze tip;
	Koordinata od_koordinata;
	Koordinata do_koordinata;
	Figure tip_figure;

	bool operator==(Poteza rhs) const;
};

class Sahovnica {
	array<array<Figura, 8>, 8> deska;
	map<Barva, bool> velike_rosade; 
	map<Barva, bool> male_rosade;
	map<Barva, bool> ima_kralja;

	Koordinata mimohod;



	void inicializiraj();
public:
	/*
		Vrne nam smer premikanja kmeta v odvisnosti od barve
	*/
	static int smer_kmeta(Barva barva) {
		return (barva == Barva::bela) ? 1 : -1;
	}
	/*
		Vrne nam vrstico kmetov v odvisnosti od barve
	*/
	static int zacetek_kmeta(Barva barva) {
		return (barva == Barva::bela) ? 2 : 7;
	}
	/*
		Vrne nam zacetno vrstico v odvisnosti od barve
	*/
	static int zacetna_vrstica(Barva barva) {
		return (barva == Barva::bela) ? 1 : 8;
	}		
	/*
		Vrne nam zadnjo vrstico v odvisnosti od barve
	*/
	static int konec_kmeta(Barva barva) {
		return (barva == Barva::bela) ? 8 : 1;
	}	


	void prestej_figure(unordered_map<Figura, int>&);

	Sahovnica(bool postavi_figure = true);

	Figura& polje(Koordinata pos);
	bool zasedeno(Koordinata pos) const;

	void premakni(Poteza p);
	void dosegljive_pozicije(Koordinata pos, unordered_set<Koordinata>& results, bool samo_zajemi = false);

	void vse_poteze(Barva barva, vector<Poteza>& results);
	bool veljavna_poteza(Poteza poteza);
 	double alfa_beta(Poteza poteza, double alpha, double beta, int globina, Barva maksimiziraj);
 	double oceni_potezo(Poteza poteza, Barva maksimiziraj);
	void dobre_poteze(Barva barva, vector<pair<double, Poteza>>& rezultat);
	Poteza predlagaj(Barva barva);
	bool sah(Barva barva);
	void mozne_poteze(Barva barva, vector<Poteza>& rezultat);
	bool premakni_nakljucno(Barva barva);

};

};


namespace std {
	template<> struct hash<Sahist::Koordinata> {
		inline size_t operator()(Sahist::Koordinata pos) const {
			return pos.stolpec * 8 + pos.vrstica;
		}
	};

	template<> struct hash<Sahist::Poteza> {
		inline size_t operator()(Sahist::Poteza poteza) const {
			return hash<Sahist::Koordinata>()(poteza.od_koordinata) * 64 + 
			hash<Sahist::Koordinata>()(poteza.do_koordinata);
		}
	};

	template<> struct hash<Sahist::Figura> {
		inline size_t operator()(Sahist::Figura figura) const {
			return static_cast<int>(figura.barva) * 10 + // Število figur 
			static_cast<int>(figura.tip);
		}	
	};	
};
