# Tema 4 PCOM (client HTTP)
## Introducere
In cadrul acestei teme am citit comenzi de la tastatura date de user, si am executat fiecare comanda in parte, pana la introducerea comenzii exit.
<br>
Pentru executarea comenzilor am apelat cate o functie cu nume descriptiv pentru comanda curenta:

- **registerUser()**
- **loginUser(char\*)**
- **enterLibrary(char\*, char\*)**
- **getBooks(char\*, char\*)**
- **getBook(char\*, char\*)**
- **addBook(char\*, char\*)**
- **deleteBook(char\*, char\*)**
- **logoutUser(char\*)**

Si totodata, am folosit 3 functii pentru crearea mesajelor de request (GET, POST, DELETE):

- **compute_get_request(...)**
- **compute_post_request(...)**
- **compute_delete_request(...)**

In plus, am folosit o functie de verificare daca un string este **_numar_** sau nu. <br>
(sursa functie: https://www.geeksforgeeks.org/program-check-input-integer-string/).

```C
//  rescriere facuta de mine
int isNumber(char *s) {
    for (int i = 0; i < strlen(s); i++) {
        if (isDigit(s[i]) == 0) {
            return 0;
        }
    }
    return 1;
}
```

La fiecare comanda primita, deschid o conexiune catre SERVER si o inchid la final dupa ce primesc raspunsul, in urma request-ului facut (GET, POST, DELETE).

## Descriere functionalitate program

 ### **_Register_**
 - Pentru comanda "register" am apelat functia **registerUser()**. <br>
 - In cadrul functiei am citit de la input, username-ul si parola, am creat un obiect JSON cu ajutorul functiilor din biblioteca **parson**, am adaugat campurile username si password in obiect si apoi am creat un string nou (data_body) din obiectul JSON, serializandu-l cu functia **json_serialize_to_string_pretty(...)**. Dupa aceea am creat un mesaj de tip POST, adaugand si stringul de tip JSON (DATA) si l-am trimis la server. Apoi am preluat raspunsul de la server intr-un string si am cautat codul **"201 Created"** in raspunsul de la server, caz in care stiam ca utilizatorul a fost creat cu succes. Altfel afisam un cod de eroare.

 ### **_Login_**
 - Pntru comanda "login" am apelat functia **loginUser(char\*)**. <br>
 - In cadrul functiei am citit de la input, username-ul si parola, am creat un obiect JSON cu ajutorul functiilor din biblioteca **parson**, am adaugat campurile username si password in obiect si apoi am creat un string nou (data_body) din obiectul JSON, serializandu-l cu functia **json_serialize_to_string_pretty(...)**. Dupa aceea am creat un mesaj de tip POST, adaugand si stringul de tip JSON (DATA) si l-am trimis la server. Apoi am preluat raspunsul de la server si am cautat header-ul **Set-Cookie:** pentru a retine cookie-ul de sesiune al utilizatorului. Cookie-ul l-am retinut in stringul primit ca parametru in functie. Dupa aceea am verificat daca in raspuns se gaseste codul de succes "200 OK", caz in care afisam un mesaj de succes si setam variabila globala **user_logat** pe 1 pentru a retine faptul ca un user este logat la server. Altfel afisam un mesaj corespunzator de eroare.

### **_Enter_Library_**
- Pentru comanda "enter_library" am apelat functia **enterLibrary(char\*, char\*)**. <br>
- In cadrul functiei am verificat la inceput daca exista un utilizator logat pe server. Daca nu exista un user logat afisam un mesaj de eroare si nu executam comanda. Altfel, puneam cookie-ul de sesiune al utilizatorului conectat intr-o matrice de string-uri si cream un mesaj de tip GET adaugand acest cookie in apel, pentru verificarea accesului utilizatorului curent. Dupa crearea mesajului, il trimit catre server. Urmand ca raspunsul primit de la server sa contina token-ul de autorizare pe care il parsez din mesaj si il retin intr-un string **token** pe care il primesc ca parametru. Dupa retinerea token-ului verific daca mesajul de la server a fost unul de succes cautand codul "200 OK", caz in care afisez un mesaj corespunzator si setez flagul de user_autorizat pe 1. Altfel afisez un mesaj de eroare.

### **_GetBooks_**
- Pentru comanda "get_books" am apelat functia **getBooks(char\*, char\*)**. <br>
- In cadrul functiei am verificat la inceput daca utilizatorul curent este autorizat sau nu (daca este autorizat asta inseamna ca utilizatorul a fost si logat anterior). Daca acesta nu era autorizat afisam un mesaj de eroare si nu executam comanda. Altfel, cream un mesaj de tip GET in care adaugam cookie-ul de sesiune si token-ul de autorizare, pe care il trimiteam catre server. Dupa ce primeam raspunsul de la server, cautam inceputul DATA-ului (corpul JSON ce continea cartile), retineam corpul JSON-ului si il afisam serializat folosind functia mentionata mai sus. In cazul in care nu gaseam un corp JSON, afisam un mesaj de eroare.

### **_GetBook_**
- Pentru comanda "get_book" am apelat functia **getBook(char\*, char\*)**. <br>
- In cadrul functiei am verificat la fel ca mai sus daca utilizatorul este autorizat si are acces la biblioteca. Daca acesta nu era autorizat afisam un mesaj de eroare si nu executam comanda. Altfel, citesc id-ul cartii de la input, verific daca acesta nu este cumva un **string** in loc de un **numar**, caz in care afisez un mesaj de eroare si nu execut comanda mai departe, in caz contrar creez calea catre cartea cu id-ul "n" (n este introdus de la tastatura) si compun un mesaj de tip GET in care introduc cookie-ul de seiune si token-ul de autorizare. Mai departe trimit mesajul catre server, primesc raspunsul si caut in raspuns daca nu a aparut vreun cod de eroare ("error"), caz in care retin intr-un string corpul JSON de la final si il afisez serializat. Altfel afisez un mesaj de eroare.

### **_AddBook_**
- Pentru comanda "add_book" am apelat functia **addBook(char\*, char\*)**. <br>
- In cadrul functiei am verificat la inceput la user-ul curent este autorizat si are acces la biblioteca (daca nu era autorizat afisam un mesaj de eroare si nu executam comanda). Dupa aceea citesc de la tastatura detaliile despre carte (titlu ,autor, gen, publicatie si numarul de pagini) si verific daca datele introduse corespun tipurilor de date pentru fiecare camp in parte (numar de pagini sa fie numar si restul stringuri si fiecare camp contine macar un caracter). Daca un camp introdus nu corespunde tipului sau de date afisez un mesaj de eroare corespunzator. Altfel, creez un obiect JSON introducand pe rand fiecare camp al cartii si il stochez serializat intr-un string (body_data), care reprezinta partea de DATA din POST request. Dupa aceea creez un mesaj de tip POST in care introduc cookie-ul de sesiune, token-ul de autorizare si body_data-ul creat anterior. Trimit mesajul catre server si verific daca raspunsul contine codul de succes "200 OK", caz in care cartea a fost adaugata cu succes si afisez un mesaj corespunzator, altfel afisez un mesaj de eroare.

### **_DeleteBook_**
- Pentru comanda "delete_book" am apelat functia **deleteBook(char\*, char\*)**. <br>
- In cadrul functiei am verificat la inceput daca utilizatorul curent are acces la biblioteca, iar daca nu are am afisat un mesaj de eroare si nu am executat comanda. In caz ca acesta este autorizat am citit de la input id-ul cartii care se doreste a fi stearsa, am verificat sa fie **numar** si nu **string** si am creat calea catre cartea cu id-ul dat. Dupa aceea am creat un mesaj de tip DELETE request in care am introdus cookie-ul de sesiune si token-ul de autorizare si l-am trimis catre server. La final am verificat daca raspunsul continea codul de succes "200 OK" caz in care am afisat un mesaj corespunzator. Altfel, am afisat un mesaj de eroare.

### **_Logout_**
- Pentru comanda "logout" am apelat functia **logoutUser(char\*)**. <br>
- In cadrul functiei am verificat la inceput daca exista un user logat pe server pentru a-l putea deloga. In cazul in care nu exista afisez un mesaj de eroare si nu fac nicio cerere la server. Altfel creez un mesaj de tip GET adaugand cookie-ul de sesiune al utilizatorului curent si il trimit catre server. Apoi caut in raspunsul de la server codul de succes "200 OK", afisez un mesaj corespunzator si setez cele doua flaguri user_logat si user_autorizat pe 0. Daca nu gasesc codul de succes afisez un mesaj de eroare.
- In main dupa apelul functiei sterg si cookie-ul de sesiune si token-ul de autorizare.

### **_Exit_**
- Pentru comanda "exit" setez cele doua flaguri pe 0, sterg cookie-ul si token-ul si opresc programul instant.

## Functiile de Request
Functiile pentru get si post sunt similare cu cele din laborator singurele modificari fiind adaugarea parametrului de token si schimbarea data_body-ului din char** in char* (string de tip JSON). Functia de Request Delete este aceeasi functie cu cea de GET fiind modificata doar denumirea metodei din get in delete.

### **_GET_**
- In cadrul functiei **compute_get_request(...)** am creat mesajul adaugat pe rand:
    - Numele metodei (GET), url-ul si alti parametrii daca aveam \r\n
    - Host-ul \r\n
    - Cookie-uri de sesiune daca aveam \r\n
    - Token de autorizare daca aveam \r\n
    - \r\n

### **_POST_**
- In cadrul functiei **compute_post_request(...)** am creat mesajul adaugat pe rand:
    - Numele metodei (POST), url-ul \r\n
    - Host-ul \r\n
    - Content-Type \r\n
    - Content-Length \r\n
    - Cookie-uri de sesiune daca aveam \r\n
    - Token de autorizare daca aveam \r\n
    - \r\n
    - DATA (JSON format string)

### **_DELETE_**
- In cadrul functiei **compute_delete_request(...)** am creat mesajul adaugat pe rand:
    - Numele metodei (DELETE), url-ul si alti parametrii daca aveam \r\n
    - Host-ul \r\n
    - Cookie-uri de sesiune daca aveam \r\n
    - Token de autorizare daca aveam \r\n
    - \r\n

## Biblioteca PARSON (JSON)
- Am folosit aceasta biblioteca pentru a crea obiecte de tip JSON si pentru a serializa aceste obiecte in string-uri pe care le trimit ca si corpul DATA dintr-un POST REQUEST.
```C
//  Exemplu de folosire pentru data_body la login
JSON_Value *body_value = json_value_init_object();
JSON_Object *body_object = json_value_get_object(body_value);
json_object_set_string(body_object, "username", username);
json_object_set_string(body_object, "password", password);
char *body_data = json_serialize_to_string_pretty(body_value);
```
- Totodata, am folosit functia de serializare si pe string-urile primite si in raspunsul de la server pentru a avea o afisare mai froasa a cartilor.
```C
//  Exemplu de folosire pentru afisare carti
response = strstr(response, "[");
JSON_Value *books_value = json_parse_string(response);
char *serialized_books = json_serialize_to_string_pretty(books_value);
printf("%s\n", serialized_books);
```
