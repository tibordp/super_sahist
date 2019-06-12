import tkinter as tk
import tkinter.messagebox
import math
import threading
import time


from enum import Enum 
import random
from itertools import product

import sahist 

class Stanje(Enum):
	izbiram_figuro = 1
	izbiram_cilj = 2
	izbira_promocije = 3
	konec_igre = 4
	racunam_potezo = 5

class NacinIgre(Enum):
	igralec_igralec = 1
	igralec_racunalnik = 2
	racunalnik_racunalnik = 3

def nasprotna_barva(barva):
	return sahist.Barva.crna if barva == sahist.Barva.bela else sahist.Barva.bela

class SelekcijaFigure:
	"""Ko kmet pride na zadnje polje, mora izbrati zamenjavo. Ta razred nam izriše okvir,
	   v katerem figuro izberemo."""

	def __init__(self, sahovnica, barva):
		zgoraj_levo = (15 + 60, 15 + 140)
		spodaj_desno = (400 + 15 - zgoraj_levo[0], 400 + 15 - zgoraj_levo[1])

		self.pozicije_slik = list()

		mozne_figure = [sahist.Figure.kraljica, sahist.Figure.trdnjava, sahist.Figure.tekac, sahist.Figure.konj]

		self.sahovnica = sahovnica

		self.elements = [ 
			sahovnica.platno.create_rectangle( zgoraj_levo[0], zgoraj_levo[1], 
				spodaj_desno[0], spodaj_desno[1], fill = "#efefef" )
		]

		for i in range(0, 4):
			x, y = (zgoraj_levo[0] + 45 + 60 * i , zgoraj_levo[1] + 50)
			self.pozicije_slik.append( (x,y, mozne_figure[i]) )
			self.elements.append(sahovnica.platno.create_image(x,y,
				image = sahovnica.figure_slike[mozne_figure[i], barva],
				activeimage = sahovnica.figure_slike[mozne_figure[i], nasprotna_barva(barva)]))

	def izbrisi(self):
		for i in self.elements:
			self.sahovnica.platno.delete(i)

	def katera_figura(self, event):
		"""Ko uporabnik klikne, nam ta rutina pretvori koordinate miškinega klika v figuro"""
		for i in range(0, 4):
			px, py, pfigura = self.pozicije_slik[i]
			if event.x >= px - 24 and event.x <= px + 24 and event.y >= py - 24 and event.y <= py + 24:
				return pfigura

class KonecIgre:
	"""Ta razred nam izriše okvir, ki nas obvesti o šahmatu oz. patu."""

	def __init__(self, sahovnica, besedilo):
		zgoraj_levo = (15 + 60, 15 + 140)
		spodaj_desno = (400 + 15 - zgoraj_levo[0], 400 + 15 - zgoraj_levo[1])

		self.sahovnica = sahovnica

		self.elements = [ 
			sahovnica.platno.create_rectangle( zgoraj_levo[0], zgoraj_levo[1], 
				spodaj_desno[0], spodaj_desno[1], fill = "#efefef" ),
			sahovnica.platno.create_text( (zgoraj_levo[0]+spodaj_desno[0]) / 2, (zgoraj_levo[1]+spodaj_desno[1]) / 2, 
				 anchor = "center", font = ("TkDefaultFont", 40), fill = "black",  text = besedilo )
		]

	def izbrisi(self):
		for i in self.elements:
			self.sahovnica.platno.delete(i)		


