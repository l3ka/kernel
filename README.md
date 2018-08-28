*  Title: Windows X
*  By: Darko Lekic (lekic.d7@gmail.com)
*  By: Nikola Pejic (peja96bl@gmail.com)
*  Description: OOS description 

https://wiki.osdev.org/GCC_Cross-Compiler

1. Instalirati sledece:
ZAVRSENO  ---   GMP	     ---   libgmp3-dev 
ZAVRSENO  ---   MPC	     ---   libmpc-dev 
ZAVRSENO  ---   MPFR	   ---   libmpfr-dev 
ZAVRSENO  ---   Texinfo	 ---   texinfo 
ZAVRSENO  ---   CLooG	   ---   libcloog-isl-dev
ZAVRSENO  ---   ISL	     ---   libisl-dev

2. Skinuti u home/src source code linkera i compilera
You can download the desired Binutils release by visiting the Binutils website or directly accessing the GNU main mirror.
You can download the desired GCC release by visiting the GCC website or directly accessing the GNU main mirror.

3. Za trajno postavljanje globalnih varijabli
/# gedit ~/.profile

Za provjeru verzije cross complier-a
$HOME/opt/cross/bin/$TARGET-gcc --version


Osnovi operativnih sistema:
Projektni, objasnjenja:
** One of the most important system resources that you need to manage is the processor or CPU 
** The next most important resource in the system is fairly obvious: Memory
** The last resource that your kernel needs to manage are hardware resources
** -j2 kod make gdje nema install


qemu-system-i386 -cdrom os.iso
qemu-system-i386 -kernel os.bin



Procesori 386 i noviji imaju 16-bitni režim rada koji je veoma blizak načinu rada procesora 8086 i naziva se "realni režim rada", 
sa bitnom razlikom da je izvršavanje 32-bitnih instrukcija moguće. Osim ovog, imaju i 32-bitni režim rada (naziva se "zaštićeni režim rada" odnosno "protected mode"). I u ovom režimu mogu da izvršavaju i 32-bitne i 16-bitne instrukcije (dakle isto kao i u realnom). 
Međutim, razlike su ogromne.

Nezasticeni mod --> nezaštićen jer ne proverava limit, pravo pristupa i vrstu memorije, ali i dalje koristi baznu adresu za računanje fizičke adrese.

Granularity bit. If 0 the limit is in 1 B blocks (byte granularity), if 1 the limit is in 4 KiB blocks (page granularity).


Kada loadujemo nas kernel, mi postavimo grub, a grub pronadje nas multiboot header. Boot fajl smo pisali u asm, jer nam asm omogucava da poravnamo segmente na nacin na koji mi to hocemo, i to specifikujemo linkerskom skriptom. Linkerska skripta sadrzi polozaje segmenata kada se nas source kod kompajlira. Poravnavanja su bitna zbog grub-a, jer grub ocekuje multiboot header na pocetku naseg kernel fajla(binarni fajl, i on mora da pocne sa odredjenim setom bita). Grub kao bootloader se ucita, pronadje sve kernele koji se nalaze u direktorijumu boot sa kernelima, i da bi nesto bilo kernel po specifikaciji grub-a to nesto mora da sadrzi nekoliko elemenata i to magic number, poravnavanje flagova i checksum(koji je minus zbir ta dva). 
Grub kao bootloader je za nas vec napravio gdt - Global descriptor table. 
GDT sadrzi podatke gdje se u nasem OS nalaze odredjene vrste memorijskog prostora, tj. kako smo particionisali memoriju. GDT nam govori gdje se nalaze, koje su privilegije za pristup i ostale meta info o tim memorijskim segmentima. 
Grub kada loaduje nas kernel, on je vec podesio neki gdt, kako bi on sam radio jer je i sam grub neka vrsta kernela. To da je podesio gdt znaci da je pozvao procesorsku insrukciju koja govori gdje pocinje gdt. GDT i IDT - Interrupt Descriptor Table -  nisu cisto softverski pojam, procesor zna za taj koncept i ocekuje da mu mi specifikujemo gdje se ta tabela nalazi. Ako takva tabela nije specifikovana onda se koristi BIOS - Basic Input/Output System. To znaci da dok god nam nije specifikovana tablea sa ulazima, bios ce da nam radio ulaz i izlaz(vazi samo za realni mod rada). Mi radimo sa zasticenim modom(32-bitni) i zato moramo da specifikujemo gdt i idt. 
Bios korisi IVT - Interrupt Vector Table, i to se koristi prije prelaska u zasticeni mod rada. Mi bismo mogli da radimo i sa biosom input(koristimo ivt), ali to necemo nego cemo da ucitamo gdt i idt(preci cemo iz realnog u zasticeni mod). Mi cemo procesoru specifikovati gdje se ove tabele nalaze, saljemo procesorsku instrukciju(saljemo je cpu-u), u vidu pokazivaca koji pokazuje na meta info koja sadrzi 2 podatka offset i pocetak te tabele. 

