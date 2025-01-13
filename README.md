# Telemetry
Proiectul este alcatuit dintr-un daemon si o librarie care expune functionalitatea daemonului.

Daemonul se ocupa cu distributia de mesaje pe canale de distributie, intre mai multi participanti.

Canalele de distributie pot fi organizate ierarhic, in asa fel incat un mesaj trimis pe un canal 'parinte' va fi trimis si pe canalele 'copil'.

# Distribuire task-uri:
* **Pascaru Dan Alexandru** – Reprezentare canale
* **Petre Robert Cristian** – Comunicare librarie-daemon
* **Popescu Tiberiu** – Comunicare user-canale
* **Voicu Ioan Vladut** – Creare daemon

# Daemonizare
Daemonul respecta modelul de "new-style daemons". Daemonizarea este realizata printr-o librarie C header-only, aceasta permitand daemonizarea in asa fel incat sa se creeze un PID file, sa ofere optiunea de logging (expunand o interfata dedicata) si sa urmeze desigur pasii necesari daemonizarii conforme.

Libraria daemon.h expune un API de daemonizare cu urmatoarele functionalitati:
- pornire si event-loop pentru un daemon;
- API secundar de logging
- integrare systemd (scuze, dar acapareaza lumea)

Pentru a folosi daemon.h:

- creati un `daemon_config_t` cu setarile dorite: nivelul de logging, working dir, umask, etc., logica aplicatiei fiind un callback pentru care puteti furniza argumente, tipic UNIX;
- apelati, pasand structul creat anterior, functia initialize_daemon(); eventual, daca este cazul, creati un handler pentru a procesa perechi KV, si folositi functia parse_config_file();
- este recomandat daca ati folosit functionalitatea pentru configurari sa creati un handler care sa reincarce configuratia, si setati-l pentru a fi apelat in cazul SIGHUP cu functia register_config_handler;
- daca intentionati sa integrati cu systemd, aveti la dispozitie daemon_notify_status
- de preferat, in codul dumneavoastra, verificati starea daemonului si actionati corespunzator:
  - daemon_should_reload() returneaza daca s-a primit SIGHUP; in caz afirmativ, daca s-a setat deja handlerul de reload, va rog pregatiti daemonul pentru a redeschide socket-uri, fisiere, etc.;
  - daemon_is_running() returneaza daca inca ruleaza daemonul; in caz in care nu mai ruleaza, inchideti inainte sa se termine event-loop-ul toate socket-urile si descriptorii de fisiere deschis;

# Reprezentare canale

## Libraria dir.h
### Crearea si stergerea directoarelor
- **create_dir**: Permite crearea de directoare multiple(separate prin '/') si configurarea initiala a fisierelor de log(log.txt) in fiecare director nou creat.
- **remove_dir**: Permite stergerea directoarelor si a intregului continut din ele si din adancimea lor.

### Gestionarea fisierelor de log
- **open_log**: Deschide un fisier de log dintr-o ruta specificata si il returneaza.

### Parcurgerea directoarelor
- **get_dir**: Deschide si returneaza toate subdirectoarele dintr-un director sub forma unei liste simplu inlantuite sub forma de ruta absoluta.
- **Structura listei**: list->path, list->next si mereu se termina cu NULL.