class Sahovnica(object):
	"""Razred, ki kontrolira vizualni gradnik šahovnice"""

	def __init__(self, platno):
		"""Naloži figure z diska in naredi polja"""

		self.platno = platno
		self.velikost_polja = 400 / 8;

		# Naložimo sličice figur
		datoteke = {
			(sahist.Figure.kmet, sahist.Barva.bela) : "figure/kmet_beli.png",
			(sahist.Figure.trdnjava, sahist.Barva.bela) : "figure/trdnjava_bela.png",
			(sahist.Figure.konj, sahist.Barva.bela) : "figure/konj_beli.png",
			(sahist.Figure.kralj, sahist.Barva.bela) : "figure/kralj_beli.png",
			(sahist.Figure.tekac, sahist.Barva.bela) : "figure/tekac_beli.png",
			(sahist.Figure.kraljica, sahist.Barva.bela) : "figure/kraljica_bela.png",
			(sahist.Figure.kmet, sahist.Barva.crna) : "figure/kmet_crni.png",
			(sahist.Figure.trdnjava, sahist.Barva.crna) : "figure/trdnjava_crna.png",
			(sahist.Figure.konj, sahist.Barva.crna) : "figure/konj_crni.png",
			(sahist.Figure.kralj, sahist.Barva.crna) : "figure/kralj_crni.png",
			(sahist.Figure.tekac, sahist.Barva.crna) : "figure/tekac_crni.png",
			(sahist.Figure.kraljica, sahist.Barva.crna) : "figure/kraljica_crna.png"
		}

		self.figure_slike = dict()

		for (tip, barva), ime in datoteke.items():
			self.figure_slike[tip, barva] = tk.PhotoImage(file=ime)
		
		# Kam lahko premaknemo figuro označimo z ikono, ki jo postavimo
		# na polja:
		self.mozna_poteza = tk.PhotoImage(file="figure/mozna_poteza.png")

		self.polja = list()
		self.figure = list()
		self.mozne_poteze = list()

		sirina_ks = 12 
		self.kvadrat_stanja = self.platno.create_rectangle(sirina_ks,sirina_ks,430-sirina_ks, 430- sirina_ks, width=1)

		for i in range(0, 8):
			vrstica = list()

			for j in range(0, 8):
				vrstica.append(self.platno.create_rectangle(
				   self.velikost_polja * i + 15, 
				   self.velikost_polja * j + 15, 
				   self.velikost_polja * (i+1) + 15,
				   self.velikost_polja * (j+1) + 15,
				   fill = "white" if (i + j) % 2 == 0 else "black",
				))

			self.polja.append(vrstica)


	def postavi_figure(self, sah):
		"""Postavi figure glede na trenutno stanje šahovnice"""
		for i in self.figure:
			self.platno.delete(i)

		self.figure = list()

		for i, j in product(range(1,9), range(1,9)):
			if sah.zasedeno(sahist.Koordinata(i, j)):
				figura = sah.polje(sahist.Koordinata(i,j));
				slika = self.figure_slike[(figura.tip, figura.barva)]

				self.figure.append(self.platno.create_image( 
					i * self.velikost_polja - 24 + 15, 
					(9-j) * self.velikost_polja - 24 + 15 - 1, image=slika)
				)

	def pozicija(self, event):
		"""Ugotovi koordinato polja iz pozicije kurzorja"""
		vp = self.velikost_polja
		return (math.floor((event.x - 15) / vp) + 1, 8 - math.floor((event.y - 15) / vp))

	def pobarvaj(self, stolpec, vrstica, barva):
		"""Pobarva želeno polje na dano barvo"""
		self.platno.itemconfig(self.polja[stolpec - 1][8 - vrstica],
			fill=barva)	

	def postavi_ikono(self, stolpec, vrstica, ikona):
		"""Postavi ikono na polje, kamor lahko prestavimo figure"""
		self.mozne_poteze.append(self.platno.create_image( 
					stolpec * self.velikost_polja - 24 + 15, 
					(9-vrstica) * self.velikost_polja - 24 + 15 - 1, image=ikona)
				)
		# Dvignemo figure nad ikone:
		for figura in self.figure:
			self.platno.tag_raise(figura)

	def razbarvaj(self):
		"""Pobarva polja nazaj na črno/belo"""
		for i in range(1, 9):
			for j in range(1, 9):
				self.platno.itemconfig(self.polja[i - 1][j - 1],
					fill="white" if (i + j) % 2 == 0 else "black", 
					outline="black", width=1)	
		for i in self.mozne_poteze:
			self.platno.delete(i)	

class PotezaThread(threading.Thread):
	"""Razred, ki nam asinhrono izračuna potezo. Ko konča, sproži event, ki ga Tkinter 
	izvrši v glavnem threadu."""

	def __init__(self, sah, barva):
		threading.Thread.__init__(self)
		self.sah = sah
		self.barva = barva
		self.poteza = None

	def run(self):
		self.poteza = self.sah.predlagaj(self.barva)
		
		# Zaspimo za vsaj 100ms, zato da se GUI ne neha odzivati
		time.sleep(0.1)
		aplikacija.event_generate("<<Poteza>>")