Gdt uglavnom za user mod programe. Gdt mora da postoji ako hocemo da koristimo idt. IDT je tabela svih interrupt-a, i ta tabela mora da sadrzi servisne rutine za sve interrupt-e. GDT mora da postoji kako se ne bi koristila ona koju grub interno definise, kako bi mogli definisati svoje segmente. 
Jer ako definisemo svoje segmente i kod, a cpu koristi grub-ovu gdt tableu moze se desiti da cpu prepise neku nasu instrukciju i tako izazove triple-fault. Gdt pravimo samo da nam grub ne bi razvalio sve i izazvao triple-fault. 
Triple-fault je crash i on izaziva automatski restart sistema, te da bi ga izbjegli moramo definisati gdt. 
U idt moramo da napravimo interrupt servisnu rutinu, za svaku def moramo imati definisanu rutinu. 
Postoje 2 vrste ovih rutina, jedna koja salje kod greske i druga koja ne salje kod greske. 
IRG - Interrupt Request, kada softverski pozivamo neki interrupt. IRG, servisne rutine koje se pozovu kada se softverski pozove neki interrupt. IRG nam omogucavaju da registujemo novu servisnu rutinu. 

INT 80hex generalno oznacava sistemski poziv. 
CLI - Clear Interrupt Bit, setuje flag na procesoru koji mu govori da ne servisira interrupt-e. Moramo iskljuciti servisiranje interrupt-a dok podesimo ponovo sve tabele. CPU servisira interrupt-e nakon svake izvrsene instrukcije. STI - Set Interrupt Bit, kaze procesoru da sad moze servisirati interrupte. 
U isrs.c rutine su extern zato sto ce rutine biti definisane asemblerom. 

gdt_flush() - vrijednost koja se unosi u eax registar predstavlja offset u tableli. 

Svi segmenti registri ce biti setovani na 0x10 sto oznacava drugi unos u globalnoj deskriptorskoj tabeli koja nam definise data segment. 
Prva, 0x08, nam definise code segment. 

jmp 0x08:flush2 --> znaci uradi far jump na flush2 koristeci ovaj code segment, i onda ce ovo setovati cs na 0x08. !?
ISR koje ne prihvataju kod greske, kod njih push-amo 0(nulu). 

C da bi radio mora imati sistemski stek. Ako ne setujemo stek necemo moci da radimo iz C-a, linkovace se ali ce se srusiti, imacemo triple-fault. Moramo uraditi boot iz asemblera i moramo setovati sistemski stek. 

IRQ je tabela sa prioritetima, i ako imamo 2 irq-a koji treba da budu servisirani, bice servisiran onaj sa nizim indeksom, odnosno sa vecim prioritetom(nizi indeks => veci prioritet).
Multiboot sekcija mora biti na pocetku i mora pocinjati nakon jednog megabajta. Zasto, Zatooo! Tako Grub ocekuje. Grub ce pokusa da procita to, ako ne nadje na 1MB odustaje. Multiboot mora da pocne na tacno 1MB od pocetka fajla. 
Kod steka prvo rezervisemo, pa stavimo labelu jer stek raste u suprotnom smjeru. 
Sve interrupt servisne rutine moraju da zavrsavaju sa iret instrukcijom, jer one push-aju i specijalne registre. 
.bss - Block Started by Symbol, segment kod nas sadrzi stek. 
.text segment je code segment. 
Voditi racuna da ne napravimo preklapanje izmedju prostora registrovanog u gdt-u i onoga u linkerskoj skripti??. 
I flag u skripti je specifikovan kako bi se linkovali svi common fajlovi iz include direktorijuma. 
