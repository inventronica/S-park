# SmartPark
## Abstract
În zilele noastre, numărul mașinilor crește exponențial, problema locurilor de parcare în orașe devenind din ce în ce mai critică. Una dintre modalitațile de rezolvare a acestei problme este construirea de locuri de parcare, cu toate acestea, pe toți ne deranjează clădirile imense din mijlocul orașelor, a căror scop este numai adăpostirea mașinilor. Dificultațile pe care le prezintă solțiile sctuale sunt costurile mari, necesitatea de spații mari, ineficența din punct de vedere al gestionării cursului mașinilor, din cauza necesității spațiului de manevră, și nu în ultimul rând, aspectul și imposibilitatea integrării în ansamblul arhitectural al zonei în care sunt amplasate, în special în centrele istorice si arhitecturale.

 Astfel, proiectul prezentat face parte dintr-unul mai larg,încearcând să rezolve problemele prezentate, prin dezvoltarea unei parcări inteligente, subterane, modulare, de tip carusel, eficientă din punct de vedere al spațiului utilizat, al costului și al timpului de construcție, putând fi cu ușurință integrată în orice spațiu arhitectural.

## Partea fizică

Ideea noastră a pornit de la un concept deja existent, numit Rotary Parking, însă noi l-am adaptat renunțând la roata dințată din partea superioară, care bloca accesul masinilor atunci cand vehiculele intrau prin partea de sus. Astfel, am ajuns sa creăm o parcare cu o cupolă unde am amplasat o serie de role pe care lanțurile gondolelor alunecă.

Capacul cutiei reprezintă nivelul solului, adică nivelul de la care mașinile încep coborârea catre subteran. În interiorul cutiei se află gondolele, placa de dezvoltare ESP32, driver-ul de motor, cititorul de carduri RFID și motorul care învârte mecanismul. De acesta sunt atârnate 8 gondole (locuri de parcare). Ele sunt acționate de o roată dințată, în partea inferioară iar în partea superioară, se află cupola care elimină nevoia de a avea o a 2-a roata dințată, așa cum se prezintă conceptul Rotary Parking, permițând, astfel, intrarea mașinilor prin partea superioară a sistemului. 

În proiectul nostru placa de dezvoltare ESP are 3 funcții principale: comunicarea cu cititorul de carduri, acționarea motorului și determinarea poziției actuale a locului de parcare. Placa comunică cu server-ul și îi trimite informații pentru a afișa pe aplicație date importante, cum ar fi numărul de locuri libere.

### <u>Modul de funcționare</u>
Proiectul nostru are 3 părți principale:
- Interfața cu utilizatorii, prin intermediul unui modul RFID și prin aplicația web
- Interfața cu serverul, prin websockets
- Comanda motorului 

Utilizatorilor li se vor pune la dispoziție două modalități de interacțiune cu parcarea: 

Prima, **fizică**, prin intermediul cardurilor RFID, iar a doua, **virtuală**, comunicând direct cu server-ul prin aplicația de telefon.

În starea de repaus, parcarea, cu excepția situației când aceasta este plină, va prezenta un loc liber. Un utilizator va putea parca în acest spațiu, după care va înregistra parcarea, fie prin acționarea cardului, fie prin aplicația pe telefon. În cazul înregistrării prin card, modulul de comandă va trimite datele cardului către server, așteptând validarea acestora. 

În ambele cazuri, după validarea datelor, parcarea va pune la dispoziție următorul loc liber.

Revendicarea mașinii se va face într-un mod similar: persoana va accesa parcarea prin card sau aplicație, serverul va valida datele, iar în cazul acesta, parcarea va aduce la dispoziția utilizatorului mașina parcată de acesta. După revendicare, va rămâne în aceeași poziție, oferind locul eliberat următoarei persoane.

În cazul în care mai multe persoane încearcă revendicarea mașinilor în același timp, aceste cereri for fi plasate într-o **coadă** și vor fi procesate pe rând.

Deși nu sunt prezenți în machetă, s-au luat în considerare în program senzori de prezență umană, astfel încât mișcarea parcării să nu înceapă decât în absența persoanelor din aria de pericol.
Server-ul va putea accesa oricând informațiile despre parcare și starea acesteia. De asemenea server-ul va putea seta diferiți parametri de funcționare.

- Modulul de comandă se va conecta la rețea prin Wi-Fi.
- Alimentarea modulului de comandă se poate face atât la 6 - 12V, prin portul _VIN_ al microcontroller-ului cât și la 3.3V, cu alimentare directă. 
- Alimentarea motorului se va face separat, la 6-9 V, și va fi comandată printr-un driver cu punte H.

![block_scheme](../master/spark-embedded-master/schema-bloc.png)

### <u>Materiale utilizate</u>:
- Placă de dezvoltare NodeMCU-32S bazată pe ESP32-WROOM
- Modul RFID-RC522
- Driver de motoare cu punte H L298N
- Sursă de alimentare 6-9 V (2 baterii Li-Ion de 3.7V)
- DC motor cu un reductor amplasat pe acesta
- Encoder magnetic pentru a afla numărul de rotații ale motorului

## Partea electrică

Pentru partea electrică

![electrical_scheme](../master/spark-embedded-master/electrica.png)

## Avantaje
Server

 → Utilizator → Administrator →Comunicarea cu alte parcari

## Partea de cod

## Bibliografie