class Igra(object):
	"""Ta razred nam predstavlja eno igro šaha"""

	def __init__(self, glavno_okno, nacin):
		# Nastavimo placeholderja za okni za iziro figure in za šahmat
		self.selektor = None
		self.pasica = None

		self.prejsnje = list()

		self.glavno_okno = glavno_okno
		self.sahovnica = self.glavno_okno.sahovnica

		self.nacin = nacin
		
		# Ustvarimo objekt sahist.Sahovnica, ki izvajaa glavno logiko programa (glej sahist.h, sahist.cc)
		self.sah = sahist.Sahovnica(True)
		self.sahovnica.razbarvaj()
		self.sahovnica.postavi_figure(self.sah)

		# Nastavimo, kaj se zgodi, ko uporabnik klikne na šahovnico
		self.sahovnica.platno.bind("<Button-1>", self.on_click)

		if nacin == NacinIgre.igralec_racunalnik:
			# Naključno izberemo, kdo bo začel igro - mi ali računalnik
			self.na_potezi = random.choice([sahist.Barva.bela, sahist.Barva.crna])
									
			if self.na_potezi == sahist.Barva.crna:
				self.racunalnik_zacne = True
				self.odgovori()
				return
			else:
				self.racunalnik_zacne = False

		elif nacin == NacinIgre.igralec_igralec:
			self.na_potezi= sahist.Barva.bela

		elif nacin == NacinIgre.racunalnik_racunalnik:
			self.na_potezi= sahist.Barva.bela
			self.odgovori()
		
		self.pobarvaj_kvadrat_stanja()
		self.stanje = Stanje.izbiram_figuro 

	def premakni(self, poteza):
		self.prejsnje.append(self.sah.kopija())

		if self.nacin != NacinIgre.igralec_racunalnik or len(self.prejsnje) > 1:
			self.glavno_okno.omogoci_razveljavi(True)

		self.sah.premakni(poteza)
		self.sahovnica.postavi_figure(self.sah)
	
	def razveljavi(self):
		if self.nacin == NacinIgre.igralec_racunalnik:
			self.prejsnje.pop()

		self.sah = self.prejsnje.pop()
		self.sahovnica.postavi_figure(self.sah)	
		self.sahovnica.razbarvaj()

		if self.nacin == NacinIgre.igralec_igralec:
			self.na_potezi = nasprotna_barva(self.na_potezi)

		# Ne moremo razveljaviti računalnikove prve poteze
		if len(self.prejsnje) == 0 or \
			(
				self.nacin == NacinIgre.igralec_racunalnik and \
				self.racunalnik_zacne and len(self.prejsnje) == 1 \
			 ) :
			self.glavno_okno.omogoci_razveljavi(False)

	def on_izracunana_poteza(self, event):
		"""Event handler, ki se izvrši, ko thread, ki računa potezo računalnika konča izvajanje. 
		Tukaj premaknemo figuro in nastavimo ustrezna stanja."""
		poteza = self.nit.poteza
		
		if poteza is None:
			return

		if self.nacin == NacinIgre.igralec_racunalnik or self.nacin == NacinIgre.racunalnik_racunalnik:	
			# Če smo ga matirali oz. patirali:
			if poteza.tip == sahist.Poteze.konec:
				self.preveri_sahmat(nasprotna_barva(self.na_potezi))
				return
			else:
				self.premakni(poteza)
				kam = poteza.do_koordinata
				
				# Pobarvamo polje, kamor je nasprotnik premaknil figuro
				self.sahovnica.pobarvaj(kam.stolpec, kam.vrstica, "#FA4A4A")
				
				# Če nas je računalnik matiral:
				if not self.sah.mozne_poteze(self.na_potezi):
					self.preveri_sahmat(self.na_potezi)
					return
		
		if self.nacin == NacinIgre.racunalnik_racunalnik:
			self.na_potezi = nasprotna_barva(self.na_potezi)
			self.odgovori()
			return

		self.pobarvaj_kvadrat_stanja()
		self.stanje = Stanje.izbiram_figuro

	def pobarvaj_kvadrat_stanja(self):
		"""Pobarvamo kvadrat, ki obdaja šahovnico z barvo igralca, ki je na potezi"""
		if self.na_potezi == sahist.Barva.bela:
			self.sahovnica.platno.itemconfig(self.sahovnica.kvadrat_stanja, fill="#ffffff")	
		else:
			self.sahovnica.platno.itemconfig(self.sahovnica.kvadrat_stanja, fill="#000000")	

	def predlagaj_potezo(self, barva):
		# Medtem ko program računa, ne moremo razveljaviti
		self.glavno_okno.omogoci_razveljavi(False)
		self.nit = PotezaThread(self.sah, barva)
		self.sahovnica.platno.itemconfig(self.sahovnica.kvadrat_stanja, fill="#ee2222")
		aplikacija.bind("<<Poteza>>", self.on_izracunana_poteza)	
		self.nit.start()


	def preveri_sahmat(self, barva):
		"""Preverimo ali je trenutna situacija šahmat ali zgolj pat in pokažemo ustrezno okno."""
		if self.sah.sah(barva):
			self.pasica = KonecIgre(self.sahovnica, "Šah mat!")					
		else:
			self.pasica = KonecIgre(self.sahovnica, "Pat!")	
		
		# Pobarvamo kralja
		for i in range(1,9):
			for j in range(1,9):
				fig = self.sah.polje(sahist.Koordinata(i, j)) 
				if fig.tip == sahist.Figure.kralj and fig.barva == barva:
					self.sahovnica.pobarvaj(i,j, "#ff0000")

		self.stanje = Stanje.konec_igre

	def odgovori(self):
		"""Ko je uporabnik izbral svojo potezo, je odgovor odvisen od tega s kom/čim igramo."""
		self.sahovnica.razbarvaj()
		aplikacija.update_idletasks()

		if self.nacin == NacinIgre.igralec_igralec:
			self.na_potezi = nasprotna_barva(self.na_potezi)
			self.pobarvaj_kvadrat_stanja()

			if not self.sah.mozne_poteze(self.na_potezi):
				self.preveri_sahmat(self.na_potezi)
				return

			self.stanje = Stanje.izbiram_figuro 
		
		elif self.nacin == NacinIgre.igralec_racunalnik:
			self.predlagaj_potezo(nasprotna_barva(self.na_potezi));
			self.stanje = Stanje.racunam_potezo

		elif self.nacin == NacinIgre.racunalnik_racunalnik:
			self.predlagaj_potezo(self.na_potezi)
			self.stanje = Stanje.racunam_potezo
		
	def on_click(self, event):
		"""Event handler za klik na šahovnico. Kaj naredi je odvisno od stanja (self.stanje)"""
		
		stolpec, vrstica = self.sahovnica.pozicija(event)

		# Če smo kliknili izven šahovnice
		if stolpec < 1 or stolpec > 8 or vrstica < 1 or vrstica > 8:
			return

		if self.stanje == Stanje.izbiram_figuro:

			if self.sah.polje(sahist.Koordinata(stolpec, vrstica)).barva != self.na_potezi:
				return

			self.sahovnica.razbarvaj()

			self.stanje = Stanje.izbiram_cilj
			self.izbrana_pozicija = (stolpec, vrstica)
			self.sahovnica.pobarvaj(stolpec, vrstica, "#99d9ea")
			
			for poteza in self.sah.mozne_poteze(self.na_potezi):
				od = (poteza.od_koordinata.stolpec, poteza.od_koordinata.vrstica)
				do = (poteza.do_koordinata.stolpec, poteza.do_koordinata.vrstica)
				if od == (stolpec, vrstica):
					self.sahovnica.postavi_ikono(do[0], do[1], self.sahovnica.mozna_poteza)

		elif self.stanje == Stanje.izbiram_cilj:			
			for poteza in self.sah.mozne_poteze(self.na_potezi):
				od = (poteza.od_koordinata.stolpec, poteza.od_koordinata.vrstica)
				do = (poteza.do_koordinata.stolpec, poteza.do_koordinata.vrstica)

				if self.izbrana_pozicija == od and (stolpec, vrstica) == do:
					
					# Če je izbrana poteza promocija kmeta, moramo vprašati igralca, za katero figuro
					# ga želi zamenjati.
					if poteza.tip == sahist.Poteze.promocija:
						self.selektor = SelekcijaFigure(self.sahovnica, self.na_potezi)
						self.koncna_pozicija = (stolpec, vrstica)
						self.stanje = Stanje.izbira_promocije
						return

					self.premakni(poteza)
					self.odgovori()
					return

			# Če smo kliknili nekam, kamor nimamo nobene veljavne poteze, razbarvamo
			# in s tem pozovemo igralca k monovni izbiri.

			self.sahovnica.razbarvaj()
			self.stanje = Stanje.izbiram_figuro				
		
		elif self.stanje == Stanje.izbira_promocije:
			figura = self.selektor.katera_figura(event)
			
			# Če je igralec izbral figuro, za katero želi zamenjati kmeta, zapremo okno
			# in izvršimo potezo, sicer ga pustimo odprtega.

			if figura is not None:
				for poteza in self.sah.mozne_poteze(self.na_potezi):
					od = (poteza.od_koordinata.stolpec, poteza.od_koordinata.vrstica)
					do = (poteza.do_koordinata.stolpec, poteza.do_koordinata.vrstica)

					if self.izbrana_pozicija == od and self.koncna_pozicija == do and poteza.tip_figure == figura:
						self.selektor.izbrisi()
						self.premakni(poteza)
						self.odgovori()
						return
				
				self.sahovnica.razbarvaj()
				self.stanje = Stanje.izbiram_figuro	

		elif self.stanje == Stanje.konec_igre:
			self.pasica.izbrisi()
	
	def izbrisi(self):
		# Izbrišemo morebitne odprte okvirje
		if self.pasica is not None:
			self.pasica.izbrisi()
		if self.selektor is not None:
			self.selektor.izbrisi()
		
		# Odstranimo event handlerje:
		aplikacija.unbind("<<Poteza>>")	

