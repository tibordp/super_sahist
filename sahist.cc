#include "sahist.h"

using namespace std;
using namespace std::rel_ops;


namespace Sahist {

bool Figura::operator==(Figura rhs) const {
	return tip == rhs.tip && 
	barva == rhs.barva;
}	

bool Koordinata::operator==(Koordinata rhs) const {
	return stolpec == rhs.stolpec && 
	vrstica == rhs.vrstica;
}

Koordinata Koordinata::operator+(Koordinata rhs) const {
	return {stolpec + rhs.stolpec, vrstica + rhs.vrstica};
}

Koordinata& Koordinata::operator+=(Koordinata rhs) {
	stolpec += rhs.stolpec;
	vrstica += rhs.vrstica;
	return *this;
}	

inline Koordinata::operator bool() const {
	return stolpec >= 1 && stolpec <= 8 && 
	vrstica >= 1 && vrstica <= 8;
}

bool Poteza::operator==(Poteza rhs) const {
	return tip == rhs.tip && 
	od_koordinata == rhs.od_koordinata &&
	do_koordinata == rhs.do_koordinata &&
	tip_figure == rhs.tip_figure;
}

/*
	Postavi šahobvnico na začetno pozicijo
*/

void Sahovnica::inicializiraj() {
	const array<Figure, 8> vrstica =  {
		Figure::trdnjava, Figure::konj, 
		Figure::tekac,	Figure::kraljica,
		Figure::kralj,	Figure::tekac,
		Figure::konj, Figure::trdnjava
	};
	
	for (int i = 0; i < 8; ++i)
	{
		deska[0][i] =  {vrstica[i], Barva::bela};
		deska[1][i] =  {Figure::kmet, Barva::bela};
		for (int j = 2; j < 6; ++j)
		{
			deska[j][i] = Figura::prazna();
		} 
		deska[6][i] =  {Figure::kmet, Barva::crna};
		deska[7][i] =  {vrstica[i], Barva::crna};
	}
 /*
	Dokler ne premaknemo trdnjave ali kralja, so nam rošade na voljo
 */
	velike_rosade[Barva::bela] = true; velike_rosade[Barva::crna] = true;
	male_rosade[Barva::bela]   = true; male_rosade[Barva::crna] = true;
	ima_kralja[Barva::bela]   = true; ima_kralja[Barva::crna] = true;

	mimohod = Koordinata::neveljavna();
}

Sahovnica::Sahovnica(bool postavi_figure) {
	if (postavi_figure)
		inicializiraj();
}

/*
	V spominu imamo indeksirano od ničle, mi pa bomo indeksirali od 1.
*/

inline Figura& Sahovnica::polje(Koordinata pos) {
	return  deska[pos.vrstica - 1][pos.stolpec - 1];
}

inline bool Sahovnica::zasedeno(Koordinata pos) const {
	return deska[pos.vrstica - 1][pos.stolpec - 1] != Figura::prazna();
}

void Sahovnica::premakni(Poteza p) {
	auto& zacetek = polje(p.od_koordinata);
	auto& konec = polje(p.do_koordinata);

	/* Čim premaknemo kralja ali ustrezno trdnjavo, izgubimo možnost za [to specifično]
	   rošado */

	mimohod = Koordinata::neveljavna();

	if (konec.tip == Figure::kralj)
	{
		ima_kralja[konec.barva] = false;
	}

	if (zacetek.tip == Figure::kralj)
	{
		velike_rosade[zacetek.barva] = false;
		male_rosade[zacetek.barva] = false;
	}

	if (zacetek.tip == Figure::trdnjava)
	{
		if (p.od_koordinata.stolpec == 1)
			velike_rosade[zacetek.barva] = false;
		else
			male_rosade[zacetek.barva] = false;
	}		

	/* Če se je kmet premaknil za dve polji, dovolimo nasportniku en passant za eno potezo */

	if (zacetek.tip == Figure::kmet)
	{
		if (abs(p.do_koordinata.vrstica - p.od_koordinata.vrstica) == 2 )
			mimohod = p.do_koordinata;
	}		

	switch (p.tip)	{
		case Poteze::standardna:
			konec = zacetek; // = konec (Does it make you happy, you're so strange...)
			zacetek = Figura::prazna();
		break;

		case Poteze::velika_rosada:
		case Poteze::mala_rosada: {
			auto& trdnjava_zacetek = polje({(p.tip == Poteze::mala_rosada) ? 8 : 1, p.od_koordinata.vrstica});
			auto& trdnjava_konec   = polje({(p.tip == Poteze::mala_rosada) ? 6 : 4, p.od_koordinata.vrstica});

			konec = zacetek; 
			trdnjava_konec = trdnjava_zacetek;

			zacetek = Figura::prazna();
			trdnjava_zacetek = Figura::prazna();
			break;
		}
		case Poteze::promocija:
			konec = { p.tip_figure, zacetek.barva }; 
			zacetek = Figura::prazna();
			break;

		case Poteze::mimohod:
		konec = zacetek; 
		zacetek = Figura::prazna();

		// Izbrišemo še nasprotnikovega kmeta:
		polje({p.do_koordinata.stolpec, p.od_koordinata.vrstica}) = Figura::prazna();

		break;			
	}
}

/*
	Vrne množico vseh možnih premikov določene figure. Rošad in en-passantov tukaj ni, to hendla
	spodnja funkcija.
*/

void Sahovnica::dosegljive_pozicije(Koordinata pos, unordered_set<Koordinata>& results, bool samo_zajemi)
{
	auto figura = polje(pos);
	unordered_set<Koordinata> mozni_premiki;
	unordered_set<Koordinata> mozni_zajemi;

	auto dodaj_sprehod = [&](Koordinata smerni_vektor) {
		auto pozicija = pos + smerni_vektor;
		/*
			dodaj_sprehodimo se v smeri smernega vektorja, dokler ne naletimo na oviro.
		*/
		if (smerni_vektor == Koordinata({0, 0}))
			return;

		while (pozicija) {
			mozni_zajemi.insert(pozicija);
			if (zasedeno(pozicija))
				break;
			pozicija += smerni_vektor;
		}
	};

	switch	(figura.tip) {
		case Figure::kmet:
		if (pos.vrstica == Sahovnica::zacetek_kmeta(figura.barva) 
			&& !zasedeno({ pos.stolpec , pos.vrstica + 
				Sahovnica::smer_kmeta(figura.barva) }))
				mozni_premiki.insert({ pos.stolpec , pos.vrstica + 
				Sahovnica::smer_kmeta(figura.barva) * 2 });
		mozni_premiki.insert({ pos.stolpec , pos.vrstica + 
			Sahovnica::smer_kmeta(figura.barva) });

		mozni_zajemi.insert({ pos.stolpec + 1, pos.vrstica + 
			Sahovnica::smer_kmeta(figura.barva) });
		mozni_zajemi.insert({ pos.stolpec - 1, pos.vrstica + 
			Sahovnica::smer_kmeta(figura.barva) });
		break;

		case Figure::kralj: 
		for (int i = -1; i <= 1; ++i)
			for (int j = -1; j <= 1; ++j)
				if (i != 0 || j != 0)
					mozni_zajemi.insert({ pos.stolpec + i, pos.vrstica + j});
		break;

		case Figure::tekac:
		/* Tekac se premika diagonalno */
		dodaj_sprehod({-1,-1}); dodaj_sprehod({-1,1}); 
		dodaj_sprehod({1,-1}); dodaj_sprehod({1,1});
		break;

		case Figure::trdnjava:
		/* Trdnjava se premika ortogonalno */
		dodaj_sprehod({0,-1}); dodaj_sprehod({0,1}); 
		dodaj_sprehod({-1,0}); dodaj_sprehod({1,0});				
		break;

		case Figure::kraljica:
		/* Trdnjava se premika v vseh 8 smeri  */
		for (int i = -1; i <= 1; ++i)
			for (int j = -1; j <= 1; ++j)
				dodaj_sprehod({i,j});
			break;

			case Figure::konj: {
				const array<Koordinata, 8> konjevi_premiki = {{ 
					{ - 2, - 1 }, { - 2, + 1 }, 
					{ - 1, - 2 }, { - 1, + 2 },
					{ + 1, - 2 }, { + 1, + 2 },
					{ + 2, - 1 }, { + 2, + 1 }
				}};

				for (auto premik : konjevi_premiki)
				{
					auto f = pos + premik;
					mozni_zajemi.insert(f);
				}
				break;
			}			
		default:					

		break;
	}

	for (auto& koordinata : mozni_zajemi) {
		if (!koordinata || 
			!zasedeno(koordinata) || 
			polje(koordinata).barva == figura.barva) 
			continue;
		results.insert(koordinata);
	}

	if (!samo_zajemi) {
		if (figura.tip != Figure::kmet)
			mozni_premiki.insert(mozni_zajemi.begin(), mozni_zajemi.end());

		for (auto& koordinata : mozni_premiki) {
			if (!koordinata || 
				zasedeno(koordinata)) 
				continue;

			results.insert(koordinata);
		}		
	}
}

/*
	Vrne množico vseh "veljavnih" potez, tudi specialnih. Niso vse poteze veljavne, saj
	lahko npr. požremo kralja. Ampak, če sledimo pravilom, do te situacije nikoli ne more priti.
	Poteze ki so dejansko veljavne - torej take, ki te ne spravijo v šah, pa vrne spodnja funkcija.
*/

void Sahovnica::vse_poteze(Barva barva, vector<Poteza>& results) 
{
	for (int vrstica = 1; vrstica <= 8; ++vrstica)
	{
		for (int stolpec = 1; stolpec <= 8; ++stolpec)
		{
			auto figura = polje({stolpec, vrstica});

			if (polje({stolpec, vrstica}).barva != barva) 
				continue;

			unordered_set<Koordinata> dosegljive;
			dosegljive_pozicije({stolpec, vrstica}, dosegljive);

			for (auto nova_pozicija : dosegljive) 
			{
				/* 
					Če pridemo s kmetom na zadnje polje, imamo 4 različne možnosti za menjavo
				*/
				if ((nova_pozicija.vrstica == Sahovnica::konec_kmeta(barva)) && (figura.tip == Figure::kmet))
				{
					array<Figure, 4> zamenjave = {{ Figure::kraljica, Figure::trdnjava, Figure::konj, Figure::tekac }};
					for (auto zamenjava : zamenjave)
						results.push_back( { Poteze::promocija, {stolpec, vrstica}, nova_pozicija, zamenjava } );
				}
				else
					results.push_back({ Poteze::standardna, {stolpec, vrstica}, nova_pozicija });

			}
		}
	}

	/* Dodamo rošade, če je prostor in če še nismo premikali figur */
	const int vrstica = Sahovnica::zacetna_vrstica(barva);

	if (velike_rosade[barva]) 
	{
		if (!zasedeno({2, vrstica}) && !zasedeno({3, vrstica}) && !zasedeno({4, vrstica}))
			results.push_back({ Poteze::velika_rosada, {5, vrstica}, {3, vrstica}  });
	}

	if (male_rosade[barva]) 
	{
		if (!zasedeno({6, vrstica}) && !zasedeno({7, vrstica}))
			results.push_back({ Poteze::mala_rosada, {5, vrstica}, {7, vrstica} });			
	}

	if (mimohod && polje(mimohod).barva != barva) 
	{
		auto nasprotnik = polje(mimohod);
	/* Če najdemo svojega kmeta levo ali desno od nasprotnikovega kmeta in je polje pod nasprotnikovim kmetom prazno,
	   lahko nasprotnikovega kmeta požremo en passant */
		auto preveri = [&] (Koordinata pos_kmeta) {
			if (!pos_kmeta) return;
			auto kmet = polje(pos_kmeta);

			if (kmet.barva == barva && 
				kmet.tip == Figure::kmet && 
				!zasedeno({mimohod.stolpec, mimohod.vrstica + Sahovnica::smer_kmeta(barva)}) ) 
			{
				results.push_back({ Poteze::mimohod, pos_kmeta, 
					{mimohod.stolpec, mimohod.vrstica + Sahovnica::smer_kmeta(barva)} });					
			}
		};

		preveri({mimohod.stolpec - 1, mimohod.vrstica});
		preveri({mimohod.stolpec + 1, mimohod.vrstica});
	}

}

/* Ali nas bo naslednja poteza pripeljala v šah */

bool Sahovnica::veljavna_poteza(Poteza poteza) 
{
	auto figura = polje(poteza.od_koordinata);

	Sahovnica nova(*this);
	nova.premakni(poteza);
	vector<Poteza> nasprotnikove_poteze;

	/* Izpišemo si vse poteze, ki jih lahko naredi nasprotnik in preverimo, če je katera izmed 
		njih zajem našega kralja */
	nova.vse_poteze(Figura::nasprotna_barva(figura.barva), nasprotnikove_poteze);

	for (auto nasprotnikova_poteza : nasprotnikove_poteze) 
	{
		if (nova.polje(nasprotnikova_poteza.do_koordinata).tip == Figure::kralj)
			return false;
		
	/* Pri rošadah ne sme biti napadeno nobeno izmed polj, ki so med vključno kraljem in trdnjavo, 
		s katero rošadiramo. */

		if (poteza.tip == Poteze::velika_rosada)
		{
			for (int i = 1; i <= 5; ++i) 
				if (nasprotnikova_poteza.do_koordinata == 
					Koordinata({i, Sahovnica::zacetna_vrstica(figura.barva)}))
					return false;
		}

		if (poteza.tip == Poteze::mala_rosada)
		{
			for (int i = 5; i <= 8; ++i)	
				if (nasprotnikova_poteza.do_koordinata == 
					Koordinata({i, Sahovnica::zacetna_vrstica(figura.barva)}))
					return false;
		}
	}
			
	/* Kar je doubro, nej slabou */
	return true;
}	

void Sahovnica::prestej_figure(unordered_map<Figura, int>& rezultat) 
{
	rezultat[Figura({ Figure::kralj,    Barva::crna})] = 0;
	rezultat[Figura({ Figure::kralj,    Barva::bela})] = 0;
	rezultat[Figura({ Figure::kraljica, Barva::crna})] = 0;
	rezultat[Figura({ Figure::kraljica, Barva::bela})] = 0;
	rezultat[Figura({ Figure::trdnjava, Barva::crna})] = 0;
	rezultat[Figura({ Figure::trdnjava, Barva::bela})] = 0;
	rezultat[Figura({ Figure::tekac,    Barva::crna})] = 0;
	rezultat[Figura({ Figure::tekac,    Barva::bela})] = 0;
	rezultat[Figura({ Figure::konj,     Barva::crna})] = 0;
	rezultat[Figura({ Figure::konj,     Barva::bela})] = 0;
	rezultat[Figura({ Figure::kmet,     Barva::crna})] = 0;
	rezultat[Figura({ Figure::kmet,     Barva::bela})] = 0;

	for (auto const& vrstica : deska)
		for (auto const& figura : vrstica)
		{
			if (figura == Figura::prazna())
				continue;
			rezultat[figura]++;
		}
}

/*
	Iskanje optimalne poteze s pomočjo alfa-beta rezanja. Vrednotenjska funkcija je zelo simplistična,
*/

double Sahovnica::alfa_beta(Poteza poteza, double alpha, double beta, int globina, Barva maksimiziraj) 
{	
	auto faktor = (maksimiziraj == Barva::bela) ? -1 : 1;

	if (globina == 0)
	{
		premakni(poteza);
		unordered_map<Figura, int> figure;
		prestej_figure(figure);
		return ( 200 * (figure[{ Figure::kralj,    Barva::crna}] - figure[{ Figure::kralj,    Barva::bela}]) +
			      19 * (figure[{ Figure::kraljica, Barva::crna}] - figure[{ Figure::kraljica, Barva::bela}]) +
			       9 * (figure[{ Figure::trdnjava, Barva::crna}] - figure[{ Figure::trdnjava, Barva::bela}]) +
		           4 * (figure[{ Figure::tekac,    Barva::crna}] - figure[{ Figure::tekac,    Barva::bela}]) +
		           4 * (figure[{ Figure::konj,     Barva::crna}] - figure[{ Figure::konj,     Barva::bela}]) +
		           2 * (figure[{ Figure::kmet,     Barva::crna}] - figure[{ Figure::kmet,     Barva::bela}]))
		* faktor;
	}
	else
	{
		auto njegova_barva = Figura::nasprotna_barva(polje(poteza.od_koordinata).barva);

		Sahovnica nova(*this);
		vector<Poteza> odgovori;

		nova.premakni(poteza);

		// Če smo že izgubili kralja, ne rabimo več iskati v globino:
		if (!nova.ima_kralja[Barva::bela])
			return 200 * faktor;
	
		if (!nova.ima_kralja[Barva::crna])
			return -200 * faktor;		
		
		nova.vse_poteze(njegova_barva, odgovori);
	
		double najboljsa = -numeric_limits<double>::infinity();

		for (auto const& odgovor : odgovori) 
		{
			double vrednost = -nova.alfa_beta(poteza, -beta, -alpha, globina - 1, Figura::nasprotna_barva(maksimiziraj));
			if (vrednost >= beta)
				return vrednost;
			if (vrednost > najboljsa) 
			{
				najboljsa = vrednost;
				if (vrednost > alpha)
					alpha = vrednost;
			}
				
		}
		return najboljsa;
	}
} 

double Sahovnica::oceni_potezo(Poteza poteza, Barva barva) {
	
	int globina = 4;

	if (barva == Barva::bela) 
		return  alfa_beta(poteza, -numeric_limits<double>::infinity(), numeric_limits<double>::infinity(), globina, Barva::crna);
	else
		return  alfa_beta(poteza, -numeric_limits<double>::infinity(), numeric_limits<double>::infinity(), globina, Barva::bela);
}


void Sahovnica::dobre_poteze(Barva barva, vector<pair<double, Poteza>>& rezultat) 
{
	vector<Poteza> poteze;
	vse_poteze(barva, poteze);
	vector<thread> niti;

	mutex mtx;

	for (auto const& i : poteze)
	{
		if (veljavna_poteza(i)) 
		{
			niti.push_back(thread([&] {
				double ocena = oceni_potezo(i, barva);
				{   // Ne premakni tega { navzgor!
					// Da si niti ne hodijo v zelje pri vstavljanju rezultatov v vektor
					lock_guard<mutex> lg(mtx);
					rezultat.push_back( {ocena, i} );
				}
			}));
		}
	}

	for (auto &i : niti)
		i.join();
}	

Poteza Sahovnica::predlagaj(Barva barva)
{
	using vp = pair<double, Poteza>;
	vector<vp> poteze;

	dobre_poteze(barva, poteze);

	sort(poteze.begin(), poteze.end(), [](vp a, vp b) 
	{
		return a.first < b.first;
	});

	if (!poteze.empty())
	{
		auto end = poteze.begin();
		for (; end != poteze.end(); ++end)
		{
			if (end->first != poteze.begin()->first)
				break;		
		}

		shuffle(poteze.begin(), end, random_device());
		return poteze.front().second;
	}
	else
		return { Poteze::konec };
}	


void Sahovnica::mozne_poteze(Barva barva, vector<Poteza>& rezultat) 
{
	vector<Poteza> poteze;
	vse_poteze(barva, poteze);

	for (auto const& i : poteze)
	{
		if (veljavna_poteza(i)) 
			rezultat.push_back(i);
	}
}	

bool Sahovnica::sah(Barva barva) 
{
	vector<Poteza> nasprotnikove_poteze;

	/* Izpišemo si vse poteze, ki jih lahko naredi nasprotnik in preverimo, če je katera izmed 
		njih zajem našega kralja */
	vse_poteze(Figura::nasprotna_barva(barva), nasprotnikove_poteze);

	for (auto nasprotnikova_poteza : nasprotnikove_poteze) 
	{
		if (polje(nasprotnikova_poteza.do_koordinata).tip == Figure::kralj)
			return true;
	}
	
	return false;
}

bool Sahovnica::premakni_nakljucno(Barva barva) 
{
	vector<Poteza> poteze;

	vse_poteze(barva, poteze);
	//shuffle(poteze.begin(), poteze.end(), random_device());

	while (!poteze.empty())
	{
		if (veljavna_poteza(poteze.back()))
		{
			premakni(poteze.back());
			return true;
		}	
		poteze.pop_back();
	}		

	// Nimamo več veljavnih potez - ergo smo v matu ali patu
	return false;
}

};
