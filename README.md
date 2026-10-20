# Progetto Sistemi Operativi - Gioco di Forza 4 in C (IPC e System Call Unix)

Questo repository contiene l'elaborato di gruppo sviluppato per il corso di **Sistemi Operativi** (A.A. 2021/2022) presso l'Università di Verona. Il progetto implementa il gioco **Forza 4** basato su un'architettura Client-Server in ambiente Unix/Linux, sfruttando le system call per la concorrenza, la memoria condivisa, i semafori e le inter-process communication (IPC).

## Autori
* **Simone Rossato** (VR471881)
* **Matteo Mantovani** (VR474454)
* **Thomas Lonardi** (VR473647)

---

## Descrizione del Progetto
Il sistema si compone di due moduli eseguibili principali (`F4Server` e `F4Client`) che consentono a due giocatori (oppure a un giocatore umano contro una CPU gestita autonomamente) di sfidarsi da terminale su una griglia di dimensioni personalizzabili (dimensione minima 5x5).

### Tecnologie e Meccanismi Unix Utilizzati
* **Inter-Process Communication (IPC):** Utilizzo di code di messaggi (`msgget`, `msgsnd`, `msgrcv`) definite in `msg_queue.h` per lo scambio di configurazioni e parametri (griglia, simboli, PID, tempo di timeout, turni).
* **Memoria Condivisa (Shared Memory):** Gestione del tabellone di gioco mediante segmenti di memoria condivisa (`shmget`, `shmat`, `shmdt`, `shmctl`) definiti in `shared_memory.h` per garantire l'aggiornamento dinamico della griglia.
* **Semafori System V:** Sincronizzazione dell'accesso alle risorse condivise e alternanza dei turni di gioco tramite un set di semafori (`semget`, `semctl`, `semop`) definiti in `semaphore.h`.
* **Gestione dei Segnali:** Gestione asincrona degli eventi (abbandono partita con `SIGINT`/CTRL+C, gestione dei timeout con `SIGALRM` e notifiche di fine partita/vittoria/sconfitta tramite `SIGUSR1` e `SIGUSR2`) definita in `segnale.h`.

---

## Moduli e Architettura del Codice

### File Sorgente (`src/`)
* `F4Server.c`: Processo arbitro. Inizializza le strutture IPC (memoria condivisa, semafori, code di messaggi), gestisce il matchmaking dei client, controlla lo stato della griglia e decreta la vittoria o la parità.
* `F4Client.c`: Processo utente. Permette la gestione dell'input da tastiera o dell'inserimento casuale in caso di giocatore CPU, invia le mosse al server e ne attende il riscontro.
* `controllo.c` / `controllo.h`: Implementazione della logica di gioco. Contiene le funzioni per il controllo orizzontale, verticale e diagonale della quadrupla vincente, il controllo della parità e le routine di inserimento del gettone.
* `matrix.c` / `matrix.h`: Funzioni di utilità per convertire il tabellone di gioco tra array lineare in memoria condivisa e matrice bidimensionale a schermo.
* `segnale.c` / `segnale.h`: Handler dei segnali per la gestione di disconnessioni, scadenze dei timer per inattività, vittoria/sconfitta e pulizia corretta delle risorse alla terminazione del server o del client.
* `semaphore.c` / `semaphore.h`: Wrapper per le operazioni atomiche sui semafori (`semOp`).
* `shared_memory.c` / `shared_memory.h`: Funzioni di supporto per la creazione, l'aggancio, lo sgancio e la rimozione della memoria condivisa.
* `errExit.c` / `errExit.h`: Gestione centralizzata delle eccezioni con terminazione sicura dei processi tramite segnale.

---

## Struttura del Repository
```text
f4/
├── src/
│   ├── F4Server.c
│   ├── F4Client.c
│   ├── controllo.c
│   ├── controllo.h
│   ├── matrix.c
│   ├── matrix.h
│   ├── segnale.c
│   ├── segnale.h
│   ├── semaphore.c
│   ├── semaphore.h
│   ├── shared_memory.c
│   ├── shared_memory.h
│   ├── msg_queue.h
│   ├── errExit.c
│   └── errExit.h
├── obj/          # Cartella dei file oggetto compilati (.o)
├── bin/          # Cartella degli eseguibili generati (F4Server, F4Client)
├── Makefile      # Script di compilazione del progetto
└── Relazione.pdf # Relazione tecnica dell'elaborato
```

### Compilazione ed Esecuzione

```bash
# Compilazione del progetto
make

# Avvio del Server (Righe, Colonne, Gettone1, Gettone2)
./bin/F4Server 6 7 O X

# Avvio Giocatore 1 (in un altro terminale)
./bin/F4Client Giocatore1

# Avvio Giocatore 2 (in un terzo terminale)
./bin/F4Client Giocatore2

# In alternativa, sfidare la CPU:
./bin/F4Client Giocatore1 \*

# Pulizia dei file oggetto e binari
make clean