class Sahist(tk.Frame):
	def __init__(self, parent):
		tk.Frame.__init__(self, parent)
		self.parent = parent
		self.parent.wm_title("Šahist")
		self.parent.resizable(width=False, height=False)
		self.ustvari_gradnike()
		self.pack()

	def nova_igra(self, nacin):
		self.igra.izbrisi()
		self.igra = Igra(self, nacin)
	
	def omogoci_razveljavi(self, state):
		self.poteza_meni.entryconfig(0, state='normal' if state else 'disabled')

	def izhod(self):
		self.parent.quit()

	def ustvari_gradnike(self):
		self.platno = tk.Canvas(self, width=430, height=430)
		self.platno.pack()

		self.sahovnica = Sahovnica(self.platno)

		meniji = tk.Menu(self.parent)
		self.parent.config(menu=meniji)

		
		igra_meni = tk.Menu(meniji, tearoff=0)
		igra_meni.add_command(label="Nova igra (igralec-igralec)", command=
			lambda: self.nova_igra(NacinIgre.igralec_igralec))
		igra_meni.add_command(label="Nova igra (igralec-CPU)", command=
			lambda: self.nova_igra(NacinIgre.igralec_racunalnik))
		igra_meni.add_command(label="Nova igra (CPU-CPU)", command = 
			lambda: self.nova_igra(NacinIgre.racunalnik_racunalnik))
		igra_meni.add_separator()
		igra_meni.add_command(label="Izhod", command=self.izhod)

		meniji.add_cascade(label="Igra", menu=igra_meni)

		self.poteza_meni = tk.Menu(meniji, tearoff=0)
		self.poteza_meni.add_command(label="Razveljavi", 
			command=lambda: self.igra.razveljavi(), state = 'disabled')
		meniji.add_cascade(label="Poteza", menu=self.poteza_meni)
		
		# Ob zagonu takoj zaćnemo z igro
		self.igra = Igra(self, NacinIgre.igralec_racunalnik)

aplikacija = tk.Tk()

glavno_okno = Sahist(aplikacija)
glavno_okno.mainloop()
