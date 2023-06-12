# SmartPark

În zilele noastre, numărul mașinilor crește exponențial, problema locurilor de parcare în orașe devenind din ce în ce mai critică. Una dintre modalitațile de rezolvare a acestei problme este construirea de locuri de parcare, cu toate acestea, pe toți ne deranjează clădirile imense din mijlocul orașelor, a căror scop este numai adăpostirea mașinilor. Dificultațile pe care le prezintă solțiile sctuale sunt costurile mari, necesitatea de spații mari, ineficența din punct de vedere al gestionării cursului mașinilor, din cauza necesității spațiului de manevră, și nu în ultimul rând, aspectul și imposibilitatea integrării în ansamblul arhitectural al zonei în care sunt amplasate, în special în centrele istorice si arhitecturale.

 Astfel, proiectul prezentat face parte dintr-unul mai larg,încearcând să rezolve problemele prezentate, prin dezvoltarea unei parcări inteligente, subterane, modulare, de tip carusel, eficientă din punct de vedere al spațiului utilizat, al costului și al timpului de construcție, putând fi cu ușurință integrată în orice spațiu arhitectural.

## Partea fizică

Ideea noastră a pornit de la un concept deja existent, numit Rotary Parking, însă noi l-am adaptat renunțând la roata dințată din partea superioară, care bloca accesul masinilor atunci cand vehiculele intrau prin partea de sus. Astfel, am ajuns sa creăm o parcare cu o cupolă unde am amplasat o serie de role pe care lanțurile gondolelor alunecă.

Capacul cutiei reprezintă nivelul solului, adică nivelul de la care mașinile încep coborârea catre subteran. În interiorul cutiei se află gondolele, placa de dezvoltare ESP32, driver-ul de motor, cititorul de carduri RFID și motorul care învârte mecanismul. De acesta sunt atârnate 8 gondole (locuri de parcare). Ele sunt acționate de o roată dințată, în partea inferioară iar în partea superioară, se află cupola care elimină nevoia de a avea o a 2-a roata dințată, așa cum se prezintă conceptul Rotary Parking, permițând, astfel, intrarea mașinilor prin partea superioară a sistemului. 

În proiectul nostru placa de dezvoltare ESP are 3 funcții principale: comunicarea cu cititorul de carduri, acționarea motorului și determinarea poziției actuale a locului de parcare. Placa comunică cu server-ul și îi trimite informații pentru a afișa pe aplicație date importante, cum ar fi numărul de locuri libere.

![electrical_scheme](../master/spark-embedded-master/electrica.png)

## Partea electrică

## Avantaje

## Partea de cod
